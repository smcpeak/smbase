// set-util-iface.h
// Interface for `set-util.h`.

// One of the reasons this file exists is to allow TUs to get the
// declaration of `operator<<(set)`, which is needed if the `operator<<`
// for some other container that contains a set will be used.

#ifndef SMBASE_SET_UTIL_IFACE_H
#define SMBASE_SET_UTIL_IFACE_H

#include "set-util-fwd.h"              // fwds for this module

#include <iosfwd>                      // std::ostream [n]
#include <set>                         // std::set [n]
#include <vector>                      // std::vector [n]


// Insert 't' into 's', return true if it was inserted (i.e., it was not
// already there).
template <class T>
bool setInsert(std::set<T> &s, T const &t);


// Insert 't' into 's', requiring that it not already be there.
template <class T>
void setInsertUnique(std::set<T> &s, T const &t);


template <class T>
void setInsertAll(std::set<T> &dest, std::set<T> const &src);


// True if every element in 'subset' is also in 'superset'.
template <class T>
bool isSubsetOf(std::set<T> const &subset, std::set<T> const &superset);


// If 'smaller' is a subset of 'larger', return true.  Otherwise, set
// 'extra' to one of the elements that is in 'smaller' but not in
// 'larger', and return false.
template <class T>
bool isSubsetOf_getExtra(T &extra /*OUT*/,
                         std::set<T> const &smaller,
                         std::set<T> const &larger);


// Call 'func' on every element in 'input' and return the set of all of
// the results.
template <typename OELT, typename IELT, typename FUNC>
std::set<OELT> setMapElements(std::set<IELT> const &input,
                              FUNC const &func);


// Return a vector containing the elements of 's' in natural order.
template <class T>
std::vector<T> setToVector(std::set<T> const &s);


// Write `s` to `os`.  `printElement` should be like:
//
//   void printElement(std::ostream &os, T const &t);
//
// and write `t` to `os`.
template <class T, class PRINT_ELEMENT>
void setWrite(
  std::ostream &os,
  std::set<T> const &s,
  PRINT_ELEMENT const &printElement);


template <class T>
std::ostream& operator<< (std::ostream &os, std::set<T> const &s);


// Object that can participate in an operator<< output chain.
template <class T, class PRINT_ELEMENT>
class SetWriter {
public:      // data
  // Set to write.
  std::set<T> const &m_set;

  // Element printer.
  PRINT_ELEMENT const &m_printElement;

public:      // methods
  inline SetWriter(std::set<T> const &s, PRINT_ELEMENT const &pe);

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
  std::set<T> const &s,
  PRINT_ELEMENT const &pe);


#endif // SMBASE_SET_UTIL_IFACE_H
