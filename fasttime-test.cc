// fasttime-test.cc
// Tests for `fasttime` module.

#include "smbase/fasttime.h"           // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  fastTimeInitialize();

  unsigned start = fastTimeMilliseconds;

  // See how many times we can read the timer in 100 ms.
  unsigned ct = 0;
  unsigned elapsed = 0;
  while (elapsed < 100) {
    ++ct;
    elapsed = fastTimeMilliseconds - start;
  }

  // This test can't "fail"; I just run it with VERBOSE=1 and manually
  // inspect the results.
  DIAG("Read the fast timer " << ct <<
       " (" << (ct / 1000000) << " M)" <<
       " times in " << elapsed << " ms.")
  DIAG("The timer 1000-loop counter is " <<
       fastTimeThread1000Loops << ".")
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_fasttime()
{
  if (!fastTimeIsImplemented()) {
    DIAG("fasttime module is not implemented on this platform.")
    return;
  }

  test_basics();
}


// EOF
