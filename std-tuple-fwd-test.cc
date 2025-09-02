// std-tuple-fwd-test.cc
// Tests for `std-tuple-fwd.h`.

#include "std-tuple-fwd.h"             // module under test

// Declare a function using the forward declaration.
static stdfwd::tuple<int, char const *> getATuple();

// Here, `std` should be safe.
static std::tuple<int, char const *> getATuple2();

#include "xassert.h"                   // xassert

#include <tuple>                       // std::tuple

static char const * const ptr = "seven";

// Define it using the usual name.
static std::tuple<int, char const *> getATuple()
{
  return {3, ptr};
}

static std::tuple<int, char const *> getATuple2()
{
  return {3, ptr};
}

// Called from unit-tests.cc.
void test_std_tuple_fwd()
{
  std::tuple<int, char const *> expect{3, ptr};
  xassert(getATuple() == expect);
  xassert(getATuple2() == expect);
}


// EOF
