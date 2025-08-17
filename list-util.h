// list-util.h
// Utilities for `std::list`.

#ifndef SMBASE_LIST_UTIL_H
#define SMBASE_LIST_UTIL_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/stringb.h"            // stringbc for xfailure_stringbc
#include "smbase/xassert.h"            // xassert

#include <cstddef>                     // std::size_t
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


// Return a reference to the element at position `index`.
template <typename T, typename A>
T const &listAtC(std::list<T,A> const &lst, std::size_t index)
{
  // Walk the list.
  auto iter = lst.begin();
  for (std::size_t i = 0; i < index && iter != lst.end(); ++i, ++iter)
    {}

  if (iter == lst.end()) {
    xfailure_stringbc(
      "listAt: List has fewer than " << index << " elements.");
  }

  return *iter;
}


// Same, but non-const.
template <typename T, typename A>
T &listAt(std::list<T,A> &lst, std::size_t index)
{
  return const_cast<T &>(listAtC(lst, index));
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_LIST_UTIL_H
