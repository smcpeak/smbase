// set-util.h
// Utilities related to `std::set`.

#ifndef SMBASE_SET_UTIL_H
#define SMBASE_SET_UTIL_H

#include "set-util-iface.h"                      // interface for this module

#include "smbase/chained-cond.h"                 // smbase::cc::z_le_lt
#include "smbase/iter-and-end.h"                 // smbase::ConstIterAndEnd
#include "smbase/nway-comparison-result.h"       // smbase::NWayComparisonResult
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/sm-span.h"                      // smbase::Span
#include "smbase/xassert.h"                      // xassert

#include <optional>                              // std::optional
#include <ostream>                               // std::ostream
#include <set>                                   // std::set
#include <vector>                                // std::vector


OPEN_NAMESPACE(smbase)


// NOTE: Comments describing these functions are in `set-util-iface.h`.


template <class T>
bool setInsert(std::set<T> &s, T const &t)
{
  auto res = s.insert(t);
  return res.second;
}


template <class T>
void setInsertUnique(std::set<T> &s, T const &t)
{
  bool inserted = setInsert(s, t);
  xassert(inserted);
}


template <class T>
bool setInsertAll(std::set<T> &dest, std::set<T> const &src)
{
  bool ret = false;

  for (auto const &v : src) {
    ret |= setInsert(dest, v);
  }

  return ret;
}


template <typename T, typename C, typename A>
bool setErase(std::set<T,C,A> &s, T const &t)
{
  return s.erase(t) > 0;
}


template <class T, class KEY>
bool setContains(stdfwd::set<T> const &s, KEY const &k)
{
  return s.count(k) > 0;
}


template <class T>
bool isSubsetOf(std::set<T> const &subset, std::set<T> const &superset)
{
  for (auto it = subset.begin(); it != subset.end(); ++it) {
    auto it2 = superset.find(*it);
    if (it2 == superset.end()) {
      // '*it' is in 'subset' but not in 'superset'.
      return false;
    }
  }
  return true;
}


template <class T>
bool isSubsetOf_getExtra(T &extra /*OUT*/,
                         std::set<T> const &smaller,
                         std::set<T> const &larger)
{
  for (T const &element : smaller) {
    if (!setContains(larger, element)) {
      extra = element;
      return false;
    }
  }

  return true;
}


template <class T>
std::optional<T> setHasElementNotIn(
  std::set<T> const &smaller,
  std::set<T> const &larger)
{
  for (T const &element : smaller) {
    if (!setContains(larger, element)) {
      return std::make_optional(element);
    }
  }

  return std::nullopt;
}


// I choose to keep this as a separate implementation from
// `setsAreDisjoint` because it is much simpler, possibly faster, has
// fewer dependencies, and is useful for behavior comparison testing.
template <typename K, typename C, typename A>
bool setIsDisjointWith(std::set<K,C,A> const &a,
                       std::set<K,C,A> const &b)
{
  // Comparator to use.
  C isLessThan;

  auto it_a = a.begin();
  auto it_b = b.begin();

  // Traverse the sets in parallel, advancing whichever iterator is
  // smaller at each step.
  while (it_a != a.end() && it_b != b.end()) {
    if (isLessThan(*it_a, *it_b)) {
      ++it_a;
    }
    else if (isLessThan(*it_b, *it_a)) {
      ++it_b;
    }
    else {
      // Found a match, sets are not disjoint.
      return false;
    }
  }

  // Disjoint.
  return true;
}


// Compare N `IterAndEnd` to find the one with the smallest value.
template <typename K, typename C, typename A>
NWayComparisonResult compareNSetIterators(
  Span<ConstIterAndEnd<std::set<K,C,A>>> iterAndEnds,
  C const &isLessThan)
{
  // The number of sets we are working with.
  int const N = static_cast<int>(iterAndEnds.size());

  // The number of iterators that were found to have reached their
  // ends.
  int numEnded = 0;

  // Index of the iterator with the smallest value, or -1 if no value
  // yet seen.
  int smallestIndex = -1;

  // Find the smallest element in `iterAndEnds`.
  for (int i=0; i < N; ++i) {
    ConstIterAndEnd<std::set<K,C,A>> &iae = iterAndEnds[i];

    if (iae.empty()) {
      ++numEnded;
    }

    else if (smallestIndex < 0) {
      smallestIndex = i;
    }

    else {
      ConstIterAndEnd<std::set<K,C,A>> &smallest =
        iterAndEnds[smallestIndex];

      if (isLessThan(*iae, *smallest)) {
        // New smallest.
        smallestIndex = i;
      }
      else if (isLessThan(*smallest, *iae)) {
        // Keep the current smallest.
      }
      else {
        // Equal values; sets are not disjoint.
        return NWayComparisonResult(smallestIndex, i);
      }
    }
  }

  if (numEnded >= N-1) {
    // Iterators exhausted, sets are disjoint.  (The only way that
    // `numEnded` would not be exactly `N-1` is if all of the sets
    // were empty at the start, in which case `numEnded` would be `N`
    // here.)
    return NWayComparisonResult();
  }

  xassert(smallestIndex >= 0);
  return NWayComparisonResult(smallestIndex);
}


template <typename K, typename C, typename A>
bool setsAreDisjoint(
  Span<ConstIterAndEnd<std::set<K,C,A>>> iterAndEnds)
{
  int const N = iterAndEnds.size();
  if (N < 2) {
    // Nothing to compare, so disjointness is assured.
    return true;
  }

  // Comparator to use.
  C isLessThan;

  // Iterate over all sets in parallel, advancing the smallest iterator
  // at each step.
  while (true) {
    NWayComparisonResult result =
      compareNSetIterators(iterAndEnds, isLessThan);
    if (result.hasEqualIndices()) {
      return false;          // Equal elements; not disjoint.
    }
    else if (result.isNull()) {
      return true;           // Exhausted iterators; disjooint.
    }
    else {
      int smallestIndex = result.getSmallest();
      xassert(cc::z_le_lt(smallestIndex, N));
      ++iterAndEnds[smallestIndex];
    }
  }

  // Not reached.
  return false;
}


template <typename K, typename C, typename A>
std::set<K,C,A> setUnion(std::set<K,C,A> const &a,
                         std::set<K,C,A> const &b)
{
  std::set<K,C,A> ret;

  for (K const &k : a) {
    ret.insert(k);
  }
  for (K const &k : b) {
    ret.insert(k);
  }

  return ret;
}


template <typename K, typename C, typename A>
std::size_t setRemoveMany(std::set<K,C,A> &larger,
                          std::set<K,C,A> const &smaller)
{
  std::size_t ret = 0;

  for (K const &k : smaller) {
    ret += larger.erase(k);
  }

  return ret;
}


template <typename K, typename C, typename A>
void setInsertMany(std::set<K,C,A> &dest,
                   std::set<K,C,A> &&src)
{
  // C++17 has a method to do this in one step.  But I can't get a count
  // of inserted elements this way.  If I decide I want the count, I'll
  // need to resort to either using node handles or performing a count
  // before and after.
  dest.merge(std::move(src));
}


template <typename OELT, typename IELT, typename FUNC>
std::set<OELT> setMapElements(std::set<IELT> const &input,
                              FUNC const &func)
{
  std::set<OELT> output;

  for (IELT const &ielt : input) {
    output.insert(func(ielt));
  }

  return output;
}


template <class T>
std::vector<T> setToVector(std::set<T> const &s)
{
  std::vector<T> ret;
  for (T const &elt : s) {
    ret.push_back(elt);
  }
  return ret;
}


template <typename K, typename C, typename A>
bool setRemove(std::set<K,C,A> &s, K const &k)
{
  return s.erase(k) > 0;
}


template <typename K, typename C, typename A>
void setRemoveExisting(std::set<K,C,A> &s, K const &k)
{
  bool erased = setRemove(s, k);
  xassert(erased);
}


template <class T, class PRINT_ELEMENT>
void setWrite(
  std::ostream &os,
  std::set<T> const &s,
  PRINT_ELEMENT const &printElement)
{
  os << '{';

  int ct = 0;
  for (auto const &e : s) {
    if (ct > 0) {
      os << ", ";
    }
    printElement(os, e);
    ++ct;
  }

  os << '}';
}


template <class T>
std::ostream& operator<< (std::ostream &os, std::set<T> const &s)
{
  setWrite(os, s,
    [](std::ostream &os, T const &t) -> void {
      os << t;
    });
  return os;
}


template <class T, class PRINT_ELEMENT>
SetWriter<T,PRINT_ELEMENT>::SetWriter(
  std::set<T> const &s,
  PRINT_ELEMENT const &pe)
  : m_set(s),
    m_printElement(pe)
{}


template <class T, class PRINT_ELEMENT>
void SetWriter<T,PRINT_ELEMENT>::write(std::ostream &os) const
{
  setWrite(os, m_set, m_printElement);
}


template <class T, class PRINT_ELEMENT>
SetWriter<T,PRINT_ELEMENT> setWriter(
  std::set<T> const &s,
  PRINT_ELEMENT const &pe)
{
  return SetWriter<T,PRINT_ELEMENT>(s, pe);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SET_UTIL_H
