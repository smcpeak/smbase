// xassert-eq-container-test.cc
// Tests for `xassert-eq-container` module.

#include "smbase/xassert-eq-container.h"         // module under test

#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  std::set a{1,2,3};
  XASSERT_EQUAL_SETS(a, a);

  std::set b{1,3};
  EXPECT_EXN_SUBSTR(XASSERT_EQUAL_SETS(a, b),
    XAssert, "Expected equal sets, but a has element 2 that b lacks.");
  EXPECT_EXN_SUBSTR(XASSERT_EQUAL_SETS(b, a),
    XAssert, "Expected equal sets, but a has element 2 that b lacks.");

  std::set c{1,2,3,4};
  EXPECT_EXN_SUBSTR(XASSERT_EQUAL_SETS(a, c),
    XAssert, "Expected equal sets, but c has element 4 that a lacks.");
  EXPECT_EXN_SUBSTR(XASSERT_EQUAL_SETS(c, a),
    XAssert, "Expected equal sets, but c has element 4 that a lacks.");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_xassert_eq_container()
{
  test_basics();
}


// EOF
