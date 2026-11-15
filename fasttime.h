// fasttime.h
// Very fast method to get a measure of current time for profling.

#ifndef SMBASE_FASTTIME_H
#define SMBASE_FASTTIME_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


/* Allow easy experimentation with `unsigned` vs. `std::atomic_uint`.

   Some data: On Windows 11 with an "AMD Ryzen 9 7900X 12-Core Processor
   (4.70 GHz)", using winlibs mingw gcc 13.2.0, with no optimization,
   `volatile unsigned` outperforms `std::atomic_uint` by about a factor
   of 6, but with "-O2", they both perform about the same, which is
   about 3x faster than `volatile unsigned` without optimization.  The
   disassemblies of the optimized inner loops are identical except that
   the order of the first two instructions is swapped.

   In that configuration, the timer can be sampled about 550 M times in
   about 110 ms, which is one sample per CPU cycle to within the
   measurement uncertainty (my CPU clock maxes at around 5.3 GHz).
*/
#define SMBASE_FASTTIME_USE_ATOMIC 1

#if SMBASE_FASTTIME_USE_ATOMIC
  #include <atomic>                    // std::atomic_uint
  #define SMBASE_FASTTIME_UNSIGNED std::atomic_uint
#else
  #define SMBASE_FASTTIME_UNSIGNED volatile unsigned
#endif


OPEN_NAMESPACE(smbase)


// Number of milliseconds since some arbitrary point in the past.
// After this module has been initialized, client code can read this
// value to get the time.
//
// There is no explicit separate synchronization for this variable.  We
// rely on being able to read it atomically.  On x86_64, an ordinary
// 32-bit aligned read ("mov") instruction accomplishes that.
extern SMBASE_FASTTIME_UNSIGNED fastTimeMilliseconds;


// For diagnostic use, a count of every 1000 iterations of the loop
// that updates 'fastTimeMilliseconds'.
extern SMBASE_FASTTIME_UNSIGNED fastTimeThread1000Loops;


// True if this module is implemented on the current platform.  If this
// returns false, then `fastTimeMilliseconds` is always zero.
bool fastTimeIsImplemented();


// Start the mechanism that keeps 'fastTimeMilliseconds' up to date.
// It is safe to call this more than once (it is idempotent, and very
// fast if already initialized).
void fastTimeInitialize();


// Simple accumulator based on 'fastTimeMilliseconds'.
struct FastTimeAccumulator {
  unsigned m_start;
  unsigned &m_acc;

  explicit FastTimeAccumulator(unsigned &acc)
    : m_start(fastTimeMilliseconds),
      m_acc(acc)
  {}

  ~FastTimeAccumulator()
  {
    m_acc += fastTimeMilliseconds - m_start;
  }
};


CLOSE_NAMESPACE(smbase)


// Unit tests, defined in fasttime-test.cc.
void test_fasttime();


#endif // SMBASE_FASTTIME_H
