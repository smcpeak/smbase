// ordered-set.h
// `OrderedSet`, a sequence of unique elements.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_ORDERED_SET_H
#define SMBASE_ORDERED_SET_H

#include "ordered-set-iface.h"         // interface for this module

#include "smbase/chained-cond.h"       // smbase::cc::z_le_lt
#include "smbase/compare-util.h"       // smbase::compare, RET_IF_COMPARE
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP, EMEMB, DMEMB, MDMEMB, MCMEMB, SWAP_MEMB
#include "smbase/xassert.h"            // xassert

#include <initializer_list>            // std::initializer_list
#include <map>                         // std::map
#include <utility>                     // std::move
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// -------------------------- const_iterator ---------------------------
template <typename T, typename INDEX>
OrderedSet<T, INDEX>::const_iterator::const_iterator(
  OrderedSet const &set, INDEX index)
:
  IMEMBFP(set),
  IMEMBFP(index)
{}


template <typename T, typename INDEX>
auto OrderedSet<T, INDEX>::const_iterator::operator=(
  const_iterator const &obj) -> const_iterator &
{
  xassert(&m_set == &obj.m_set);
  CMEMB(m_index);
  return *this;
}


template <typename T, typename INDEX>
bool OrderedSet<T, INDEX>::const_iterator::operator==(
  const_iterator const &obj) const
{
  return &m_set == &obj.m_set &&
         EMEMB(m_index);
}


template <typename T, typename INDEX>
T const &OrderedSet<T, INDEX>::const_iterator::operator*() const
{
  return m_set.atC(m_index);
}


template <typename T, typename INDEX>
auto OrderedSet<T, INDEX>::const_iterator::operator++()
  -> const_iterator &
{
  xassert(m_index < m_set.size());
  ++m_index;
  return *this;
}


template <typename T, typename INDEX>
auto OrderedSet<T, INDEX>::const_iterator::operator++(int)
  -> const_iterator
{
  const_iterator ret(*this);
  ++*this;
  return ret;
}


// ---------------------------- OrderedSet -----------------------------
template <typename T, typename INDEX>
OrderedSet<T, INDEX>::~OrderedSet()
= default;


template <typename T, typename INDEX>
OrderedSet<T, INDEX>::OrderedSet()
= default;


template <typename T, typename INDEX>
void OrderedSet<T, INDEX>::rebuildIndexToElt()
{
  m_indexToElt.clear();
  m_indexToElt.reserve(m_eltToIndex.size());
  for (auto const &kv : m_eltToIndex) {
    m_indexToElt.push_back(&kv.first);
  }
}


template <typename T, typename INDEX>
OrderedSet<T, INDEX>::OrderedSet(OrderedSet const &obj)
:
  DMEMB(m_eltToIndex),
  m_indexToElt()
{
  // Rebuild `m_indexToElt` since the one in `obj` has pointers into
  // *its* set, not mine.
  rebuildIndexToElt();
}


template <typename T, typename INDEX>
OrderedSet<T, INDEX>::OrderedSet(OrderedSet &&obj)
:
  MDMEMB(m_eltToIndex),
  MDMEMB(m_indexToElt)
{}


template <typename T, typename INDEX>
OrderedSet<T, INDEX>::OrderedSet(std::initializer_list<T> ilist)
:
  m_eltToIndex(),
  m_indexToElt()
{
  for (T const &t : ilist) {
    insert(t);
  }
}


template <typename T, typename INDEX>
void OrderedSet<T, INDEX>::selfCheck() const
{
  xassert(m_indexToElt.size() == m_eltToIndex.size());

  for (INDEX i = 0; i < INDEX(m_indexToElt.size()); ++i) {
    T const *eltPtr = m_indexToElt[i];

    auto it = m_eltToIndex.find(*eltPtr);
    xassert(it != m_eltToIndex.end());

    // In addition to checking the manifest correspondence, this check
    // succeeding for all elements implies that no element of
    // `m_indexToElt` is repeated.
    xassert(it->second == i);

    xassert(&it->first == eltPtr);
  }
}


// ------------------------------ Queries ------------------------------
template <typename T, typename INDEX>
int OrderedSet<T, INDEX>::compareTo(OrderedSet const &b) const
{
  auto const &a = *this;
  return compareSequences(a, b);
}


template <typename T, typename INDEX>
bool OrderedSet<T, INDEX>::empty() const
{
  return m_eltToIndex.empty();
}


template <typename T, typename INDEX>
INDEX OrderedSet<T, INDEX>::size() const
{
  return INDEX(m_eltToIndex.size());
}


template <typename T, typename INDEX>
bool OrderedSet<T, INDEX>::contains(T const &t) const
{
  return m_eltToIndex.find(t) != m_eltToIndex.end();
}


template <typename T, typename INDEX>
T const &OrderedSet<T, INDEX>::atC(INDEX index) const
{
  xassert(cc::z_le_lt(index, size()));
  return *( m_indexToElt.at(index) );
}


template <typename T, typename INDEX>
INDEX OrderedSet<T, INDEX>::getIndex(T const &t) const
{
  auto it = m_eltToIndex.find(t);
  xassert(it != m_eltToIndex.end());
  return it->second;
}


template <typename T, typename INDEX>
auto OrderedSet<T, INDEX>::cbegin() const -> const_iterator
{
  return const_iterator(*this, 0);
}


template <typename T, typename INDEX>
auto OrderedSet<T, INDEX>::cend() const -> const_iterator
{
  return const_iterator(*this, size());
}


template <typename T, typename INDEX>
OrderedSet<T, INDEX>::operator gdv::GDValue() const
{
  using gdv::toGDValue;

  gdv::GDValue seq(gdv::GDVK_SEQUENCE);

  for (T const &t : *this) {
    seq.sequenceAppend(toGDValue(t));
  }

  return seq;
}


// --------------------------- Modifications ---------------------------
template <typename T, typename INDEX>
OrderedSet<T, INDEX> &
OrderedSet<T, INDEX>::operator=(OrderedSet const &obj)
{
  if (this != &obj) {
    // Clear this first so no pointers dangle, even temporarily.
    m_indexToElt.clear();

    m_eltToIndex = obj.m_eltToIndex;

    rebuildIndexToElt();
  }
  return *this;
}


template <typename T, typename INDEX>
OrderedSet<T, INDEX> &
OrderedSet<T, INDEX>::operator=(OrderedSet &&obj)
{
  if (this != &obj) {
    MCMEMB(m_eltToIndex);
    MCMEMB(m_indexToElt);
  }
  return *this;
}


template <typename T, typename INDEX>
void OrderedSet<T, INDEX>::clear()
{
  m_indexToElt.clear();
  m_eltToIndex.clear();
}


template <typename T, typename INDEX>
INDEX OrderedSet<T, INDEX>::insert(T const &t)
{
  auto it = m_eltToIndex.find(t);
  if (it != m_eltToIndex.end()) {
    return it->second;
  }

  INDEX newIndex = size();

  auto result = m_eltToIndex.emplace(t, newIndex);
  xassert(result.second);

  auto const &entry = *( result.first );
  m_indexToElt.push_back(&( entry.first ));

  return newIndex;
}


template <typename T, typename INDEX>
INDEX OrderedSet<T, INDEX>::insertUnique(T const &t)
{
  xassert(!contains(t));

  return insert(t);
}


template <typename T, typename INDEX>
void OrderedSet<T, INDEX>::swapWith(OrderedSet &obj)
{
  SWAP_MEMB(m_eltToIndex);
  SWAP_MEMB(m_indexToElt);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_SET_H
