// xassert-test.cc
// Tests for `xassert` module.

#include "smbase/exc.h"                // smbase::{XBase, XAssert}
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN_SUBSTR

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testXassertPtr()
{
  int i = 3;
  int *p = &i;

  // Assert and use a non-null pointer.
  EXPECT_EQ(*xassertPtr(p), 3);

  // Assert a null pointer.
  try {
    int *np = nullptr;
    xassertPtr(np);
    xfailure("should have failed");
  }
  catch (XBase &x) {
    EXPECT_HAS_SUBSTRING(x.what(), "ptr != nullptr");
  }
}


void test_xassertPostcondition()
{
  EXPECT_EXN_SUBSTR(xassertPostcondition(false),
    XAssert, "false");

  EXPECT_EXN_SUBSTR(xfailurePostcondition("something"),
    XAssert, "something");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc
void test_xassert()
{
  testXassertPtr();
  test_xassertPostcondition();
}


// EOF
