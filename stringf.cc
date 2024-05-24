// stringf.cc
// Code for stringf module.

// This file is in the public domain.

#include "stringf.h"                   // this module

#include "xassert.h"                   // xassert

#include <vector>                      // std::vector

#include <stdarg.h>                    // va_list, etc.
#include <stdio.h>                     // vsnprintf


namespace smbase {


std::string stringf(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  std::string ret = vstringf(format, args);
  va_end(args);
  return ret;
}


std::string vstringf(char const *format, va_list args)
{
  // Calculate required size.
  int size;
  {
    va_list args2;
    va_copy(args2, args);
    size = vsnprintf(
      nullptr /*dest*/,
      0 /*size*/,
      format,
      args2);

    // It should not be possible for `vsnprintf` to fail, but evidently
    // very old glibc would return negative in this case.  I don't care
    // about supporting that configuration but I want to detect it.
    xassert(size >= 0);

    va_end(args2);
  }

  // Allocate space
  std::vector<char> buf(size+1);

  // Format the string.
  int writtenSize = vsnprintf(
    buf.data(),
    size+1,
    format,
    args);
  xassert(writtenSize == size);

  // The string we send back does not have the final NUL as part of the
  // string itself.
  return std::string(buf.data(), size);
}


} // namespace smbase


// EOF
