// sm-intcmp.h
// Integer comparisons that work despite signedness mismatch.

// This is similar to the `cmp_XXX` functions in C++20 <utility>, but
// intended to be extensible to wrapped integer types.

// The implementations are loosely based on the "Possible
// implementation" at:
//
// https://en.cppreference.com/w/cpp/utility/intcmp.html

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SM_INTCMP_H
#define SMBASE_SM_INTCMP_H

#include <type_traits>                 // std::{is_signed, make_unsigned_t}

#include "sm-macros.h"                 // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)


// `strcmp`-like comparison.
template <typename T, typename U>
constexpr int intcmp_compare(T t, U u) noexcept
{
  if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
    return t < u? -1 :
           u < t? +1 :
                   0 ;
  }
  else if constexpr (std::is_signed_v<T>) {
    return t < 0? -1 :
           intcmp_compare(std::make_unsigned_t<T>(t), u);
  }
  else {
    return u < 0? +1 :
           intcmp_compare(t, std::make_unsigned_t<U>(u));
  }
}


template <typename T, typename U>
constexpr bool intcmp_equal(T t, U u) noexcept
{
  return intcmp_compare(t, u) == 0;
}


template <typename T, typename U>
constexpr bool intcmp_not_equal(T t, U u) noexcept
{
  return intcmp_compare(t, u) != 0;
}


template <typename T, typename U>
constexpr bool intcmp_less(T t, U u) noexcept
{
  return intcmp_compare(t, u) < 0;
}


template <typename T, typename U>
constexpr bool intcmp_greater(T t, U u) noexcept
{
  return intcmp_compare(t, u) > 0;
}


template <typename T, typename U>
constexpr bool intcmp_less_equal(T t, U u) noexcept
{
  return intcmp_compare(t, u) <= 0;
}


template <typename T, typename U>
constexpr bool intcmp_greater_equal(T t, U u) noexcept
{
  return intcmp_compare(t, u) >= 0;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_INTCMP_H
