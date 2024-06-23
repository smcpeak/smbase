// dummy-printf.h
// Declare `dummy_printf`, a printf-like function that does nothing.
//
// Normally, clients `#include "sm-test.h"` to get this, but that file
// requires C++, so this is an alternative for C clients.

// This file is in the public domain.

#ifndef SMBASE_DUMMY_PRINTF_H
#define SMBASE_DUMMY_PRINTF_H

#include "sm-macros.h"     // SM_PRINTF_ANNOTATION


#ifdef __cplusplus
extern "C" {
#endif


// Function that has the same signature as `printf` but does nothing.
// In test code, it can be useful to use a macro to map `printf` to this
// to silence it by default.
//
// This is defined in sm-test.cc.
int dummy_printf(char const *fmt, ...)
  SM_PRINTF_ANNOTATION(1, 2);


#ifdef __cplusplus
}
#endif


#endif // SMBASE_DUMMY_PRINTF_H
