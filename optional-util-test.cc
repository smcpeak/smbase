// optional-util-test.cc
// Tests for `optional-util`.

#include "optional-util.h"             // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ

#include <optional>                    // std::optional


OPEN_ANONYMOUS_NAMESPACE


void testOptionalToString()
{
  std::optional<int> none;
  std::optional<int> one(1);

  EXPECT_EQ(optionalToString(none, "NONE"), "NONE");
  EXPECT_EQ(optionalToString(one, "NONE"), "1");

  EXPECT_EQ(stringb(none), "null");
  EXPECT_EQ(stringb(one), "1");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_optional_util()
{
  testOptionalToString();
}


// EOF
