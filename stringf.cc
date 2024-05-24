// stringf.cc
// Code for stringf module.

// This file is in the public domain.

#include "stringf.h"                   // this module

#include "array.h"                     // Array
#include "nonport.h"                   // vnprintf

#include <stdarg.h>                    // va_list, etc.; vsprintf
#include <unistd.h>                    // write

using std::string;


string stringf(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  string ret = vstringf(format, args);
  va_end(args);
  return ret;
}


// this should eventually be put someplace more general...
#ifndef va_copy
  #ifdef __va_copy
    #define va_copy(a,b) __va_copy(a,b)
  #else
    #define va_copy(a,b) (a)=(b)
  #endif
#endif


string vstringf(char const *format, va_list args)
{
  // estimate string length
  va_list args2;
  va_copy(args2, args);
  int est = vnprintf(format, args2);
  va_end(args2);

  // allocate space
  Array<char> buf(est+1);

  // render the string
  int len = vsprintf(buf.ptr(), format, args);

  // check the estimate, and fail *hard* if it was low, to avoid any
  // possibility that this might become exploitable in some context
  // (do *not* turn this check off in an NDEGUG build)
  if (len > est) {
    // don't go through fprintf, etc., because the state of memory
    // makes that risky
    static char const msg[] =
      "fatal error: vnprintf failed to provide a conservative estimate,\n"
      "memory is most likely corrupted\n";

    // To suppress the unused-result warning with GCC-5.4, it is
    // necessary to both negate and cast to void.
    (void)!write(2 /*stderr*/, msg, strlen(msg));
    abort();
  }

  // happy
  return string(buf.ptrC());
}


// EOF
