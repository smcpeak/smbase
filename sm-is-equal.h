// sm-is-equal.h
// Check if two values are equal, returning false if exactly one is
// a negative integral type, and using `operator==` otherwise.

/* This file is CC BY-SA 4.0:

     https://creativecommons.org/licenses/by-sa/4.0/

   The code comes from this question by "plaisthos":

     https://stackoverflow.com/questions/59516741/compare-two-integers-for-equality-in-c-with-unknown-int-types

   and specifically this answer by "KamilCuk":

     https://stackoverflow.com/a/59516805/2659307

   I have then modified it:
     - Use my preferred style (indentation, etc.).
     - Add comments.
     - Pass by `const` reference instead of by value.
     - Rename to `is_equal`.
     - Put it in `smbase` namespace.
*/

#ifndef SMBASE_SM_IS_EQUAL_H
#define SMBASE_SM_IS_EQUAL_H

#include <type_traits>                 // std::{enable_if, is_integral, is_signed, common_type}

#include "sm-macros.h"                 // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)


// This one is used when the argument types are integral types with
// opposite signedness.
//
// If the `enable_if` check passes, the return type is `bool`.
template <typename A, typename B>
typename std::enable_if<
  std::is_integral<A>::value &&
  std::is_integral<B>::value &&
  (std::is_signed<A>::value ^ std::is_signed<B>::value)
, bool>::type
is_equal(A const &a, B const &b)
{
  using C = typename std::common_type<A, B>::type;

  // Confirm they have the same sign.  Then convert both to `C` to avoid
  // a warning about comparing values with different signedness.
  return ((a >= 0) == (b >= 0)) &&
         static_cast<C>(a) == static_cast<C>(b);
}


// This is used in all other cases, including when the types are not
// integers or even primitives.
template<typename A, typename B>
typename std::enable_if<
  !(
    std::is_integral<A>::value &&
    std::is_integral<B>::value &&
    (std::is_signed<A>::value ^ std::is_signed<B>::value)
  )
, bool>::type
is_equal(A const &a, B const &b)
{
  return a == b;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_IS_EQUAL_H
