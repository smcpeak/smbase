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

#include "stringb.h"                   // ostringstream
#include "xmsg.h"                      // DEFINE_XMSG_SUBCLASS

#include <limits>                      // std::numeric_limits
#include <typeinfo>                    // typeid

using std::ostream;


// I do not want to explicitly refer to 'std' when accessing
// numeric_limits because that would preclude someone from defining
// their own number class and providing a numeric_limits specialization
// for it (since only the standard library is permitted to put things
// into 'std').
using std::numeric_limits;


// Exception thrown when there would be an arithmetic overflow.
DEFINE_XMSG_SUBCLASS(XOverflow);


// Print 'n' to 'os' as digits rather than a character.
template <class NUM>
ostream &insertAsDigits(ostream &os, NUM n)
{
  if (sizeof(n) == 1) {
    os << (int)n;
  }
  else {
    os << n;
  }
  return os;
}


// Throw an exception when overflow would happen.
template <class NUM>
void detectedOverflow(NUM a, NUM b, char op)
{
  ostringstream oss;

  // Note: On GCC, name() returns a mangled type name, so for the
  // primitive types it will be like "i" for "int", "a" for "char",
  // etc.  That's not ideal for human readability.
  oss << "Arithmetic overflow of type \"" << typeid(b).name() << "\": ";

  insertAsDigits(oss, a) << ' ' << op << ' ';
  insertAsDigits(oss, b) << " would overflow.";
  throw XOverflow(oss.str());
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
  else if (a == -1) {
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


// Unit tests, in test-overflow.cc.
int test_overflow();


#endif // OVERFLOW_H
