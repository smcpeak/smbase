// sm-type-traits.h
// Some custom type traits classes on top of `<type_traits>`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SM_TYPE_TRAITS_H
#define SMBASE_SM_TYPE_TRAITS_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <type_traits>                 // module we are extending


OPEN_NAMESPACE(smbase)


// True iff `A` is const-qualified, and `B` is the type obtained by
// removing that qualifier from `A`.
//
// ChatGPT helped write this.
template <typename A, typename B>
struct IsConstAndNonConst
  : std::bool_constant<
      std::is_const<A>::value &&
      std::is_same<
        std::remove_const_t<A>, B
      >::value
    >
{};

template <typename A, typename B>
inline constexpr bool IsConstAndNonConst_v = IsConstAndNonConst<A,B>::value;


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_TYPE_TRAITS_H
