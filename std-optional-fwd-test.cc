// std-optional-fwd-test.cc
// Tests for `std-optional-fwd.h`.

#include "std-optional-fwd.h"          // module under test

static std::optional<int> getAnOptInt();

// Must come before sm-test.h.
#include "optional-util.h"             // operator<<(optional)

#include "sm-test.h"                   // EXPECT_EQ

#include <optional>                    // std::optional

static std::optional<int> getAnOptInt()
{
  return 32;
}

// Called from unit-tests.cc.
void test_std_optional_fwd()
{
  EXPECT_EQ(getAnOptInt(), std::optional<int>(32));
}


// EOF
