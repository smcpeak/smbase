// ordered-flat-map.h
// `OrderedFlatMap`, like `std::flat_map` but remembering insertion order.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_H
#define SMBASE_ORDERED_FLAT_MAP_H

#include "ordered-flat-map-iface.h"    // interface for this module

#include "smbase/chained-cond.h"       // smbase::cc::z_le_lt
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/overflow.h"           // convertNumber
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, MDMEMB, CMEMB, EMEMB, IMEMBFP, SELFCHECK
#include "smbase/vector-util.h"        // vecToElementSet, vecContains
#include "smbase/xassert.h"            // xassert, xassertdb, xfailure_stringbc, xassertPrecondition

#include <algorithm>                   // std::lower_bound
#include <exception>                   // std::exception
#include <set>                         // std::set
#include <tuple>                       // std::{get, tuple}
#include <utility>                     // std::{forward, move}


OPEN_NAMESPACE(smbase)


// ------------------ OrderedFlatMap::const_iterator -------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::const_iterator(const_iterator const &obj)
  : DMEMB(m_map),
    DMEMB(m_mapModificationCount),
    DMEMB(m_index)
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::const_iterator(OrderedFlatMap const &map, size_type index)
  : IMEMBFP(map),
    m_mapModificationCount(map.m_modificationCount),
    IMEMBFP(index)
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::operator=(const_iterator const &obj) -> const_iterator &
{
  xassertPrecondition(&m_map == &obj.m_map);
  CMEMB(m_mapModificationCount);
  CMEMB(m_index);
  return *this;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::isValid() const -> bool
{
  return m_mapModificationCount == m_map.m_modificationCount;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::operator==(const_iterator const &obj) const -> bool
{
  // Both iterators must be valid.
  xassertPrecondition(isValid());
  xassertPrecondition(obj.isValid());

  // Both iterators must refer to the same container.
  xassertPrecondition(&m_map == &(obj.m_map));

  return m_index == obj.m_index;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::operator!=(const_iterator const &obj) const -> bool
{
  return !operator==(obj);
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::isEnd() const -> bool
{
  xassertPrecondition(isValid());
  return m_index == m_map.size();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::operator++() -> const_iterator &
{
  xassertPrecondition(!isEnd());

  ++m_index;

  return *this;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::const_iterator::operator*() const -> const_reference
{
  xassertPrecondition(!isEnd());

  return const_reference(m_map.keyAtIndex(m_index),
                         m_map.valueAtIndex(m_index));
}


// --------------------- OrderedFlatMap::iterator ----------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::iterator(iterator const &obj)
  : DMEMB(m_iter)
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::iterator(OrderedFlatMap &map, size_type index)
  : m_iter(map, index)
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::operator=(iterator const &obj) -> iterator &
{
  CMEMB(m_iter);
  return *this;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::isValid() const -> bool
{
  return m_iter.isValid();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::operator==(iterator const &obj) const -> bool
{
  return EMEMB(m_iter);
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::operator!=(iterator const &obj) const -> bool
{
  return !operator==(obj);
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::isEnd() const -> bool
{
  return m_iter.isEnd();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::operator++() -> iterator &
{
  ++m_iter;
  return *this;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::iterator::operator*() const -> reference
{
  const_reference ret = *m_iter;

  // The lack of a `const` qualifier when we first accepted the
  // container reference justifies removing `const` here.
  return reference(ret.first, const_cast<VALUE&>(ret.second));
}


// -------------------------- OrderedFlatMap ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::~OrderedFlatMap()
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::selfCheck() const -> void
{
  std::set<KEY, COMPARE> keySet =
    vecToElementSet<KEY, COMPARE>(m_sortedKeys);

  // Invariant: All elements are unique.
  xassert(m_sortedKeys.size() == keySet.size());

  // All vectors have the same size.
  xassert(m_correspValues.size() == m_sortedKeys.size());
  xassert(m_insertionOrder.size() == m_sortedKeys.size());

  // Invariant: The set of values in `m_insertionOrder` is [0,size()-1].
  std::set<unsigned> insKeySet =
    vecToElementSet<unsigned>(m_insertionOrder);
  xassert(insKeySet.size() == m_sortedKeys.size());
  unsigned expect = 0;
  for (unsigned index : insKeySet) {
    xassert(index == expect);
    ++expect;
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
/*static*/ bool OrderedFlatMap<KEY, VALUE, COMPARE>::equalKeys(KEY const &k1, KEY const &k2)
{
  COMPARE lt;
  return !lt(k1, k2) && !lt(k2, k1);
}


// --------------------------- Constructors ----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::OrderedFlatMap()
  : m_sortedKeys(),
    m_correspValues(),
    m_insertionOrder()
{
  SELFCHECK();
}


template <typename KEY, typename VALUE, typename COMPARE>
OrderedFlatMap<KEY, VALUE, COMPARE>::OrderedFlatMap(OrderedFlatMap const &obj)
  : DMEMB(m_sortedKeys),
    DMEMB(m_correspValues),
    DMEMB(m_insertionOrder)
{
  SELFCHECK();
}


template <typename KEY, typename VALUE, typename COMPARE>
OrderedFlatMap<KEY, VALUE, COMPARE>::OrderedFlatMap(OrderedFlatMap &&obj)
  : MDMEMB(m_sortedKeys),
    MDMEMB(m_correspValues),
    MDMEMB(m_insertionOrder)
{
  // The move ctor must leave `obj` in a valid state, which for me means
  // that all invariants hold.  However, technically, the three move
  // ctors for the members could have left them with different sizes.
  // So, clear `obj` to ensure its invariants hold.
  obj.clear();

  SELFCHECK();
}


// -------------------------- Element access ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
auto OrderedFlatMap<KEY, VALUE, COMPARE>::lookup(KEY const &key) const
  -> std::tuple<size_type, bool>
{
  auto it = std::lower_bound(m_sortedKeys.begin(), m_sortedKeys.end(),
                             key, COMPARE());

  // This is guaranteed to be constant-time by the fact that we started
  // with a vector iterator and `lower_bound` returns the same type as
  // its parameters.  With C++20 I could do
  // `static_assert(std::contiguous_iterator<decltype(it)>);` to
  // double-check, but I'm currently targeting C++17.
  size_type index = it - m_sortedKeys.begin();

  bool found = (
    index < m_sortedKeys.size() &&
    equalKeys(m_sortedKeys.at(index), key)
  );

  return std::make_tuple(index, found);
}


template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedFlatMap<KEY, VALUE, COMPARE>::contains(KEY const &key) const
{
  return std::get<1>(lookup(key));
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedFlatMap<KEY, VALUE, COMPARE>::valueAtKey(KEY const &key) const
{
  auto [index, found] = lookup(key);

  // Key must be mapped.
  xassert(found);

  return m_correspValues.at(index);
}


template <typename KEY, typename VALUE, typename COMPARE>
KEY const &OrderedFlatMap<KEY, VALUE, COMPARE>::keyAtIndex(size_type index) const
{
  xassertPrecondition(cc::z_le_lt(index, m_insertionOrder.size()));
  return m_sortedKeys.at(m_insertionOrder.at(index));
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedFlatMap<KEY, VALUE, COMPARE>::valueAtIndex(size_type index) const
{
  xassertPrecondition(cc::z_le_lt(index, m_insertionOrder.size()));
  return m_correspValues.at(m_insertionOrder.at(index));
}


// ----------------------------- Iterators -----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::cbegin() const -> const_iterator
{
  return const_iterator(*this, 0);
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::cend() const -> const_iterator
{
  return const_iterator(*this, size());
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::begin() const -> const_iterator
{
  return cbegin();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::end() const -> const_iterator
{
  return cend();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::begin() -> iterator
{
  return iterator(*this, 0);
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::end() -> iterator
{
  return iterator(*this, size());
}


// ----------------------------- Capacity ------------------------------
template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedFlatMap<KEY, VALUE, COMPARE>::empty() const
{
  return m_sortedKeys.empty();
}


template <typename KEY, typename VALUE, typename COMPARE>
auto OrderedFlatMap<KEY, VALUE, COMPARE>::size() const -> size_type
{
  return m_sortedKeys.size();
}


// ----------------------------- Modifiers -----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
template <typename K, typename V>
VALUE &OrderedFlatMap<KEY, VALUE, COMPARE>::insertNewKeyAndValue(
  size_type index, K &&key, V &&value)
{
  unsigned unsigned_index = convertNumber<unsigned>(index);

  // Adding a key invalidates all iterators.
  ++m_modificationCount;

  try {
    // Insert into sorted maps at `index`.
    m_sortedKeys.insert(m_sortedKeys.begin() + index,
                        std::forward<K>(key));
    m_correspValues.insert(m_correspValues.begin() + index,
                           std::forward<V>(value));

    // Increment every element of `m_insertOrder` that is `index` or
    // larger in order to match the vector insertion we just did.
    for (unsigned &idx : m_insertionOrder) {
      if (idx >= index) {
        ++idx;
      }
    }

    // As a result of the previous step, no element of
    // `m_insertionOrder` equals `index`.
    xassertdb(!vecContains(m_insertionOrder, unsigned_index));

    // Append `index`, thereby recording when `key` was inserted.
    m_insertionOrder.push_back(unsigned_index);
  }

  catch (std::exception &x) {
    // If an exception is thrown above, this class's invariants might be
    // broken.  Restore them by clearing, then fail an assertion.
    this->clear();                     // gcov-ignore
    xfailure_stringbc(
      "Exception during OrderedFlatMap::insertNewKeyAndValue: " <<
      x.what());
  }

  // Return a reference to the value in the array.
  return m_correspValues.at(index);
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE &OrderedFlatMap<KEY, VALUE, COMPARE>::valueRefAtKey(KEY const &key)
{
  auto [index, found] = lookup(key);

  if (found) {
    return m_correspValues.at(index);
  }
  else {
    // Insert default-constructed value.
    return insertNewKeyAndValue(index, key, VALUE());
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
template <typename K, typename V, typename>
bool OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtKey(
  K &&key, V &&value)
{
  auto [index, found] = lookup(key);

  if (found) {
    m_correspValues.at(index) = std::forward<V>(value);
    return false;
  }

  else {
    insertNewKeyAndValue(index,
      std::forward<K>(key), std::forward<V>(value));
    return true;
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
template <typename K, typename V, typename>
void OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtNewKey(
  K &&key, V &&value)
{
  bool inserted =
    setValueAtKey(std::forward<K>(key), std::forward<V>(value));

  // Although we only learn about it afterwards, a violation here means
  // the precondition `!contains(key)` was false on entry.
  //
  // Note that the map contents have been altered here, so code that
  // recovers from this assertion needs to be a little cautious.
  xassertPrecondition(inserted);
}


template <typename KEY, typename VALUE, typename COMPARE>
void OrderedFlatMap<KEY, VALUE, COMPARE>::clear()
{
  // Note: This function is called in a few places where the class
  // invariants might be broken, and its purpose is to restore them by
  // any means necessary.  Consequently, this is the one public function
  // for which the class invariants are *not* a precondition.

  ++m_modificationCount;

  m_sortedKeys.clear();
  m_correspValues.clear();
  m_insertionOrder.clear();
}


// --------------------------- Serialization ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
OrderedFlatMap<KEY, VALUE, COMPARE>::operator gdv::GDValue() const
{
  using gdv::toGDValue;

  gdv::GDValue ret(gdv::GDVK_ORDERED_MAP);

  for (auto [key, value] : *this) {
    // Convert each key and value to GDV, then add them as a mapping in
    // `ret`.
    //
    // One subtlety is, although the keys are all distinct, it is
    // possible for two keys to yield the same GDV (they normally
    // shouldn't, but nothing prevents it).  If that happens, the second
    // (or later) mapping with a duplicate GDV key will be silently
    // discarded.
    ret.orderedMapInsertValueAt(
      toGDValue(key),
      toGDValue(value)
    );
  }

  return ret;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_FLAT_MAP_H
