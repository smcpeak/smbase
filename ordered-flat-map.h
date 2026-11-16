// ordered-flat-map.h
// OrderedFlatMap, like `std::flat_map` but remembering insertion order.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_H
#define SMBASE_ORDERED_FLAT_MAP_H

#include "ordered-flat-map-iface.h"    // interface for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, MDMEMB, SELFCHECK
#include "smbase/vector-util.h"        // vecToElementSet
#include "smbase/xassert.h"            // xassert, xassertdb, xfailure_stringbc

#include <algorithm>                   // std::lower_bound
#include <exception>                   // std::exception
#include <set>                         // std::set
#include <tuple>                       // std::{get, tuple}
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


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

  // Invariant: The set of keys in `m_insertionOrder` is the same as the
  // set of keys in `m_sortedKeys`.
  std::set<KEY, COMPARE> insKeySet =
    vecToElementSet<KEY, COMPARE>(m_insertionOrder);
  xassert(keySet == insKeySet);
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
  xassert(index < m_insertionOrder.size());
  return m_insertionOrder.at(index);
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedFlatMap<KEY, VALUE, COMPARE>::valueAtIndex(size_type index) const
{
  KEY const &key = this->keyAtIndex(index);
  return valueAtKey(key);
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
VALUE &OrderedFlatMap<KEY, VALUE, COMPARE>::insertNewKeyAndValue(
  size_type index, KEY const &key, VALUE &&value)
{
  try {
    // Insert into sorted maps at `index`.
    m_sortedKeys.insert(m_sortedKeys.begin() + index, key);
    m_correspValues.insert(m_correspValues.begin() + index,
                           std::move(value));

    // Append to insertion order.
    m_insertionOrder.push_back(key);
  }

  catch (std::exception &x) {
    // If an exception is thrown above, this class's invariants might be
    // broken.  Restore them by clearing, then fail an assertion.
    this->clear();
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
bool OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtKey(
  KEY const &key, VALUE const &value)
{
  auto [index, found] = lookup(key);

  if (found) {
    m_correspValues.at(index) = value;
    return false;
  }

  else {
    // Make a copy of `value` here since the callee moves it.
    insertNewKeyAndValue(index, key, VALUE(value));
    return true;
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtKey(
  KEY const &key, VALUE &&value)
{
  auto [index, found] = lookup(key);

  if (found) {
    m_correspValues.at(index) = std::move(value);
    return false;
  }

  else {
    insertNewKeyAndValue(index, key, std::move(value));
    return true;
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
void OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtNewKey(
  KEY const &key, VALUE const &value)
{
  bool inserted = setValueAtKey(key, value);
  xassert(inserted);
}


template <typename KEY, typename VALUE, typename COMPARE>
void OrderedFlatMap<KEY, VALUE, COMPARE>::setValueAtNewKey(
  KEY const &key, VALUE &&value)
{
  bool inserted = setValueAtKey(key, std::move(value));
  xassert(inserted);
}


template <typename KEY, typename VALUE, typename COMPARE>
void OrderedFlatMap<KEY, VALUE, COMPARE>::clear()
{
  // Note: Due to the way this function is called by the move ctor, the
  // members might (technically) not all have the same size here.

  m_sortedKeys.clear();
  m_correspValues.clear();
  m_insertionOrder.clear();
}


// ------------------------------ Lookup -------------------------------
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


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_FLAT_MAP_H
