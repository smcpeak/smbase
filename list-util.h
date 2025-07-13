// list-util.h
// Utilities for `std::list`.

#ifndef SMBASE_LIST_UTIL_H
#define SMBASE_LIST_UTIL_H

#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "xassert.h"                   // xassert

#include <list>                        // std::list
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// Move the first element of `lst`, which must exist, out of the list,
// and return that value.
template <typename T, typename A>
T listMoveFront(std::list<T,A> &lst)
{
  xassert(!lst.empty());

  // Move the first value.
  T ret = std::move(lst.front());

  // Remove the now-indeterminate first value.
  lst.pop_front();

  return ret;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_LIST_UTIL_H
