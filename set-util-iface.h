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

#include "set-util-fwd.h"              // fwds for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::optional
#include "smbase/std-set-fwd.h"        // stdfwd::set
#include "smbase/std-vector-fwd.h"     // stdfwd::vector

#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(smbase)


// Insert 't' into 's'.  Return true if it was inserted, false if it was
// already there.
template <class T>
bool setInsert(stdfwd::set<T> &s, T const &t);


// Insert 't' into 's', requiring that it not already be there.
template <class T>
void setInsertUnique(stdfwd::set<T> &s, T const &t);


// Insert all elements of `src` into `dest`.  Return true if at least
// one element was inserted.
template <class T>
bool setInsertAll(stdfwd::set<T> &dest, stdfwd::set<T> const &src);


// True if `k` is in `s`.
//
// There is a `contains` in `container-util.h` that also works, but in
// some cases I prefer to be explicit about the type involved.
//
// The key type is allowed to be different from `T` to allow the use of
// types implicitly convertible to or comparable with `T`.
//
template <class T, class KEY>
bool setContains(stdfwd::set<T> const &s, KEY const &k);


// True if every element in 'subset' is also in 'superset'.
template <class T>
bool isSubsetOf(stdfwd::set<T> const &subset, stdfwd::set<T> const &superset);


// If 'smaller' is a subset of 'larger', return true.  Otherwise, set
// 'extra' to one of the elements that is in 'smaller' but not in
// 'larger', and return false.
template <class T>
bool isSubsetOf_getExtra(T &extra /*OUT*/,
                         stdfwd::set<T> const &smaller,
                         stdfwd::set<T> const &larger);


// If there is an element in `smaller` that is not in `larger`, return
// the first such.
//
// This is basically the same as `isSubsetOf_getExtra` (with opposite
// return value sense), except it does not require an existing `T`
// object, which can be an issue when `T` lacks a default constructor.
//
template <class T>
std::optional<T> setHasElementNotIn(
  stdfwd::set<T> const &smaller,
  stdfwd::set<T> const &larger);


// Call 'func' on every element in 'input' and return the set of all of
// the results.
template <typename OELT, typename IELT, typename FUNC>
stdfwd::set<OELT> setMapElements(stdfwd::set<IELT> const &input,
                                 FUNC const &func);


// Return a vector containing the elements of 's' in natural order.
template <class T>
stdfwd::vector<T> setToVector(stdfwd::set<T> const &s);


// Write `s` to `os`.  `printElement` should be like:
//
//   void printElement(std::ostream &os, T const &t);
//
// and write `t` to `os`.
template <class T, class PRINT_ELEMENT>
void setWrite(
  std::ostream &os,
  stdfwd::set<T> const &s,
  PRINT_ELEMENT const &printElement);


template <class T>
std::ostream& operator<< (std::ostream &os, stdfwd::set<T> const &s);


// Object that can participate in an operator<< output chain.
template <class T, class PRINT_ELEMENT>
class SetWriter {
public:      // data
  // Set to write.
  stdfwd::set<T> const &m_set;

  // Element printer.
  PRINT_ELEMENT const &m_printElement;

public:      // methods
  inline SetWriter(stdfwd::set<T> const &s, PRINT_ELEMENT const &pe);

  inline void write(std::ostream &os) const;

  friend std::ostream &operator<< (std::ostream &os, SetWriter const &obj)
  {
    obj.write(os);
    return os;
  }
};


// Make a 'SetWriter' object, deducing its template arguments.  `pe` is
// like `printElement` in `setWrite`.
template <class T, class PRINT_ELEMENT>
SetWriter<T,PRINT_ELEMENT> setWriter(
  stdfwd::set<T> const &s,
  PRINT_ELEMENT const &pe);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SET_UTIL_IFACE_H
