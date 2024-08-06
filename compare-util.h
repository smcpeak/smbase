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


template <class CONTAINER>
int compareSequences(CONTAINER const &a, CONTAINER const &b)
{
  auto aIt = a.cbegin();
  auto bIt = b.cbegin();

  while (aIt != a.cend() &&
         bIt != b.cend()) {
    RET_IF_COMPARE(*aIt, *bIt);

    ++aIt;
    ++bIt;
  }

  if (bIt != b.cend()) {
    // `a` is a prefix of `b`, so is less.
    return -1;
  }

  if (aIt != a.cend()) {
    return +1;
  }

  return 0;
}


#endif // SMBASE_COMPARE_UTIL_H
