// set-util-iface.h
// Interface for `set-util.h`.

// One of the reasons this file exists is to allow TUs to get the
// declaration of `operator<<(set)`, which is needed if the `operator<<`
// for some other container that contains a set will be used.  That
// declaration may need to be included before certain other headers,
// which could be challenging when using the full header with
// implementation dependencies.

#ifndef SMBASE_SET_UTIL_IFACE_H
#define SMBASE_SET_UTIL_IFACE_H

#include "set-util-fwd.h"                        // fwds for this module

#include "smbase/iter-and-end-fwd.h"             // smbase::ConstIterAndEnd
#include "smbase/nway-comparison-result-fwd.h"   // smbase::NWayComparisonResult
#include "smbase/sm-macros.h"                    // DEPRECATED, OPEN_NAMESPACE
#include "smbase/sm-span-fwd.h"                  // smbase::Span
#include "smbase/std-optional-fwd.h"             // std::optional
#include "smbase/std-set-fwd.h"                  // stdfwd::set, std::set
#include "smbase/std-vector-fwd.h"               // stdfwd::vector

#include <cstddef>                               // std::size_t
#include <iosfwd>                                // std::ostream [n]


OPEN_NAMESPACE(smbase)


// Insert 't' into 's'.  Return true if it was inserted, false if it was
// already there.
template <typename T, typename C, typename A>
bool setInsert(std::set<T,C,A> &s, T const &t);


// Insert 't' into 's', requiring that it not already be there.
template <typename T, typename C, typename A>
void setInsertUnique(std::set<T,C,A> &s, T const &t);


// Insert all elements of `src` into `dest`.  Return the number of
// elements inserted, which may be less than `src.size()` if some of its
// elements were already present.
template <typename T, typename C1, typename A1,
                      typename C2, typename A2>
std::size_t setInsertAll(std::set<T,C1,A1> &dest,
                         std::set<T,C2,A2> const &src);


// Insert all elements of `src` into `dest`, moving from `src` where
// possible.  This does not return an element count due to an
// implementation issue, but could (with some effort) be modified to do
// so if needed.
template <typename K, typename C, typename A>
void setInsertAll_move(std::set<K,C,A> &dest,
                       std::set<K,C,A> &&src);


// Remove `t` from `s`.  Return true if `s` changed as a result.
template <typename T, typename C, typename A>
bool setErase(std::set<T,C,A> &s, T const &t);


// True if `k` is in `s`.
//
// There is a `contains` in `container-util.h` that also works, but in
// some cases I prefer to be explicit about the type involved.
//
// The key type is allowed to be different from `T` to allow the use of
// types implicitly convertible to or comparable with `T`.
//
template <typename T, typename C, typename A, typename KEY>
bool setContains(std::set<T,C,A> const &s, KEY const &k);


// True if every element in 'subset' is also in 'superset'.
template <typename T, typename C1, typename A1,
                      typename C2, typename A2>
bool isSubsetOf(std::set<T,C1,A1> const &subset,
                std::set<T,C2,A2> const &superset);


// If 'smaller' is a subset of 'larger', return true.  Otherwise, set
// 'extra' to one of the elements that is in 'smaller' but not in
// 'larger', and return false.
template <typename T, typename C1, typename A1,
                      typename C2, typename A2>
bool isSubsetOf_getExtra(T &extra /*OUT*/,
                         std::set<T,C1,A1> const &smaller,
                         std::set<T,C2,A2> const &larger);


// If there is an element in `smaller` that is not in `larger`, return
// the first such.
//
// This is basically the same as `isSubsetOf_getExtra` (with opposite
// return value sense), except it does not require an existing `T`
// object, which can be an issue when `T` lacks a default constructor.
//
template <typename T, typename C1, typename A1,
                      typename C2, typename A2>
std::optional<T> setHasElementNotIn(
  std::set<T,C1,A1> const &smaller,
  std::set<T,C2,A2> const &larger);


// True if there is no element in both `a` and `b`.
template <typename K, typename C, typename A>
bool setIsDisjointWith(std::set<K,C,A> const &a,
                       std::set<K,C,A> const &b);


/* True if there is no element common to any two of the sets of elements
   accessible via `iterAndEnds`, i.e., every element in their union is
   contained by exactly one set.

   To use this with, say, three sets, do something like this:

     ConstIterAndEnd<std::set<int>> iterAndEnds[] = {
       constIterAndEnd(set1),
       constIterAndEnd(set2),
       constIterAndEnd(set3),
     };
     bool areDisjoint = setsAreDisjoint(Span(iterAndEnds));

   NOTE: The iterators pointed to by the span are *modified* by this
   algorithm!  (But the sets they refer to are not.)  This can be
   exploited to learn which sets had equal elements; see
   `compareNSetIterators`.

   The run time is O(s * N) where `s` is the sum of the sizes of the
   sets and `N` is the number of sets.  It traverses all sets in
   parallel (O(s) steps), advancing the smallest iterator at each step
   (O(N) to find the smallest, O(1) to advance it).
*/
template <typename K, typename C, typename A>
bool setsAreDisjoint(
  Span<ConstIterAndEnd<std::set<K,C,A>>> iterAndEnds);


/* Compare N `IterAndEnd`s to find the one with the smallest value, or
   report that two are equal, or that no comparison is possible.

   This can be used after calling `setsAreDisjoint` to learn which sets
   had a common element and what it was, since that function leaves
   `iterAndEnds` with its iterators pointing at the spot that caused it
   to stop.
*/
template <typename K, typename C, typename A>
NWayComparisonResult compareNSetIterators(
  Span<ConstIterAndEnd<std::set<K,C,A>>> iterAndEnds,
  C const &isLessThan = C());


// Return a set containing the union of `a` and `b`.
template <typename K, typename C, typename A>
std::set<K,C,A> setUnion(std::set<K,C,A> const &a,
                         std::set<K,C,A> const &b);


// Remove from `larger` all elements in `smaller`.  Return the number of
// elements removed.
template <typename K, typename C, typename A>
std::size_t setRemoveMany(std::set<K,C,A> &larger,
                          std::set<K,C,A> const &smaller);


template <typename K, typename C, typename A>
void setInsertMany(std::set<K,C,A> &dest,
                   std::set<K,C,A> &&src)
  DEPRECATED("Use `setInsertAll_move` instead.");  // 2026-10-26


// Call 'func' on every element in 'input' and return the set of all of
// the results.
template <typename OELT,
          typename OC = std::less<OELT>,
          typename OA = std::allocator<OELT>,
          typename IELT, typename IC, typename IA,
          typename FUNC>
std::set<OELT,OC,OA> setMapElements(std::set<IELT,IC,IA> const &input,
                                    FUNC const &func);


// Return a vector containing the elements of 's' in natural order.
template <typename T, typename C, typename A>
stdfwd::vector<T> setToVector(std::set<T,C,A> const &s);


// Remove `k` from `s`.  Return true if it was there before.
template <typename K, typename C, typename A>
bool setRemove(std::set<K,C,A> &s, K const &k);


// Remove `k` from `s`, asserting that it was there before.
template <typename K, typename C, typename A>
void setRemoveExisting(std::set<K,C,A> &s, K const &k);


// Write `s` to `os`.  `printElement` should be like:
//
//   void printElement(std::ostream &os, T const &t);
//
// and write `t` to `os`.
template <typename T, typename C, typename A, typename PRINT_ELEMENT>
void setWrite(
  std::ostream &os,
  std::set<T,C,A> const &s,
  PRINT_ELEMENT const &printElement);


template <typename T, typename C, typename A>
std::ostream& operator<< (std::ostream &os, std::set<T,C,A> const &s);


// Object that can participate in an operator<< output chain.
template <typename T, typename C, typename A, typename PRINT_ELEMENT>
class SetWriter {
public:      // data
  // Set to write.
  std::set<T,C,A> const &m_set;

  // Element printer.
  PRINT_ELEMENT const &m_printElement;

public:      // methods
  inline SetWriter(std::set<T,C,A> const &s, PRINT_ELEMENT const &pe);

  inline void write(std::ostream &os) const;

  friend std::ostream &operator<< (std::ostream &os, SetWriter const &obj)
  {
    obj.write(os);
    return os;
  }
};


// Make a 'SetWriter' object, deducing its template arguments.  `pe` is
// like `printElement` in `setWrite`.
template <typename T, typename C, typename A, typename PRINT_ELEMENT>
SetWriter<T,C,A,PRINT_ELEMENT> setWriter(
  std::set<T,C,A> const &s,
  PRINT_ELEMENT const &pe);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SET_UTIL_IFACE_H
