// sm-test-test2.cc
// More tests for `sm-test`.

// This file is in the public domain.

// The reason for another test module is I want to exercise some of the
// macros while `using namespace smbase;` is *not* in scope, and the top
// of `sm-test-test` has tests that are sensitive to their location in
// that file so I can't easily insert tests at the top.

#include "smbase/sm-test.h"            // module under test; and test harness to use

#include "smbase/exc.h"                // smbase::{xmessage, XMessage}
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE


OPEN_ANONYMOUS_NAMESPACE


void test_EXPECT_EXN()
{
  EXPECT_EXN_SUBSTR(smbase::xmessage("foo"), smbase::XMessage, "foo");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from sm-test.cc.
void test_sm_test2()
{
  test_EXPECT_EXN();
}


// EOF
