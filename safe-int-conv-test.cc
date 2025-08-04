// safe-int-conv-test.cc
// Tests for `safe-int-conv` module.

#include "smbase/safe-int-conv.h"      // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cstdint>                     // std::int64_t, etc.

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  static_assert(IsSafelyConvertible_v<signed char, int>);
  static_assert(IsSafelyConvertible_v<unsigned char, int>);

  static_assert(IsSafelyConvertible_v<int, int>);
  static_assert(!IsSafelyConvertible_v<int, unsigned>);

  static_assert(IsSafelyConvertible_v<int, long>);

  static_assert(IsSafelyConvertible_v<int32_t, int64_t>);
  static_assert(!IsSafelyConvertible_v<int64_t, int32_t>);

  static_assert(IsSafelyConvertible_v<uint32_t, uint64_t>);
  static_assert(IsSafelyConvertible_v<uint32_t, int64_t>);
  static_assert(!IsSafelyConvertible_v<uint64_t, uint32_t>);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_safe_int_conv()
{
  test_basics();
}


// EOF
