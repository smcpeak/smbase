// typ.h            see license.txt for copyright and terms of use
// File that I am in the process of removing entirely.

#ifndef SMBASE_TYP_H
#define SMBASE_TYP_H

// These days stdint.h should be common enough to rely on.  But for
// C++98 and C++03, at least on mingw, I need to explicitly request
// the limit and constant macros.
#define __STDC_LIMIT_MACROS            // INT64_MAX, etc.
#define __STDC_CONSTANT_MACROS         // INT64_C, etc.
#include <stdint.h>                    // uintptr_t

#include <stddef.h>                    // NULL


// division with rounding towards +inf
// (when operands are positive)
template <class T>
inline T div_up(T const &x, T const &y)
{ return (x + y - 1) / y; }


// mutable
#ifdef __BORLANDC__
#  define MUTABLE
#  define EXPLICIT
#else
#  define MUTABLE mutable
#  define EXPLICIT explicit
#endif


#define SWAP(a,b) \
  temp = a;       \
  a = b;          \
  b = temp /*user supplies semicolon*/


// verify something is true at compile time (will produce
// a compile error if it isn't)
// update: use STATIC_ASSERT defined in macros.h instead
//#define staticAssert(cond) extern int dummyArray[cond? 1 : 0]


#endif // SMBASE_TYP_H
