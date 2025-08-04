// safe-int-conv.h
// Safe integer conversion.

// See license.txt for copyright and terms of use.

// This overlaps somewhat with the purpose of the `overflow` module.
// My thinking is `overflow` should concentrate on arithmetic (and
// perhaps be renamed), while this module would only do conversions.
// Right now it's just metaprogramming checks, however.

#ifndef SMBASE_SAFE_INT_CONV_H
#define SMBASE_SAFE_INT_CONV_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <limits>                      // std::numeric_limits
#include <type_traits>                 // std::{is_integral_v, is_signed_v, etc.}


OPEN_NAMESPACE(smbase)


// `value` is true if all of the following are true:
//
// * Both `SRC` and `DEST` are integer types.
//
// * When an expression of type `SRC` is converted by `static_cast` to
//   an expression of type `DEST`, the result of such a conversion
//   represents the same numeric value.
//
template <typename SRC, typename DEST>
struct IsSafelyConvertible {
  static constexpr bool value =
    std::is_integral_v<SRC> &&
    std::is_integral_v<DEST> &&
    (
      // Both are signed.
      (std::is_signed_v<SRC> &&
       std::is_signed_v<DEST> &&
       std::numeric_limits<SRC>::min() >= std::numeric_limits<DEST>::min() &&
       std::numeric_limits<SRC>::max() <= std::numeric_limits<DEST>::max()) ||

      // `SRC` is signed and `DEST` is unsigned.  This is not
      // convertible since the `SRC` value could be negative.
      (std::is_signed_v<SRC> &&
       std::is_unsigned_v<DEST> &&
       false) ||

      // `SRC` is unsigned and `DEST` is signed.
      (std::is_unsigned_v<SRC> &&
       std::is_signed_v<DEST> &&

       // We need to take care here to do the comparison using unsigned
       // values on both sides.
       std::numeric_limits<SRC>::max() <=
         static_cast<std::make_unsigned_t<DEST>>(std::numeric_limits<DEST>::max())) ||

      // Both are unsigned.
      (std::is_unsigned_v<SRC> &&
       std::is_unsigned_v<DEST> &&
       std::numeric_limits<SRC>::max() <= std::numeric_limits<DEST>::max())
    );
};

template <typename SRC, typename DEST>
inline constexpr bool IsSafelyConvertible_v =
  IsSafelyConvertible<SRC, DEST>::value;


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SAFE_INT_CONV_H
