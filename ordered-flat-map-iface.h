// ordered-flat-map-iface.h
// Interface decls for `ordered-flat-map`.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_IFACE_H
#define SMBASE_ORDERED_FLAT_MAP_IFACE_H

#include "ordered-flat-map-fwd.h"      // fwds for this module

#include "smbase/compare-util-iface.h" // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
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

    * Default-constructible.

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

  // Map keys, in the order originally inserted.
  //
  // Invariant: m_insertionOrder.size() == m_sortedKeys.size()
  //
  // Invariant: The set of keys in `m_insertionOrder` is the same as the
  // set of keys in `m_sortedKeys`.
  std::vector<KEY> m_insertionOrder;

protected:   // funcs
  // Return the first index in `m_sortedKeys` whose key is not less than
  // `key`, or `m_sortedKeys.size()` if all are.
  size_type lookup(KEY const &key) const;

  // True if `k1` and `k2` are equal according to `COMPARE`.
  static bool equalKeys(KEY const &k1, KEY const &k2);

  // Remove all elements.  (For now, this is protected because it is
  // just an implementation detail of the move ctor.)
  void clear();

public:      // funcs
  inline ~OrderedFlatMap();

  inline void selfCheck() const;

  // -------------------------- Constructors ---------------------------
  inline OrderedFlatMap();

  inline OrderedFlatMap(OrderedFlatMap const &obj);
  inline OrderedFlatMap(OrderedFlatMap      &&obj);

  // ------------------------- Element access --------------------------
  // Return the value at `key`, which must be mapped.
  inline VALUE const &valueAtKey(KEY const &key) const;

  // Return the key at `index`.
  inline KEY const &keyAtIndex(size_type index) const;

  // Return the value at `index`.
  inline VALUE const &valueAtIndex(size_type index) const;

  // ---------------------------- Capacity -----------------------------
  // True if the container is empty.
  inline bool empty() const;

  // Number of entries in the container.
  inline size_type size() const;

  // ---------------------------- Modifiers ----------------------------
  // If `key` is mapped, return a mutable reference to its value.
  // Otherwise, insert a mapping from `key` to a default-constructed
  // value and return a reference to that value.
  inline VALUE &valueRefAtKey(KEY const &key);

  // ----------------------------- Lookup ------------------------------
  // True if `key` is mapped to some value.
  inline bool contains(KEY const &key) const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_FLAT_MAP_IFACE_H
