// overflow.h
// Arithmetic that throws `XOverflow` on overflow.

// This module avoids actually performing an overflowing operation.
// Rather, it detects when the result of an operation would be outside
// the representable range.  This is important for the signed
// fundamental integral types since, for them, any arithmetic operation
// that would overflow causes undefined behavior.

// This module also is intended to be generically applicable to any
// integral type, including user-defined types.  However, I have not
// tested with any user-defined types.

#ifndef SMBASE_OVERFLOW_H
#define SMBASE_OVERFLOW_H

#include "exc.h"                       // DEFINE_XBASE_SUBCLASS
#include "get-type-name.h"             // smbase::GetTypeName
#include "str.h"                       // stringBuilder, stringb
#include "xoverflow.h"                 // XBinaryOpOverflow, XNumericConversionLosesRange, XNumericConversionChangesSign

#include <limits>                      // std::numeric_limits
#include <optional>                    // std::optional
#include <type_traits>                 // std::is_unsigned


// Throw an exception when overflow would happen.
template <class NUM>
void detectedOverflow(NUM a, NUM b, char op)
{
  THROW(smbase::XBinaryOpOverflow(
    std::string(smbase::GetTypeName<NUM>::value),

    // Prefix operands with `+` so they print as integers even if they
    // are a `char` type.
    stringb(+a),
    stringb(+b),
    stringb(op)));
}


// Add two numbers.  Return `std::nullopt` if they would overflow.
template <class NUM>
std::optional<NUM> addWithOverflowCheckOpt(NUM a, NUM b)
{
  if (a >= 0) {
    NUM largest_b = std::numeric_limits<NUM>::max() - a;
    if (b > largest_b) {
      return std::nullopt;
    }
  }
  else {
    // Here, "smallest" means "most negative".
    NUM smallest_b = std::numeric_limits<NUM>::min() - a;
    if (b < smallest_b) {
      return std::nullopt;
    }
  }

  return a + b;
}


// Add two numbers.  Throw `XOverflow` if they would overflow.
template <class NUM>
NUM addWithOverflowCheck(NUM a, NUM b)
{
  auto ret = addWithOverflowCheckOpt(a, b);
  if (!ret.has_value()) {
    detectedOverflow(a, b, '+');
  }
  return ret.value();
}


// Subtract two numbers.  Return `std::nullopt` if they would overflow.
template <class NUM>
std::optional<NUM> subtractWithOverflowCheckOpt(NUM a, NUM b)
{
  if constexpr (std::is_unsigned<NUM>::value) {
    if (a < b) {
      return std::nullopt;
    }
  }
  else {
    // Prototype example (PE): NUM is a 4-bit number.
    if (a >= 0) {                      // PE: a in [0,3]
      if (b >= 0) {                    // PE: b in [0,3]
        // Both non-negative, cannot overflow.
      }
      else /* b < 0 */ {               // PE: b in [-4,-1]
        // PE: `max()` is 3, `largest_minus_b` is in [0,3]
        NUM largest_minus_b = std::numeric_limits<NUM>::max() - a;

        //   a     -   b
        // = a +     (-b)
        // = a + 1 + (-b) - 1
        // = a + 1 + (-b  - 1)
        // = a + 1 + -(b  + 1)
        NUM mbp1 = -(b+1);             // PE: mbp1 in [0,3]

        if (mbp1 >= largest_minus_b) {
          return std::nullopt;
        }
      }
    }
    else /* a < 0 */ {                 // PE: a in [-4,-1]
      if (b >= 0) {                    // PE: b in [0,3]
        // PE: `min()` is -4, `smallest_minus_b` is in [-3,0]
        NUM smallest_minus_b = std::numeric_limits<NUM>::min() - a;

        // Since `b` is positive, we can safely invert it.
        if (-b < smallest_minus_b) {
          return std::nullopt;
        }
      }
      else /* b < 0 */ {
        // Both negative, cannot overflow.
      }
    }
  }

  return a - b;
}


// Subtract two numbers.  Throw `XOverflow` if they would overflow.
template <class NUM>
NUM subtractWithOverflowCheck(NUM a, NUM b)
{
  auto ret = subtractWithOverflowCheckOpt(a, b);
  if (!ret.has_value()) {
    detectedOverflow(a, b, '-');
  }
  return ret.value();
}


// Multiply two integers.  Return `std::nullopt` if they would overflow.
template <class NUM>
std::optional<NUM> multiplyWithOverflowCheckOpt(NUM a, NUM b)
{
  if (a == 0) {
    // 'a' is zero.  Result should not overflow.  I could
    // directly return 0, but I will let the normal multiply
    // do that in case it is a user-defined type.
  }
  else if (a > 0) {
    if (b > 0) {
      NUM largest_b = std::numeric_limits<NUM>::max() / a;
      if (b > largest_b) {
        return std::nullopt;
      }
    }
    else {
      NUM smallest_b = std::numeric_limits<NUM>::min() / a;
      if (b < smallest_b) {
        return std::nullopt;
      }
    }
  }
  else if (a < 0 /*implies 'a' is signed*/ && -a == 1) {
    // Here I am assuming the number is represented in two's complement
    // form, which implies the magnitude of min() is one less than the
    // magnitude of max().  I'm not sure how to generalize this logic
    // to arbitrary representation limits without incurring extra
    // requirements on NUM like being able to compute the magnitude.
    if (b == std::numeric_limits<NUM>::min()) {
      return std::nullopt;
    }
  }
  else {
    if (b > 0) {
      NUM largest_b = std::numeric_limits<NUM>::min() / a;
      if (b > largest_b) {
        return std::nullopt;
      }
    }
    else {
      NUM smallest_b = std::numeric_limits<NUM>::max() / a;
      if (b < smallest_b) {
        return std::nullopt;
      }
    }
  }

  return a * b;
}


// Multiply two integers.  Throw `XOverflow` if they would overflow.
template <class NUM>
NUM multiplyWithOverflowCheck(NUM a, NUM b)
{
  auto res = multiplyWithOverflowCheckOpt(a, b);
  if (!res.has_value()) {
    detectedOverflow(a, b, '*');
  }
  return res.value();
}


// Get quotient and remainder, returning false on overflow or division
// by zero.
template <class NUM>
bool divideWithOverflowCheckOpt(
  NUM &quotient,
  NUM &remainder,
  NUM dividend,                        // aka numerator
  NUM divisor)                         // aka denominator
{
  if (divisor == 0) {
    return false;
  }

  if constexpr (std::is_signed<NUM>::value) {
    if (dividend == std::numeric_limits<NUM>::min() &&
        divisor == -1) {
      // The specific case of dividing the most negative integer by -1
      // overflows because the result would be the positive counterpart
      // of the dividend, which is not representable.
      return false;
    }
  }

  // All other cases are safe.
  quotient = dividend / divisor;
  remainder = dividend % divisor;
  return true;
}


// Get quotient and remainder, throwing `XOverflow` on overflow or
// division by zero.
//
// TODO: I should throw `XDivideByZero` in the latter case.
template <class NUM>
void divideWithOverflowCheck(
  NUM &quotient,
  NUM &remainder,
  NUM dividend,                        // aka numerator
  NUM divisor)                         // aka denominator
{
  if (!divideWithOverflowCheckOpt(
         quotient,
         remainder,
         dividend,
         divisor)) {
    detectedOverflow(dividend, divisor, '/');
  }
}


// Convert 'src' to type 'DEST', returning `std::nullopt` if it cannot
// be converted back without loss of information.
//
// This is not the same as being convertible without overflow, since
// converting -1 to unsigned is a form of overflow, but does not lose
// information.
template <class DEST, class SRC>
std::optional<DEST> convertWithoutLossOpt(SRC const &src)
{
  DEST dest = static_cast<DEST>(src);
  SRC s2 = static_cast<SRC>(dest);
  if (s2 == src) {
    // Round-trip conversion preserved information.
    return dest;
  }
  else {
    return std::nullopt;
  }
}


// Convert 'src' to type 'DEST', throwing XOverflow if it cannot be
// converted back without loss of information.
//
// TODO: Change this to return `dest` instead of passing by reference.
template <class DEST, class SRC>
void convertWithoutLoss(DEST &dest, SRC const &src)
{
  std::optional<DEST> destOpt(convertWithoutLossOpt<DEST>(src));
  if (!destOpt.has_value()) {
    // I'm repeating myself here in order to compute values used in the
    // exception object...
    dest = static_cast<DEST>(src);
    SRC s2 = static_cast<SRC>(dest);

    // Printing '+src', etc., ensures that types like 'char' will print
    // as numbers.
    THROW(smbase::XNumericConversionLosesRange(
      stringb(+src),
      stringb(+dest),
      stringb(+s2),
      sizeof(SRC),
      sizeof(DEST)));
  }

  dest = destOpt.value();
}


// Convert 'src' to 'dest', ensuring the value is exactly representable
// in the destination type.  If not, return `std::nullopt`.
//
// This is different from 'convertWithoutLossOpt' in that it requires
// the sign to be preserved.
template <class DEST, class SRC>
std::optional<DEST> convertNumberOpt(SRC const &src)
{
  std::optional<DEST> ret(convertWithoutLossOpt<DEST>(src));

  if (ret.has_value()) {
    DEST dest = ret.value();

    if ((dest < 0) != (src < 0)) {
      // The conversion changes sign.
      return std::nullopt;
    }
  }

  return ret;
}


// Convert 'src' to 'dest', ensuring the value is exactly representable
// in the destination type.
//
// This is different from 'convertWithoutLoss' in that it requires the
// sign to be preserved.
template <class DEST, class SRC>
DEST convertNumber(SRC const &src)
{
  std::optional<DEST> ret(convertNumberOpt<DEST>(src));

  if (!ret.has_value()) {
    DEST dest = static_cast<DEST>(src);
    THROW(smbase::XNumericConversionChangesSign(
      stringb(+src),
      stringb(+dest)));
  }

  return ret.value();
}


// Convenient alias for a common operation.
template <class SRC>
int safeToInt(SRC const &src)
{
  return convertNumber<int>(src);
}


// Unit tests, in overflow-test.cc.
void test_overflow();


#endif // SMBASE_OVERFLOW_H
