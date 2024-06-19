// set-util.h
// Utilities related to `std::set`.

#ifndef SMBASE_SET_UTIL_H
#define SMBASE_SET_UTIL_H

#include "container-util.h"            // CONTAINER_FOREACH

#include <ostream>                     // std::ostream
#include <set>                         // std::set


// True if every element in 'subset' is also in 'superset'.
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


// Call 'func' on every element in 'input' and return the set of all of
// the results.
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
std::ostream& operator<< (std::ostream &os, std::set<T> const &s)
{
  os << '{';

  bool first = true;
  CONTAINER_FOREACH(s, it) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << *it;
  }

  os << '}';
  return os;
}


#endif // SMBASE_SET_UTIL_H
