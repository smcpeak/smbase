// ordered-mru-flat-map.h
// `OrderedMRUFlatMap`, an `OrderedFlatMap` with a one-element MRU cache.

// This file is in the public domain.

// The tests for this module are in ordered-flat-map-test.cc.

#ifndef SMBASE_ORDERED_MRU_FLAT_MAP_H
#define SMBASE_ORDERED_MRU_FLAT_MAP_H

#include "ordered-mru-flat-map-iface.h"          // interface for this module

#include "smbase/chained-cond.h"                 // smbase::cc::z_le_lt
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/ordered-flat-map.h"             // OrderedFlatMap
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, DMEMB, MDMEMB, SELFCHECK

#include <utility>                               // std::{forward, move}

OPEN_NAMESPACE(smbase)


template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedMRUFlatMap<KEY, VALUE, COMPARE>::~OrderedMRUFlatMap()
{}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::selfCheck() const -> void
{
  Base::selfCheck();

  if (this->empty()) {
    xassert(m_mruIndex == 0);
  }
  else {
    xassert(cc::z_le_lt(m_mruIndex, this->size()));
  }
}


// --------------------------- Constructors ----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline OrderedMRUFlatMap<KEY, VALUE, COMPARE>::OrderedMRUFlatMap()
  : Base(),
    m_mruIndex(0)
{
  SELFCHECK();
}


template <typename KEY, typename VALUE, typename COMPARE>
OrderedMRUFlatMap<KEY, VALUE, COMPARE>::OrderedMRUFlatMap(OrderedMRUFlatMap const &obj)
  : Base(obj),
    DMEMB(m_mruIndex)
{
  SELFCHECK();
}


template <typename KEY, typename VALUE, typename COMPARE>
OrderedMRUFlatMap<KEY, VALUE, COMPARE>::OrderedMRUFlatMap(OrderedMRUFlatMap &&obj)
  : Base(std::move(obj)),
    MDMEMB(m_mruIndex)
{
  // The move ctor must leave `obj` in a valid state.
  obj.clear();

  SELFCHECK();
}


// -------------------------- Element access ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedMRUFlatMap<KEY, VALUE, COMPARE>::contains(KEY const &key) const
{
  return Base::contains(key);
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedMRUFlatMap<KEY, VALUE, COMPARE>::valueAtKey(KEY const &key) const
{
  return Base::valueAtKey(key);
}


template <typename KEY, typename VALUE, typename COMPARE>
KEY const &OrderedMRUFlatMap<KEY, VALUE, COMPARE>::keyAtIndex(size_type index) const
{
  return Base::keyAtIndex(index);
}


template <typename KEY, typename VALUE, typename COMPARE>
VALUE const &OrderedMRUFlatMap<KEY, VALUE, COMPARE>::valueAtIndex(size_type index) const
{
  return Base::valueAtIndex(index);
}


// ----------------------------- Iterators -----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::cbegin() const -> const_iterator
{
  return Base::cbegin();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::cend() const -> const_iterator
{
  return Base::cend();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::begin() const -> const_iterator
{
  return Base::begin();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::end() const -> const_iterator
{
  return Base::end();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::begin() -> iterator
{
  return Base::begin();
}


template <typename KEY, typename VALUE, typename COMPARE>
inline auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::end() -> iterator
{
  return Base::end();
}


// ----------------------------- Capacity ------------------------------
template <typename KEY, typename VALUE, typename COMPARE>
bool OrderedMRUFlatMap<KEY, VALUE, COMPARE>::empty() const
{
  return Base::empty();
}


template <typename KEY, typename VALUE, typename COMPARE>
auto OrderedMRUFlatMap<KEY, VALUE, COMPARE>::size() const -> size_type
{
  return Base::size();
}


// ----------------------------- Modifiers -----------------------------
template <typename KEY, typename VALUE, typename COMPARE>
VALUE &OrderedMRUFlatMap<KEY, VALUE, COMPARE>::valueRefAtKey(KEY const &key)
{
  if (m_mruIndex < this->size() &&
      this->equalKeys(this->m_sortedKeys.at(m_mruIndex), key)) {
    // Cache hit.
    return this->m_correspValues.at(m_mruIndex);
  }

  auto [index, found] = this->lookup(key);

  // Update cache.
  m_mruIndex = index;

  if (found) {
    return this->m_correspValues.at(index);
  }
  else {
    // Insert default-constructed value.
    return this->insertNewKeyAndValue(index, key, VALUE());
  }
}


template <typename KEY, typename VALUE, typename COMPARE>
template <typename K, typename V, typename>
bool OrderedMRUFlatMap<KEY, VALUE, COMPARE>::setValueAtKey(
  K &&key, V &&value)
{
  return Base::setValueAtKey(
    std::forward<K>(key), std::forward<V>(value));
}


template <typename KEY, typename VALUE, typename COMPARE>
template <typename K, typename V, typename>
void OrderedMRUFlatMap<KEY, VALUE, COMPARE>::setValueAtNewKey(
  K &&key, V &&value)
{
  return Base::setValueAtNewKey(
    std::forward<K>(key), std::forward<V>(value));
}


template <typename KEY, typename VALUE, typename COMPARE>
void OrderedMRUFlatMap<KEY, VALUE, COMPARE>::clear()
{
  Base::clear();
  m_mruIndex = 0;
}


// --------------------------- Serialization ---------------------------
template <typename KEY, typename VALUE, typename COMPARE>
OrderedMRUFlatMap<KEY, VALUE, COMPARE>::operator gdv::GDValue() const
{
  return Base::operator gdv::GDValue();
}


CLOSE_NAMESPACE(smbase)



#endif // SMBASE_ORDERED_MRU_FLAT_MAP_H
