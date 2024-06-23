// compare-util.h
// Utilities for implementing three-way compare.

#ifndef SMBASE_COMPARE_UTIL_H
#define SMBASE_COMPARE_UTIL_H

#include "compare-util-iface.h"        // interface for this module

#include <functional>                  // std::less


template <class NUM>
int compare(NUM const &a, NUM const &b)
{
  // Use this instead of '<' so this works with pointers without relying
  // on implementation-defined behavior.
  std::less<NUM> lt;

  if (lt(a, b)) {
    return -1;
  }
  else if (lt(b, a)) {
    return +1;
  }
  else {
    return 0;
  }
}


#endif // SMBASE_COMPARE_UTIL_H
