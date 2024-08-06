// ordered-map.h
// A map where the entries are extrinsically ordered.

#ifndef SMBASE_ORDERED_MAP_H
#define SMBASE_ORDERED_MAP_H

#include "smbase/compare-util.h"       // compare, RET_IF_COMPARE, DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "smbase/map-util.h"           // keySet
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, [M]DMEMB, [M]CMEMB
#include "smbase/vector-util.h"        // vecToElementSet, vecEraseAt
#include "smbase/xassert.h"            // xassertPrecondition

#include <cstddef>                     // std::size_t
#include <initializer_list>            // std::initializer_list
#include <map>                         // std::map
#include <utility>                     // std::pair
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
// put them once into the map and again into the vector.  If the keys
// are large objects then a different data structure might be
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

  // Iterate over the map entries in extrinsic order.
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
    const_iterator(const_iterator const &obj)
      : DMEMB(m_map),
        DMEMB(m_mapModificationCount),
        DMEMB(m_index)
    {}

    const_iterator(OrderedMap const &map, size_type index)
      : m_map(map),
        m_mapModificationCount(map.m_modificationCount),
        m_index(index)
    {}

    // Assigning an iterator requires that both refer to the same map.
    const_iterator &operator=(const_iterator const &obj)
    {
      xassertPrecondition(&m_map == &obj.m_map);
      CMEMB(m_mapModificationCount);
      CMEMB(m_index);
      return *this;
    }

    // True if this iterator can still be used with the container.
    bool isValid() const
    {
      return m_mapModificationCount == m_map.m_modificationCount;
    }

    bool operator==(const_iterator const &obj) const
    {
      // Both iterators must be valid.
      xassertPrecondition(isValid());
      xassertPrecondition(obj.isValid());

      // Both iterators must refer to the same container.
      xassertPrecondition(&m_map == &(obj.m_map));

      return m_index == obj.m_index;
    }

    bool operator!=(const_iterator const &obj) const
    {
      return !operator==(obj);
    }

    bool isEnd() const
    {
      xassertPrecondition(isValid());
      return m_index == m_map.size();
    }

    const_iterator &operator++()
    {
      xassertPrecondition(!isEnd());

      ++m_index;

      return *this;
    }

    value_type const &operator*() const
    {
      xassertPrecondition(!isEnd());

      return m_map.entryAtIndex(m_index);
    }
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
  ~OrderedMap() {}

  void selfCheck() const
  {
    // Same elements.
    xassertdb(vecToElementSet(m_keyVector) == keySet(m_map));

    // Same size.
    xassert(m_keyVector.size() == m_map.size());
  }

  // -------------------------- Constructors ---------------------------
  OrderedMap()
    : m_map(),
      m_keyVector()
  {}

  OrderedMap(OrderedMap const &obj)
    : DMEMB(m_map),
      DMEMB(m_keyVector)
  {}

  OrderedMap(OrderedMap &&obj)
    : MDMEMB(m_map),
      MDMEMB(m_keyVector)
  {}

  OrderedMap(std::initializer_list<value_type> ilist);

  // --------------------------- Assignment ----------------------------
  OrderedMap &operator=(OrderedMap const &obj)
  {
    CMEMB(m_map);
    CMEMB(m_keyVector);
    return *this;
  }

  OrderedMap &operator=(OrderedMap &&obj)
  {
    MCMEMB(m_map);
    MCMEMB(m_keyVector);
    return *this;
  }

  // Although `vector` has `assign` methods, `OrderedMap` does not
  // because the internal design is such that such an `assign` method
  // would not be able to overwrite existing elements with new values,
  // and consequently would provide no benefit.  See:
  //
  // https://stackoverflow.com/questions/78831995/why-do-c-sequence-containers-have-an-assign-method-but-associative-container

  // ------------------------- Element access --------------------------
  // Return the entry pair at `key`.
  value_type const &entryAtKey(KEY const &key) const
  {
    auto it = m_map.find(key);
    xassertPrecondition(it != m_map.end());

    return (*it);
  }

  value_type &entryAtKey(KEY const &key)
  {
    OrderedMap const &ths = *this;
    return const_cast<value_type&>(ths.entryAtKey(key));
  }

  VALUE const &valueAtKey(KEY const &key) const
  {
    return entryAtKey(key).second;
  }

  VALUE &valueAtKey(KEY const &key)
  {
    return entryAtKey(key).second;
  }

  // Return the entry pair at `index`.
  value_type const &entryAtIndex(size_type index) const
  {
    xassertPrecondition(index < size());
    KEY const &key = m_keyVector.at(index);

    return entryAtKey(key);
  }

  value_type &entryAtIndex(size_type index)
  {
    OrderedMap const &ths = *this;
    return const_cast<value_type&>(ths.entryAtIndex(index));
  }

  VALUE const &valueAtIndex(size_type index) const
  {
    return entryAtIndex(index).second;
  }

  VALUE &valueAtIndex(size_type index)
  {
    return entryAtIndex(index).second;
  }

  // There is no operator[] here because there is not a way to specify
  // whether that is accepting a key or an index.

  // Return the index of the entry with `key`, which must exist.
  //
  // This performs a linear search.
  //
  size_type indexOfKey(KEY const &key) const
  {
    for (size_type i=0; i < m_keyVector.size(); ++i) {
      if (m_keyVector[i] == key) {
        return i;
      }
    }

    xfailurePrecondition("indexOfKey: key not found");
    return 0;      // Not reached.
  }

  // There is no `front` or `back` because they would be relatively
  // expensive in comparison to the corresponding `vector` methods.

  // There is no `data` method because the internal storage does not
  // permit access in that way.

  // ---------------------------- Iterators ----------------------------
  const_iterator cbegin() const
  {
    return const_iterator(*this, 0);
  }

  const_iterator cend() const
  {
    return const_iterator(*this, size());
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

  // TODO: Reverse iterators.

  // ---------------------------- Capacity -----------------------------
  bool empty() const
  {
    return m_keyVector.empty();
  }

  size_type size() const
  {
    return m_keyVector.size();
  }

  // TODO: max_size

  // There are no methods `reserve`, `capacity`, or `shrink_to_fit`
  // because those are used in optimizations that would be largely
  // ineffective here due to the presence of the map.

  // ---------------------------- Modifiers ----------------------------
  void clear()
  {
    ++m_modificationCount;

    m_map.clear();
    m_keyVector.clear();
  }

  // Insert an entry and return true if the key is not already present.
  // Otherwise, return false.
  //
  // If the entry is inserted, it is appended to the sequence.
  //
  bool insert(value_type const &entry)
  {
    ++m_modificationCount;

    if (m_map.insert(entry).second) {
      m_keyVector.push_back(entry.first);
      return true;
    }
    else {
      return false;
    }
  }

  // Insert an entry at a specific location.
  //
  // Preconditions:
  //
  //   * The key must not already be present.
  //
  //   * The index must be in [0,size()].
  //
  void insertAtIndex(size_type index, value_type const &entry)
  {
    ++m_modificationCount;

    if (m_map.insert(entry).second) {
      vecInsertAt(m_keyVector, index, entry.first);
    }
    else {
      xfailurePrecondition("insertAt: key is already mapped");
    }
  }

  // TODO: emplace

  // Remove `key` if it is present.  Return true if it was present, and
  // therefore was removed.
  //
  // This requires a linear search of the sequence vector.
  //
  bool eraseKey(KEY const &key)
  {
    ++m_modificationCount;

    if (m_map.erase(key)) {
      size_type i = indexOfKey(key);
      vecEraseAt(m_keyVector, i);
      return true;
    }
    else {
      return false;
    }
  }

  // Remove the entry at `index`, which must be within bounds.
  void eraseIndex(size_type index)
  {
    ++m_modificationCount;

    xassertPrecondition(index < size());

    xassert(m_map.erase(m_keyVector[index]));
    vecEraseAt(m_keyVector, index);
  }

  // There is no `push_back`, since `insert` does what it would do.

  // TODO: pop_back

  // There is no `resize` because the case of increasing the size does
  // not make sense.

  void swap(OrderedMap &obj)
  {
    ++m_modificationCount;

    m_map.swap(obj.m_map);
    m_keyVector.swap(obj.m_keyVector);
  }

  friend void swap(OrderedMap &a, OrderedMap &b)
  {
    a.swap(b);
  }

  // ----------------------------- Lookup ------------------------------
  size_type count(KEY const &key) const
  {
    return m_map.count(key);
  }

  // There is no `find` because it would be inefficient since it would
  // have to do a linear search to return an iterator.

  bool contains(KEY const &key) const
  {
    // I am currently targeting C++17, which does not have
    // `std::map::contains`.
    return m_map.count(key) != 0;
  }

  // TODO: equal_range, lower_bound, upper_bound

  // --------------------------- Comparison ----------------------------
  // Return <0, ==0, or >= depending on how `a` compares to `b`.
  //
  // Comparison is lexicographic over the sequence of pairs.
  //
  friend int compare(OrderedMap const &a, OrderedMap const &b)
  {
    return compareSequences(a, b);
  }

  // Relational operators: == != < > <= >=
  DEFINE_FRIEND_RELATIONAL_OPERATORS(OrderedMap)
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_MAP_H
