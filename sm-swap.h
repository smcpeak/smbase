// sm-swap.h
// Define 'swap' template.

#ifndef SM_SWAP_H
#define SM_SWAP_H

// I will just include the standard definition.  If I get annoyed by
// compilation times I can revisit this header.
#if __cplusplus >= 201103L
  // C++ 11 or greater
  #include <utility>
#else
  // C++ 98 or 03
  #include <algorithm>
#endif

using std::swap;

#endif // SM_SWAP_H
