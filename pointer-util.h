// pointer-util.h
// Miscellaneous utilities related to pointer manipulation.

// This file is in the public domain.

#ifndef SMBASE_POINTER_UTIL_H
#define SMBASE_POINTER_UTIL_H

#include <stdint.h>                    // uintptr_t


// This used when I want to cast a pointer to an integer for something
// like hashing the address.  It need not be injective.
inline uintptr_t pointerToInteger(void const *p)
  { return (uintptr_t)p; }


// This can be used to compare two pointers, even when they do not point
// into the same object.
inline int comparePointerAddresses(void const *p, void const *q)
{
  // John Skaller points out that comparing addresses directly is
  // nonportable, and that std::less<> provides a solution (cppstd
  // 20.3.3p8).  But, I'm concerned about the portability of std::less
  // more than I am about the portability of address comparison.  The
  // existence of this function at least ensures I only have to change
  // one place.
  return p==q?                                       0 :
         pointerToInteger(p) < pointerToInteger(q)? -1 :      // would use std::less<> here
                                                    +1 ;
}


#endif // SMBASE_POINTER_UTIL_H
