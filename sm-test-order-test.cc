// sm-test-order-test.cc
// Tests for `sm-test-order` module.

#include "smbase/sm-test-order.h"      // module under test

#include "smbase/compare-util.h"       // smbase::compare
#include "smbase/exc.h"                // smbase::XMessage
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <functional>                  // std::reference_wrapper

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  int i1 = 1;
  int i2 = 2;
  int i3 = 3;

  EXPECT_STRICTLY_ORDERED(int, i1, i2, i3);

  EXPECT_EXN_SUBSTR(
    EXPECT_COMPARE(i1, i1, -1),
    XMessage, "actual: 0");

  EXPECT_EXN_SUBSTR(
    EXPECT_STRICTLY_ORDERED(int, i1, i3, i2),
    XMessage, "checkStrictlyOrdered: i1, i3, i2: i=1: j=2");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_test_order()
{
  test_basics();
}


// EOF
