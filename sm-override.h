// sm-override.h
// Define OVERRIDE macro.

#ifndef SM_OVERRIDE_H
#define SM_OVERRIDE_H

// See similar rationale in 'sm-noexcept.h'.  Basically, at least for
// now, smbase should be compatible with C++98 or C++11.

#if __cplusplus >= 201103L
  // C++ 11 or greater
  #define OVERRIDE override
#else
  // C++ 98 or 03
  #define OVERRIDE /*nothing*/
#endif

#endif // SM_OVERRIDE_H
