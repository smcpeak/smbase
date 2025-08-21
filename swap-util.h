// swap-util.h
// Utilities related to `std::swap`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SWAP_UTIL_H
#define SMBASE_SWAP_UTIL_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <utility>                     // std::swap


OPEN_NAMESPACE(smbase)


/* If `a > b`, swap them.

   The call:

     swapIfGreaterThan(a, b);

   is equivalent to:

     std::tie(a, b) = std::minmax(a, b);

   but avoids repeating the arguments and is, IMO, more direct.

   Ensures: a <= b
*/
template <typename T>
void swapIfGreaterThan(T &a, T &b)
{
  using std::swap;

  // Write the test using less-than since that seems more canonical.
  if (b < a) {
    swap(a, b);
  }
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SWAP_UTIL_H
