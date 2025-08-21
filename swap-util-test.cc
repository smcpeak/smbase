// swap-util-test.cc
// Tests for `swap-util` module.

#include "smbase/swap-util.h"          // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  int a = 1, b = 2;
  swapIfGreaterThan(a, b);
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2);

  a = 2;
  swapIfGreaterThan(a, b);
  EXPECT_EQ(a, 2);
  EXPECT_EQ(b, 2);

  a = 3;
  swapIfGreaterThan(a, b);
  EXPECT_EQ(a, 2);
  EXPECT_EQ(b, 3);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_swap_util()
{
  test_basics();
}


// EOF
