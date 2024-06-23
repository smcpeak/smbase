// cycles-test.c
// Tests for cycles module.

#include "cycles.h"                    // module under test

#include "dummy-printf.h"              // dummy_printf

#include <stdio.h>                     // printf


// Silence test.
static int verbose = 0;
#define printf (verbose? printf : dummy_printf)


// Called from unit-tests.cc.
void test_cycles()
{
  // Here, I exclude GCC+MSVCRT (i.e., MinGW) because GCC issues
  // a spurious warning in that case for the "ll" format specifier.
  // The code should work, but 'getCycles' is mostly obsolete
  // anyway and I just want to silence the warning.
  #if defined(__GNUC__) && !defined(__MSVCRT__)
    unsigned long long v = getCycles_ll();
    printf("getCycles: %llu\n", v);
  #endif // __GNUC__ && !__MSVCRT__

  // this segment should work on any compiler, by virtue
  // of only using 32-bit quantities
  {
    unsigned low, high;
    getCycles(&low, &high);
    printf("getCycles high=%u, low=%u\n", high, low);
  }

  // test whether the instruction causes a privileged instruction
  // fault; on my machine I get 33 cycles per call, which clearly
  // is too few for it to be trapping on each one
  {
    unsigned low1, low2, low3, high;
    getCycles(&low1, &high);
    getCycles(&low2, &high);
    getCycles(&low3, &high);
    printf("three lows in a row: %u, %u, %u\n", low1, low2, low3);
  }
}


// EOF
