// std-string-fwd-test.cc
// Tests for `std-string-fwd.h`.

#include "std-string-fwd.h"            // module under test

// Declare a function using the forward declaration.
static stdfwd::string getAString();

// Demonstrate we can use `std` too.
static std::string getAString2();

#include "sm-test.h"                   // EXPECT_EQ

#include <string>                      // std::string

// Define it using the usual name.
static std::string getAString()
{
  return "A string.";
}

static std::string getAString2()
{
  return "A string2.";
}

// Called from unit-tests.cc.
void test_std_string_fwd()
{
  EXPECT_EQ(getAString(), "A string.");
  EXPECT_EQ(getAString2(), "A string2.");
}


// EOF
