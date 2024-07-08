// optional-util-test.cc
// Tests for `optional-util`.

#include "optional-util.h"             // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ

#include <algorithm>                   // std::{min, max}
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


void testLiftToOptional()
{
  std::optional<int> none;
  std::optional<int> one(1);
  std::optional<int> two(2);

  // Minimum.
  {
    // Work around the problem of passing an overloaded function to a
    // higher-order function.
    auto myMin = [](int a, int b) -> int { return std::min(a,b); };

    EXPECT_EQ(liftToOptional(none, none, myMin), none);
    EXPECT_EQ(liftToOptional(one, none, myMin), one);
    EXPECT_EQ(liftToOptional(none, one, myMin), one);
    EXPECT_EQ(liftToOptional(two, one, myMin), one);

    // Do one with an explicit cast instead of an intermediate lambda to
    // resolve the target.
    EXPECT_EQ(liftToOptional(one, none,
      static_cast<int const & (*)(int const &, int const &)>(std::min<int>)),
      one);
  }

  // Maximum.
  {
    auto myMax = [](int a, int b) -> int { return std::max(a,b); };

    EXPECT_EQ(liftToOptional(two, one, myMax), two);
  }

  // Addition.
  {
    EXPECT_EQ(liftToOptional(one, two,
                             [](int a, int b) -> int { return a+b; }),
              std::optional<int>(3));
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_optional_util()
{
  testOptionalToString();
  testLiftToOptional();
}


// EOF
