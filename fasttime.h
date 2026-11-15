// fasttime.h
// Very fast method to get a measure of current time for profling.

#ifndef SMBASE_FASTTIME_H
#define SMBASE_FASTTIME_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)


// Number of milliseconds since some arbitrary point in the past.
// After this module has been initialized, client code can read this
// value to get the time.
//
// There is no explicit synchronization for this variable.  We simply
// assume that it can be sampled and updated atomically due to being
// no larger than a machine word.
extern volatile unsigned fastTimeMilliseconds;


// For diagnostic use, a count of every 1000 iterations of the loop
// that updates 'fastTimeMilliseconds'.
extern volatile unsigned fastTimeThread1000Loops;


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


// TODO: Actually write unit tests.
//int test_fasttime();


#endif // SMBASE_FASTTIME_H
