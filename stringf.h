// stringf.h
// Printf-like construction of std::string.

// This file is in the public domain.

// The usual C++ method of formatting with <iomanip> is, IMO, very
// verbose.  It is also somewhat error-prone since you can set something
// like hexadecimal conversion mode and forget to reset it.  The
// `printf` interface is often better, particularly for formatting
// numbers.

#ifndef SMBASE_STRINGF_H
#define SMBASE_STRINGF_H

#include "sm-macros.h"                 // SM_PRINTF_ANNOTATION, OPEN_SMBASE_NAMESPACE

#include <string>                      // std::string

#include <stdarg.h>                    // va_list


OPEN_NAMESPACE(smbase)


// Build a string using a `printf` format string and arguments.
std::string stringf(char const *format, ...)
  SM_PRINTF_ANNOTATION(1, 2);

// Same but using a `va_list`.
std::string vstringf(char const *format, va_list args);


CLOSE_NAMESPACE(smbase)


SMBASE_GLOBAL_ALIAS(stringf)
SMBASE_GLOBAL_ALIAS(vstringf)


#endif // SMBASE_STRINGF_H
