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

#include <string>                      // std::string

#include <stdarg.h>                    // va_list


// TODO: Currently these are defined in str.cc but I should move them
// into a new stringf.cc.

// TODO: Can I annotate these with attributes that will allow the
// compiler to diagnose misuse?


// Build a string using a `printf` format string and arguments.
std::string stringf(char const *format, ...);

// Same but using a `va_list`.
std::string vstringf(char const *format, va_list args);


#endif // SMBASE_STRINGF_H
