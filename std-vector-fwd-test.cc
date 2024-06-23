// std-vector-fwd-test.cc
// Tests for `std-vector-fwd.h`.

#include "std-vector-fwd.h"            // module under test

// Declare a function using the forward declaration.
static stdfwd::vector<int> getAVector();

// Here, it is not possible to use `std`.
//static std::vector<int> getAVector2();

// This must come before sm-test.h.
#include "vector-util.h"               // operator<<(vector)

#include "sm-test.h"                   // EXPECT_EQ

#include <vector>                      // std::vector

// Define it using the usual name.
static std::vector<int> getAVector()
{
  return {1,2,3};
}

// Called from unit-tests.cc.
void test_std_vector_fwd()
{
  EXPECT_EQ(getAVector(), (std::vector<int>{1,2,3}));
}


// EOF
