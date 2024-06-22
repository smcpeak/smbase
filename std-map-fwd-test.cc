// std-map-fwd-test.cc
// Tests for `std-map-fwd.h`.

#include "std-map-fwd.h"               // module under test

// Declare a function using the forward declaration.
static stdfwd::map<int, int> getAMap();

// This must come before sm-test.h.
#include "map-util.h"                  // operator<<(map)

#include "sm-test.h"                   // EXPECT_EQ

#include <map>                         // std::map

// Define it using the usual name.
static std::map<int, int> getAMap()
{
  return {{1,2}, {3,4}};
}

// Called from unit-tests.cc.
void test_std_map_fwd()
{
  EXPECT_EQ(getAMap(), (std::map<int, int>{{1,2}, {3,4}}));
}


// EOF
