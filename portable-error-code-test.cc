// portable-error-code-test.cc
// Tests for `portable-error-code` module.

#include "smbase/portable-error-code.h"           // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_names()
{
  EXPECT_EQ(toString(PortableErrorCode::R_INVALID_FILENAME),
    "R_INVALID_FILENAME");
  EXPECT_EQ(reasonCodeDescription(PortableErrorCode::R_INVALID_FILENAME),
    "File name is invalid (too long, or bad chars, or ...)");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_portable_error_code()
{
  test_names();
}


// EOF
