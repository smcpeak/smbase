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


#define SWAP(a,b) \
  temp = a;       \
  a = b;          \
  b = temp /*user supplies semicolon*/


#endif // SMBASE_TYP_H
