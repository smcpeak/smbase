// overflow.h
// Arithmetic with overflow checks.

// This module avoids actually performing an overflowing operation.
// Rather, it detects when the result of an operation would be outside
// the representable range.  This is important for the signed
// fundamental integral types since, for them, any arithmetic operation
// that would overflow causes undefined behavior.

// This module also is intended to be generically applicable to any
// integral type, including user-defined types.  However, I have not
// tested with any user-defined types.

#ifndef OVERFLOW_H
#define OVERFLOW_H

#include "exc.h"                       // DEFINE_XBASE_SUBCLASS
#include "str.h"                       // stringBuilder, stringb

#include <limits>                      // std::numeric_limits
#include <type_traits>                 // std::is_unsigned
#include <typeinfo>                    // typeid

using std::ostream;


// I do not want to explicitly refer to 'std' when accessing
// numeric_limits because that would preclude someone from defining
// their own number class and providing a numeric_limits specialization
// for it (since only the standard library is permitted to put things
// into 'std').
using std::numeric_limits;
using std::is_unsigned;
using std::is_signed;


// Exception thrown when there would be an arithmetic overflow.
DEFINE_XBASE_SUBCLASS(XOverflow);


// Throw an exception when overflow would happen.
template <class NUM>
void detectedOverflow(NUM a, NUM b, char op)
{
  THROW(XOverflow(stringb(
    // Note: On GCC, name() returns a mangled type name, so for the
    // primitive types it will be like "i" for "int", "a" for "char",
    // etc.  That's not ideal for human readability.
    "Arithmetic overflow of type \"" << typeid(b).name() << "\": " <<

    // Prefix operands with `+` so they print as integers even if they
    // are a `char` type.
    +a << ' ' << op << ' ' << +b << " would overflow.")));
}


// Add two numbers and check that they do not overflow.
template <class NUM>
NUM addWithOverflowCheck(NUM a, NUM b)
{
  if (a >= 0) {
    NUM largest_b = numeric_limits<NUM>::max() - a;
    if (b > largest_b) {
      detectedOverflow(a, b, '+');
    }
  }
  else {
    // Here, "smallest" means "most negative".
    NUM smallest_b = numeric_limits<NUM>::min() - a;
    if (b < smallest_b) {
      detectedOverflow(a, b, '+');
    }
  }

  return a + b;
}


// Subtract two numbers and check that they do not overflow.
template <class NUM>
NUM subtractWithOverflowCheck(NUM a, NUM b)
{
  if (is_unsigned<NUM>::value) {
    if (a < b) {
      detectedOverflow(a, b, '-');
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
        NUM largest_minus_b = numeric_limits<NUM>::max() - a;

        //   a     -   b
        // = a +     (-b)
        // = a + 1 + (-b) - 1
        // = a + 1 + (-b  - 1)
        // = a + 1 + -(b  + 1)
        NUM mbp1 = -(b+1);             // PE: mbp1 in [0,3]

        if (mbp1 >= largest_minus_b) {
          detectedOverflow(a, b, '-');
        }
      }
    }
    else /* a < 0 */ {                 // PE: a in [-4,-1]
      if (b >= 0) {                    // PE: b in [0,3]
        // PE: `min()` is -4, `smallest_minus_b` is in [-3,0]
        NUM smallest_minus_b = numeric_limits<NUM>::min() - a;

        // Since `b` is positive, we can safely invert it.
        if (-b < smallest_minus_b) {
          detectedOverflow(a, b, '-');
        }
      }
      else /* b < 0 */ {
        // Both negative, cannot overflow.
      }
    }
  }

  return a - b;
}


// Multiply two integers, verifying that the result does not overflow.
template <class NUM>
NUM multiplyWithOverflowCheck(NUM a, NUM b)
{
  if (a == 0) {
    // 'a' is zero.  Result should not overflow.  I could
    // directly return 0, but I will let the normal multiply
    // do that in case it is a user-defined type.
  }
  else if (a > 0) {
    if (b > 0) {
      NUM largest_b = numeric_limits<NUM>::max() / a;
      if (b > largest_b) {
        detectedOverflow(a, b, '*');
      }
    }
    else {
      NUM smallest_b = numeric_limits<NUM>::min() / a;
      if (b < smallest_b) {
        detectedOverflow(a, b, '*');
      }
    }
  }
  else if (a < 0 /*implies 'a' is signed*/ && -a == 1) {
    // Here I am assuming the number is represented in two's complement
    // form, which implies the magnitude of min() is one less than the
    // magnitude of max().  I'm not sure how to generalize this logic
    // to arbitrary representation limits without incurring extra
    // requirements on NUM like being able to compute the magnitude.
    if (b == numeric_limits<NUM>::min()) {
      detectedOverflow(a, b, '*');
    }
  }
  else {
    if (b > 0) {
      NUM largest_b = numeric_limits<NUM>::min() / a;
      if (b > largest_b) {
        detectedOverflow(a, b, '*');
      }
    }
    else {
      NUM smallest_b = numeric_limits<NUM>::max() / a;
      if (b < smallest_b) {
        detectedOverflow(a, b, '*');
      }
    }
  }

  return a * b;
}


// Get quotient and remainder, throwing on overflow or division by zero.
template <class NUM>
void divideWithOverflowCheck(
  NUM &quotient,
  NUM &remainder,
  NUM dividend,                        // aka numerator
  NUM divisor)                         // aka denominator
{
  if (divisor == 0) {
    // Although division by zero is not usually described as an
    // "overflow", since there is no possible type that would be large
    // enough to represent the result, my idea for this module is that
    // it catches any unsafe operation and maps it to an `XOverflow`
    // exception.
    detectedOverflow(dividend, divisor, '/');
  }

  if (is_signed<NUM>::value) {
    if (dividend == numeric_limits<NUM>::min() &&
        divisor == -1) {
      // The specific case of dividing the most negative integer by -1
      // overflows because the result would be the positive counterpart
      // of the dividend, which is not representable.
      detectedOverflow(dividend, divisor, '/');
    }
  }

  // All other cases are safe.
  quotient = dividend / divisor;
  remainder = dividend % divisor;
}


// Convert 'src' to type 'DEST', throwing XOverflow if it cannot be
// converted back without loss of information.
//
// This is not the same as being convertible without overflow, since
// converting -1 to unsigned is a form of overflow, but does not lose
// information.
template <class DEST, class SRC>
void convertWithoutLoss(DEST &dest, SRC const &src)
{
  dest = static_cast<DEST>(src);
  SRC s2 = static_cast<SRC>(dest);
  if (s2 != src) {
    // Printing '+src', etc., ensures that types like 'char' will print
    // as numbers.
    THROW(XOverflow(stringb(
      "convertWithoutLoss: Source value " << +src <<
      " converts to destination value " << +dest <<
      " and back to different value " << +s2 <<
      " (ss=" << sizeof(SRC) << " ds=" << sizeof(DEST) << ").")));
  }
}


// Convert 'src' to 'dest', ensuring the value is exactly representable
// in the destination type.
//
// This is different from 'convertWithoutLoss' in that it requires the
// sign to be preserved.
template <class DEST, class SRC>
DEST convertNumber(SRC const &src)
{
  DEST dest;
  convertWithoutLoss(dest, src);

  if ((dest < 0) != (src < 0)) {
    THROW(XOverflow(stringb(
      "convertNumber: Source value " << +src <<
      " and destination value " << +dest <<
      " have different signs.")));
  }

  return dest;
}


// Unit tests, in overflow-test.cc.
void test_overflow();


#endif // OVERFLOW_H
