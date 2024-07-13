// std-variant-fwd-test.cc
// Tests for `std-variant-fwd.h`.

#include "std-variant-fwd.h"           // module under test

// Declare a function using the forward declaration.
static stdfwd::variant<int, float> getAVariant();

static std::variant<int, float> getAVariant2();

#include "xassert.h"                   // xassert

#include <variant>                     // std::variant

// Define it using the usual name.
static std::variant<int, float> getAVariant()
{
  return std::variant<int, float>(3);
}

static std::variant<int, float> getAVariant2()
{
  return std::variant<int, float>(4);
}

// Called from unit-tests.cc.
void test_std_variant_fwd()
{
  xassert(std::get<int>(getAVariant()) == 3);
  xassert(std::get<int>(getAVariant2()) == 4);
}


// EOF
