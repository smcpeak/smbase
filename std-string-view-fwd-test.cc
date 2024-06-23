// std-string-view-fwd-test.cc
// Tests for `std-string-view-fwd.h`.

#include "std-string-view-fwd.h"       // module under test

// Declare a function using the forward declaration.
static std::string_view getAStringView();

#include "sm-test.h"                   // EXPECT_EQ

#include <string_view>                 // std::string_view

// Define it using the usual name.
static std::string_view getAStringView()
{
  return "A string view.";
}

// Called from unit-tests.cc.
void test_std_string_view_fwd()
{
  EXPECT_EQ(getAStringView(), "A string view.");
}


// EOF
