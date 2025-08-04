// most-sig-bit.cc
// Code for `most-sig-bit` module.

#include "most-sig-bit.h"              // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/xassert.h"            // xassertPrecondition

#include <cstdint>                     // std::uint64_t

#if defined(_MSC_VER)
  #include <intrin.h>                  // _BitScanReverse64
#endif


OPEN_NAMESPACE(smbase)


int mostSignificantBit_fallback(std::uint64_t n)
{
  std::uint64_t const one = 1;
  int pos = 0;
  if (n >= (one << 32)) { n >>= 32; pos += 32; }
  if (n >= (one << 16)) { n >>= 16; pos += 16; }
  if (n >= (one << 8))  { n >>= 8;  pos += 8;  }
  if (n >= (one << 4))  { n >>= 4;  pos += 4;  }
  if (n >= (one << 2))  { n >>= 2;  pos += 2;  }
  if (n >= (one << 1))  {           pos += 1;  }
  return pos;
}


int mostSignificantBit(std::uint64_t n)
{
  xassertPrecondition(n > 0);

  #if defined(__GNUC__) || defined(__clang__)
    return 63 - __builtin_clzll(n);

  #elif defined(_MSC_VER)
    // I have not tested this code.
    unsigned long index;
    _BitScanReverse64(&index, n);
    return static_cast<int>(index);

  #else
    // Fallback that does not use compiler built-ins.
    return mostSignificantBit_fallback(n);

  #endif
}


CLOSE_NAMESPACE(smbase)


// EOF
