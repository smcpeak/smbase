// ordered-map.h
// A map where the entries are extrinsically ordered.

#ifndef SMBASE_ORDERED_MAP_H
#define SMBASE_ORDERED_MAP_H

#include "ordered-map-fwd.h"           // fwds for this module

#include "smbase/compare-util-iface.h" // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "smbase/sm-macros.h"          // CLOSE_NAMESPACE, OPEN_NAMESPACE
#include "smbase/std-utility-fwd.h"    // std::pair [n]

#include <cstddef>                     // std::size_t
#include <initializer_list>            // std::initializer_list [n]
#include <map>                         // std::map
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// A map where the entries are extrinsically ordered.
//
// The order induced by the less-than operator on `KEY` is its
// intrinsic order, and is used for lookup purposes.
//
// But this container also stores an extrinsic order, which typically
// represents the insertion order, but can be rearranged.  The map used
// for lookup by key is kept synchronized with the vector used to store
// the order information.
//
// For simplicitly, this container duplicates the storage of the keys,
// putting them once into the map and again into the vector.  If the
// keys are large objects then a different data structure might be
// preferable.
//
// Moreover, the use of the key's intrinsic order for lookup is not
// particularly efficient in comparision to hash-based methods.
// However, hash function design is its own can of worms, so again for
// simplicity, we just use that intrinsic order.
//
template <typename KEY, typename VALUE>
class OrderedMap {
public:      // types
  // Value of one entry.  This is called `value_type` for compatibility
  // with the containers in the standard library despite the possible
  // confusion with `VALUE`.
  using value_type = std::pair<KEY const, VALUE>;

  // Type for container sizes and numeric (positional) indices.
  using size_type = std::size_t;

  // Iterate over the map entries in extrinsic order without modifying
  // anything.
  //
  // TODO: Allow random bidirectional access.
  //
  class const_iterator {
  private:     // data
    // The map we are iterating over.
    OrderedMap const &m_map;

    // Modification count of `m_map` at creation time.
    unsigned m_mapModificationCount;

    // Index of the next element in `m_keyVector`, or `size()` if this
    // is the end iterator.
    size_type m_index;

  public:      // methods
    inline const_iterator(const_iterator const &obj);

    // Begin iterating at `index`.
    inline const_iterator(OrderedMap const &map, size_type index);

    // Assigning an iterator requires that both refer to the same map.
    inline const_iterator &operator=(const_iterator const &obj);

    // True if this iterator can still be used with the container.
    inline bool isValid() const;

    inline bool operator==(const_iterator const &obj) const;
    inline bool operator!=(const_iterator const &obj) const;

    inline bool isEnd() const;

    inline const_iterator &operator++();

    inline value_type const &operator*() const;
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
    inline iterator(OrderedMap &map, size_type index);

    // Assigning an iterator requires that both refer to the same map.
    inline iterator &operator=(iterator const &obj);

    // True if this iterator can still be used with the container.
    inline bool isValid() const;

    inline bool operator==(iterator const &obj) const;
    inline bool operator!=(iterator const &obj) const;

    inline bool isEnd() const;

    inline iterator &operator++();

    inline value_type &operator*() const;
  };

private:     // data
  // Map for lookup by key, in intrinsic key order.
  std::map<KEY, VALUE> m_map;

  // Keys in extrinsic order.
  //
  // Invariants:
  //
  //   * The set of keys in `m_keyVector` is the same as the set of keys
  //     in `m_map`.
  //
  //   * The two containers have the same size.
  //
  //   * Consequently, there are no duplicates in `m_keyVector`.
  //
  std::vector<KEY> m_keyVector;

  // Number of times this container has been modified.  This is used to
  // detect the use of invalid iterators.  For now, every modification
  // invalidates all iterators.
  unsigned m_modificationCount = 0;

public:      // methods
  inline ~OrderedMap();

  inline void selfCheck() const;

  // -------------------------- Constructors ---------------------------
  inline OrderedMap();

  inline OrderedMap(OrderedMap const &obj);
  inline OrderedMap(OrderedMap &&obj);

  inline OrderedMap(std::initializer_list<value_type> ilist);

  // --------------------------- Assignment ----------------------------
  inline OrderedMap &operator=(OrderedMap const &obj);
  inline OrderedMap &operator=(OrderedMap &&obj);

  // Although `vector` has `assign` methods, `OrderedMap` does not
  // because the internal design is such that such an `assign` method
  // would not be able to overwrite existing elements with new values,
  // and consequently would provide no benefit.  See:
  //
  // https://stackoverflow.com/questions/78831995/why-do-c-sequence-containers-have-an-assign-method-but-associative-container

  // ------------------------- Element access --------------------------
  // Return the entry pair at `key`.
  inline value_type const &entryAtKey(KEY const &key) const;
  inline value_type &entryAtKey(KEY const &key);

  // Return the value at `key`.
  inline VALUE const &valueAtKey(KEY const &key) const;
  inline VALUE &valueAtKey(KEY const &key);

  // Return the entry pair at `index`.
  inline value_type const &entryAtIndex(size_type index) const;
  inline value_type &entryAtIndex(size_type index);

  // Return the value at `index`.
  inline VALUE const &valueAtIndex(size_type index) const;
  inline VALUE &valueAtIndex(size_type index);

  // There is no operator[] here because there is not a way to specify
  // whether that is accepting a key or an index.

  // Return the index of the entry with `key`, which must exist.
  //
  // This performs a linear search.
  //
  inline size_type indexOfKey(KEY const &key) const;

  // There is no `front` or `back` because they would be relatively
  // expensive in comparison to the corresponding `vector` methods.

  // There is no `data` method because the internal storage does not
  // permit access in that way.

  // ---------------------------- Iterators ----------------------------
  inline const_iterator cbegin() const;
  inline const_iterator cend() const;

  inline const_iterator begin() const;
  inline const_iterator end() const;

  inline iterator begin();
  inline iterator end();

  // TODO: Reverse iterators.

  // ---------------------------- Capacity -----------------------------
  // True if the container is empty.
  inline bool empty() const;

  // Number of entries in the container.
  inline size_type size() const;

  // TODO: max_size

  // There are no methods `reserve`, `capacity`, or `shrink_to_fit`
  // because those are used in optimizations that would be largely
  // ineffective here due to the presence of the map.

  // ---------------------------- Modifiers ----------------------------
  // Remove all entries.
  inline void clear();

  // Insert an entry and return true if the key is not already present.
  // Otherwise, return false.
  //
  // If the entry is inserted, it is appended to the sequence.
  //
  inline bool insert(value_type const &entry);
  inline bool insert(value_type      &&entry);

  // If `key` is already mapped, update its value and return false.
  // Otherwise, insert (append) a new entry that maps `key` to `value`
  // and return true.
  inline bool setValueAtKey(KEY const &key, VALUE const &value);
  inline bool setValueAtKey(KEY      &&key, VALUE      &&value);

  // Insert an entry at a specific location.
  //
  // Preconditions:
  //
  //   * The key must not already be present.
  //
  //   * The index must be in [0,size()].
  //
  inline void insertAtIndex(size_type index, value_type const &entry);

  // TODO: emplace

  // Remove `key` if it is present.  Return true if it was present, and
  // therefore was removed.
  //
  // This requires a linear search of the sequence vector.
  //
  inline bool eraseKey(KEY const &key);

  // Remove the entry at `index`, which must be within bounds.
  inline void eraseIndex(size_type index);

  // There is no `push_back`, since `insert` does what it would do.

  // TODO: pop_back

  // There is no `resize` because the case of increasing the size does
  // not make sense.

  inline void swap(OrderedMap &obj);

  friend void swap(OrderedMap &a, OrderedMap &b)
  {
    a.swap(b);
  }

  // ----------------------------- Lookup ------------------------------
  // Return the number of entries with `key`; always 0 or 1.
  inline size_type count(KEY const &key) const;

  // There is no `find` because it would be inefficient since it would
  // have to do a linear search to return an iterator.

  // True if `key` is mapped to some value.
  inline bool contains(KEY const &key) const;

  // TODO: equal_range, lower_bound, upper_bound

  // --------------------------- Comparison ----------------------------
  // Return <0, ==0, or >= depending on how `a` compares to `b`.
  //
  // Comparison is lexicographic over the sequence of pairs.
  //
  inline int compareTo(OrderedMap const &obj) const;

  friend int compare(OrderedMap const &a, OrderedMap const &b)
  {
    return a.compareTo(b);
  }

  // Relational operators: == != < > <= >=
  DEFINE_FRIEND_RELATIONAL_OPERATORS(OrderedMap)
};


// The methods declared above are defined in `ordered-map-ops.h`.


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_MAP_H
