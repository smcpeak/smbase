// sm-env-test.cc
// Tests for `sm-env`.

#include "sm-env.h"                    // module under test

#include "sm-test.h"                   // VPVAL

using namespace smbase;


// Called from unit-tests.cc.
void test_sm_env()
{
  // By setting envvar VERBOSE=1, these can be tested interactively.
  // Otherwise, they at least confirm the functions can be called
  // without crashing.
  VPVAL(envAsBool("VAR"));
  VPVAL(envOrEmpty("VAR"));
}


// EOF
