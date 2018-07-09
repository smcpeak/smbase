// sm-noexcept.h
// Define NOEXCEPT macro.

// NOEXCEPT means a function does not throw.  It is compatible with
// C++11 and C++03.
//
// I would rather just transition to fully using C++11, but smbase is
// still stuck in C++03 land, mostly because of silly configuration
// issues, so for now I'll just make this compatibility shim.

#ifndef SM_NOEXCEPT_H
#define SM_NOEXCEPT_H

#if __cplusplus >= 201103L
  // C++ 11 or greater
  #define NOEXCEPT noexcept
#else
  // C++ 98 or 03
  #define NOEXCEPT throw()
#endif

#endif // SM_NOEXCEPT_H
