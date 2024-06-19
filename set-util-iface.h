// set-util-iface.h
// Interface for `set-util`.

#ifndef SMBASE_SET_UTIL_IFACE_H
#define SMBASE_SET_UTIL_IFACE_H

#include <iosfwd>                      // std::ostream [n]
#include <set>                         // std::set [n]


// True if every element in 'subset' is also in 'superset'.
template <class T>
bool isSubsetOf(std::set<T> const &subset, std::set<T> const &superset);


// Call 'func' on every element in 'input' and return the set of all of
// the results.
template <typename OELT, typename IELT, typename FUNC>
std::set<OELT> setMapElements(std::set<IELT> const &input,
                              FUNC const &func);


template <class T>
std::ostream& operator<< (std::ostream &os, std::set<T> const &s);


#endif // SMBASE_SET_UTIL_IFACE_H
