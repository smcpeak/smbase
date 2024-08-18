// std-set-fwd-test.cc
// Tests for `std-set-fwd.h`.

#include "std-set-fwd.h"               // module under test

// Declare a function using the forward declaration.
static stdfwd::set<int> getASet();

// This must come before sm-test.h.
#include "set-util.h"                  // operator<<(set)

#include "sm-test.h"                   // EXPECT_EQ

#include <set>                         // std::set

// Define it using the usual name.
static std::set<int> getASet()
{
  return {1,2,3,4};
}

// Called from unit-tests.cc.
void test_std_set_fwd()
{
  EXPECT_EQ(getASet(), (std::set<int>{1,2,3,4}));
}


// EOF
