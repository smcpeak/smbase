// ordered-flat-map.h
// OrderedFlatMap, like `std::flat_map` but remembering insertion order.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_H
#define SMBASE_ORDERED_FLAT_MAP_H

#include "ordered-flat-map-iface.h"    // interface for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, MDMEMB, SELFCHECK
#include "smbase/vector-util.h"        // vecToElementSet
#include "smbase/xassert.h"            // xassert, xassertdb

#include <algorithm>                   // std::lower_bound
#include <set>                         // std::set


OPEN_NAMESPACE(smbase)


template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedFlatMap<KEY, VALUE, COMPARE>::~OrderedFlatMap()
{}


// TODO: Fold this into `vecToElementSet`.
template <typename T, typename C = std::less<T>>
std::set<T, C> vecToElementSetEx(std::vector<T> const &vec)
{
  std::set<T, C> ret;
  for (T const &t : vec) {
    ret.insert(t);
  }
  return ret;
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedFlatMap<KEY, VALUE, COMPARE>::selfCheck() const -> void
{
  std::set<KEY, COMPARE> keySet =
    vecToElementSetEx<KEY, COMPARE>(m_sortedKeys);

  // Invariant: All elements are unique.
  xassert(m_sortedKeys.size() == keySet.size());

  // All vectors have the same size.
  xassert(m_correspValues.size() == m_sortedKeys.size());
  xassert(m_insertionOrder.size() == m_sortedKeys.size());

  // Invariant: The set of keys in `m_insertionOrder` is the same as the
  // set of keys in `m_sortedKeys`.
  std::set<KEY, COMPARE> insKeySet =
    vecToElementSetEx<KEY, COMPARE>(m_insertionOrder);
  xassert(keySet == insKeySet);
}


template <typename KEY, typename VALUE, typename COMPARE>
auto OrderedFlatMap<KEY, VALUE, COMPARE>::lookup(KEY const &key) const -> size_type
{
  auto it = std::lower_bound(m_sortedKeys.begin(), m_sortedKeys.end(),
                             key, COMPARE());

  // TODO: Is this guaranteed to be constant-time?
  return it - m_sortedKeys.begin();
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
  SELFCHECK();
}


// -------------------------- Element access ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedFlatMap<KEY, VALUE, COMPARE>::valueAtKey(KEY const &key) const
{
  size_type index = lookup(key);

  // Key must be mapped.
  xassert(index < m_sortedKeys.size() &&
          equalKeys(m_sortedKeys.at(index), key));

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
VALUE &OrderedFlatMap<KEY, VALUE, COMPARE>::valueRefAtKey(KEY const &key)
{
  size_type index = lookup(key);

  if (index < m_sortedKeys.size() &&
      equalKeys(m_sortedKeys.at(index), key)) {
    return m_correspValues.at(index);
  }

  // Insert into sorted maps at `index`.
  m_sortedKeys.insert(m_sortedKeys.begin() + index, key);
  m_correspValues.insert(m_correspValues.begin() + index, VALUE());

  // Append to insertion order.
  m_insertionOrder.push_back(key);

  // Return a reference to the new default-constructed value.
  return m_correspValues.at(index);
}


// ------------------------------ Lookup -------------------------------
template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedFlatMap<KEY, VALUE, COMPARE>::contains(KEY const &key) const
{
  size_type index = lookup(key);

  return index < m_sortedKeys.size() &&
         equalKeys(m_sortedKeys.at(index), key);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_FLAT_MAP_H
