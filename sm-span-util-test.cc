// sm-span-util-test.cc
// Tests for `sm-span-util` module.

#include "smbase/sm-span-util.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-span.h"            // smbase::Span
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <string>                      // std::string

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_spanSum()
{
  int arr[] = {1,2,3};

  EXPECT_EQ(spanSum(Span<int>(arr)), 6);
  EXPECT_EQ(spanSum(Span<int>(arr).subspan(1)), 5);
  EXPECT_EQ(spanSum(Span<int>(arr).subspan(2)), 3);
  EXPECT_EQ(spanSum(Span<int>(arr).subspan(3)), 0);

  // Make sure it works with `const`-qualified type argument too.
  EXPECT_EQ(spanSum(Span<int const>(Span<int>(arr))), 6);

  // Cool, this works too!
  std::string arr2[] = { "abc", "def" };
  EXPECT_EQ(spanSum(Span<std::string>(arr2)), "abcdef");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_span_util()
{
  test_spanSum();
}


// EOF
