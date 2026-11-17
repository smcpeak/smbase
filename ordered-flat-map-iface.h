// ordered-flat-map-iface.h
// Interface decls for `ordered-flat-map`.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_IFACE_H
#define SMBASE_ORDERED_FLAT_MAP_IFACE_H

#include "ordered-flat-map-fwd.h"      // fwds for this module

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [f]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-tuple-fwd.h"      // std::tuple [f]
#include "smbase/std-utility-fwd.h"    // std::pair [n]

#include <cstddef>                     // std::size_t
#include <type_traits>                 // std::{decay_t, enable_if_t, is_same_v}
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


/*
  Map from KEY to VALUE where:

    1. The primary data structure is two vectors, one sorted by KEY and
       the other, of equal size, carrying corresponding VALUEs.  This
       enables efficient lookup based on binary search at the expense of
       slower insertion.  (See `std::flat_map`.)

    2. It has a separate vector of KEY that remembers the order in which
       keys were first inserted.  This order is what determines the
       numeric "index" of a key/value pair (*not* the order in the
       sorted keys vector).

  KEY requirements:

    * Copyable.

    * Comparable with `COMPARE`, inducing a total order.

  VALUE requirements:

    * Copyable.

    * Default-constructible if `valueRefAtKey` is used.

  I call this "ordered", meaning *extrinsically* ordered, for the same
  reasons that OrderedMap is called ordered.  See the comments in
  ordered-map-iface.h.

  There are several limitations of the current implementation, such as
  not allowing removal, or assignment or comparison of the entire
  container.  These should be straightforward to add if/when needed.
*/
template <typename KEY,
          typename VALUE,
          typename COMPARE> // Defaults to std::less<KEY> in -fwd.h file.
class OrderedFlatMap {
public:      // types
  // Type for container sizes and numeric (positional) indices.
  using size_type = std::size_t;

  // What dereferencing an iterator yields.  Since the keys and values
  // are stored in separate vectors, there is no `std::pair` object in
  // the container to refer to, so iterators return these pairs *by
  // value*.  Each pair contains references into the container.  (This
  // is the same approach used by `std::flat_map`.)
  using const_reference = std::pair<KEY const &, VALUE const &>;
  using reference       = std::pair<KEY const &, VALUE       &>;

  // Iterate over the map entries in extrinsic order without modifying
  // anything.
  class const_iterator {
  private:     // data
    // The map we are iterating over.
    OrderedFlatMap const &m_map;

    // Modification count of `m_map` at creation time.
    unsigned m_mapModificationCount;

    // Extrinsic index of the next element, or `size()` if this is the
    // end iterator.
    size_type m_index;

  public:      // methods
    inline const_iterator(const_iterator const &obj);

    // Begin iterating at `index`.
    inline const_iterator(OrderedFlatMap const &map, size_type index);

    // Assigning an iterator requires that both refer to the same map.
    inline const_iterator &operator=(const_iterator const &obj);

    // True if this iterator can still be used with the container.
    inline bool isValid() const;

    inline bool operator==(const_iterator const &obj) const;
    inline bool operator!=(const_iterator const &obj) const;

    inline bool isEnd() const;

    inline const_iterator &operator++();

    // Returns by value; see `const_reference`.
    inline const_reference operator*() const;
  };

  // Iterate in extrinsic order, possibly modifying the values but not
  // the keys.
  class iterator {
  private:     // data
    // Underlying const iterator to handle the mechanics.
    const_iterator m_iter;

  public:      // methods
    inline iterator(iterator const &obj);

    // Begin iterating at `index`.
    inline iterator(OrderedFlatMap &map, size_type index);

    // Assigning an iterator requires that both refer to the same map.
    inline iterator &operator=(iterator const &obj);

    // True if this iterator can still be used with the container.
    inline bool isValid() const;

    inline bool operator==(iterator const &obj) const;
    inline bool operator!=(iterator const &obj) const;

    inline bool isEnd() const;

    inline iterator &operator++();

    // Returns by value; see `reference`.
    inline reference operator*() const;
  };

protected:   // data
  // All map keys, in intrinsic key order.
  //
  // Invariant: All elements are unique.
  std::vector<KEY> m_sortedKeys;

  // Corresponding values.  `m_sortedKeys[i]` is mapped to
  // `m_correspValues[i]`.
  //
  // Invariant: m_correspValues.size() == m_sortedKeys.size()
  std::vector<VALUE> m_correspValues;

  // Indices into `m_sortedKeys` that record the order in which the keys
  // were originally inserted.
  //
  // I choose to use `unsigned` rather than `size_type` because that
  // saves memory and I expect it to be sufficient here.
  //
  // Invariant: m_insertionOrder.size() == m_sortedKeys.size()
  //
  // Invariant: The set of values in `m_insertionOrder` is [0,size()-1].
  std::vector<unsigned> m_insertionOrder;

  // Number of times this container has been structurally modified.
  // This is used to detect the use of invalid iterators.
  //
  // Unlike `OrderedMap`, only changes to the set of keys (insertion and
  // clearing) count as modifications.  Replacing the value of an
  // existing key does not, since iterators only store an index, and
  // that index still denotes the same entry afterward.
  unsigned m_modificationCount = 0;

protected:   // funcs
  // Return the first index in `m_sortedKeys` whose key is not less than
  // `key`, or `m_sortedKeys.size()` if all are.  Also return true if
  // the returned index has `key`.
  inline std::tuple<size_type, bool> lookup(KEY const &key) const;

  // True if `k1` and `k2` are equal according to `COMPARE`.
  inline static bool equalKeys(KEY const &k1, KEY const &k2);

  // Given that we have already looked up `key` and determined that it
  // is not present, and would go into `index`, insert it and `value`
  // into that location.
  template <typename K, typename V>    // Universal refs for key/value.
  inline VALUE &insertNewKeyAndValue(
    size_type index, K &&key, V &&value);

public:      // funcs
  inline ~OrderedFlatMap();

  inline void selfCheck() const;

  // -------------------------- Constructors ---------------------------
  inline OrderedFlatMap();

  inline OrderedFlatMap(OrderedFlatMap const &obj);
  inline OrderedFlatMap(OrderedFlatMap      &&obj);

  // ------------------------- Element access --------------------------
  // True if `key` is mapped to some value.
  inline bool contains(KEY const &key) const;

  // Return the value at `key`, which must be mapped.
  //
  // Requires: contains(key)
  inline VALUE const &valueAtKey(KEY const &key) const;

  // Return the key at `index`.
  //
  // Requires: 0 <= index < size()
  inline KEY const &keyAtIndex(size_type index) const;

  // Return the value at `index`.
  //
  // Requires: 0 <= index < size()
  inline VALUE const &valueAtIndex(size_type index) const;

  // ---------------------------- Iterators ----------------------------
  // Iteration is in extrinsic (insertion) order.  Iterators are
  // invalidated by inserting a new key or by `clear()`, but not by
  // changing the value of an existing key.
  inline const_iterator cbegin() const;
  inline const_iterator cend() const;

  inline const_iterator begin() const;
  inline const_iterator end() const;

  inline iterator begin();
  inline iterator end();

  // ---------------------------- Capacity -----------------------------
  // True if the container is empty.
  inline bool empty() const;

  // Number of entries in the container.
  inline size_type size() const;

  // ---------------------------- Modifiers ----------------------------
  // If `key` is already mapped, update its value and return false
  // without altering the extrinsic order.  Otherwise (the key is new),
  // insert (append) a new entry that maps `key` to `value` and return
  // true.
  template <typename K,    // Universal refs for key/value.
            typename V,

            // Require that the deduced K be (nearly) the same as KEY.
            // The primary reason is that, inside this function, we
            // might otherwise do two conversions (one for lookup and
            // one for insertion).
            typename = std::enable_if_t<std::is_same_v<std::decay_t<K>, KEY>>

            // In contrast, K will be converted at most once, so we can
            // accept whatever is compatible..
           >
  inline bool setValueAtKey(K &&key, V &&value);

  // Like `setValueAtKey`, but assert that `key` was not already
  // present.
  template <typename K,    // Universal refs for key/value.
            typename V,

            // Same rationale as above.
            typename = std::enable_if_t<std::is_same_v<std::decay_t<K>, KEY>>
           >
  inline void setValueAtNewKey(K &&key, V &&value);

  // If `key` is mapped, return a mutable reference to its value.
  // Otherwise, insert a mapping from `key` to a default-constructed
  // value and return a reference to that value.
  //
  // This is mainly useful in the case that all values are different
  // from the default-constructed value, so the caller can tell by
  // inspecting the return value whether a new mapping was created.
  inline VALUE &valueRefAtKey(KEY const &key);

  // Remove all elements.
  inline void clear();

  // -------------------------- Serialization --------------------------
  // Return as an untagged ordered map.
  operator gdv::GDValue() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_FLAT_MAP_IFACE_H
