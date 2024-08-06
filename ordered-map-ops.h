// ordered-map-ops.h
// Operations for `ordered-map` module.

#ifndef SMBASE_ORDERED_MAP_OPS_H
#define SMBASE_ORDERED_MAP_OPS_H

#include "ordered-map.h"               // interface for this module

#include "smbase/compare-util.h"       // compareSequences
#include "smbase/map-util.h"           // keySet
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, CMEMB, MCMEMB
#include "smbase/vector-util.h"        // vecEraseAt, vecInsertAt, vecToElementSet
#include "smbase/xassert.h"            // xassert, xassertPrecondition, xassertdb, xfailurePrecondition


OPEN_NAMESPACE(smbase)


// -------------------- OrderedMap::const_iterator ---------------------
template <typename KEY, typename VALUE>
inline OrderedMap<KEY, VALUE>::const_iterator::const_iterator(OrderedMap const &map, size_type index)
  : m_map(map),
    m_mapModificationCount(map.m_modificationCount),
    m_index(index)
{}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::operator=(const_iterator const &obj) -> const_iterator &
{
  xassertPrecondition(&m_map == &obj.m_map);
  CMEMB(m_mapModificationCount);
  CMEMB(m_index);
  return *this;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::isValid() const -> bool
{
  return m_mapModificationCount == m_map.m_modificationCount;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::operator==(const_iterator const &obj) const -> bool
{
  // Both iterators must be valid.
  xassertPrecondition(isValid());
  xassertPrecondition(obj.isValid());

  // Both iterators must refer to the same container.
  xassertPrecondition(&m_map == &(obj.m_map));

  return m_index == obj.m_index;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::operator!=(const_iterator const &obj) const -> bool
{
  return !operator==(obj);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::isEnd() const -> bool
{
  xassertPrecondition(isValid());
  return m_index == m_map.size();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::operator++() -> const_iterator &
{
  xassertPrecondition(!isEnd());

  ++m_index;

  return *this;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::const_iterator::operator*() const -> value_type const &
{
  xassertPrecondition(!isEnd());

  return m_map.entryAtIndex(m_index);
}


// ---------------------------- OrderedMap -----------------------------
template <typename KEY, typename VALUE>
inline OrderedMap<KEY, VALUE>::~OrderedMap()
{}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::selfCheck() const -> void
{
  // Same elements.
  xassertdb(vecToElementSet(m_keyVector) == keySet(m_map));

  // Same size.
  xassert(m_keyVector.size() == m_map.size());
}


template <typename KEY, typename VALUE>
OrderedMap<KEY, VALUE>::OrderedMap(
  std::initializer_list<value_type> ilist)
  : m_map(ilist),
    m_keyVector()
{
  // If this fails, there must have been a duplicate key.
  xassertPrecondition(m_map.size() == ilist.size());

  // The initial order is that of the initializer list.
  for (auto const &kv : ilist) {
    m_keyVector.push_back(kv.first);
  }

  selfCheck();
}


template <typename KEY, typename VALUE>
inline OrderedMap<KEY, VALUE>::OrderedMap()
  : m_map(),
    m_keyVector()
{}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::operator=(OrderedMap const &obj) -> OrderedMap &
{
  CMEMB(m_map);
  CMEMB(m_keyVector);
  return *this;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::operator=(OrderedMap &&obj) -> OrderedMap &
{
  MCMEMB(m_map);
  MCMEMB(m_keyVector);
  return *this;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::entryAtKey(KEY const &key) const -> value_type const &
{
  auto it = m_map.find(key);
  xassertPrecondition(it != m_map.end());

  return (*it);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::entryAtKey(KEY const &key) -> value_type &
{
  OrderedMap const &ths = *this;
  return const_cast<value_type&>(ths.entryAtKey(key));
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::valueAtKey(KEY const &key) const -> VALUE const &
{
  return entryAtKey(key).second;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::valueAtKey(KEY const &key) -> VALUE &
{
  return entryAtKey(key).second;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::entryAtIndex(size_type index) const -> value_type const &
{
  xassertPrecondition(index < size());
  KEY const &key = m_keyVector.at(index);

  return entryAtKey(key);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::entryAtIndex(size_type index) -> value_type &
{
  OrderedMap const &ths = *this;
  return const_cast<value_type&>(ths.entryAtIndex(index));
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::valueAtIndex(size_type index) const -> VALUE const &
{
  return entryAtIndex(index).second;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::valueAtIndex(size_type index) -> VALUE &
{
  return entryAtIndex(index).second;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::indexOfKey(KEY const &key) const -> size_type
{
  for (size_type i=0; i < m_keyVector.size(); ++i) {
    if (m_keyVector[i] == key) {
      return i;
    }
  }

  xfailurePrecondition("indexOfKey: key not found");
  return 0;      // Not reached.
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::cbegin() const -> const_iterator
{
  return const_iterator(*this, 0);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::cend() const -> const_iterator
{
  return const_iterator(*this, size());
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::begin() const -> const_iterator
{
  return cbegin();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::end() const -> const_iterator
{
  return cend();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::empty() const -> bool
{
  return m_keyVector.empty();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::size() const -> size_type
{
  return m_keyVector.size();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::clear() -> void
{
  ++m_modificationCount;

  m_map.clear();
  m_keyVector.clear();
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::insert(value_type const &entry) -> bool
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


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::insertAtIndex(size_type index, value_type const &entry) -> void
{
  ++m_modificationCount;

  if (m_map.insert(entry).second) {
    vecInsertAt(m_keyVector, index, entry.first);
  }
  else {
    xfailurePrecondition("insertAt: key is already mapped");
  }
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::eraseKey(KEY const &key) -> bool
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


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::eraseIndex(size_type index) -> void
{
  ++m_modificationCount;

  xassertPrecondition(index < size());

  xassert(m_map.erase(m_keyVector[index]));
  vecEraseAt(m_keyVector, index);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::swap(OrderedMap &obj) -> void
{
  ++m_modificationCount;

  m_map.swap(obj.m_map);
  m_keyVector.swap(obj.m_keyVector);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::count(KEY const &key) const -> size_type
{
  return m_map.count(key);
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::contains(KEY const &key) const -> bool
{
  // I am currently targeting C++17, which does not have
  // `std::map::contains`.
  return m_map.count(key) != 0;
}


template <typename KEY, typename VALUE>
inline auto OrderedMap<KEY, VALUE>::compareTo(OrderedMap<KEY, VALUE> const &obj) const -> int
{
  return compareSequences(*this, obj);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_MAP_OPS_H
