// most-sig-bit-test.cc
// Tests for `most-sig-bit` module.

#include "smbase/most-sig-bit.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cstdint>                     // UINT64_C

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


int msb(std::uint64_t n)
{
  int ret1 = mostSignificantBit(n);
  int ret2 = mostSignificantBit_fallback(n);
  EXPECT_EQ(ret1, ret2);
  return ret1;
}


void test_msb()
{
  EXPECT_EXN_SUBSTR(msb(0), XAssert, "n > 0");
  EXPECT_EQ(msb(1), 0);
  EXPECT_EQ(msb(2), 1);
  EXPECT_EQ(msb(3), 1);
  EXPECT_EQ(msb(4), 2);
  EXPECT_EQ(msb(UINT64_C( 0xFFFffffFFFFffff)), 59);
  EXPECT_EQ(msb(UINT64_C(0x1000000000000000)), 60);
  EXPECT_EQ(msb(UINT64_C(0x7FFFffffFFFFffff)), 62);
  EXPECT_EQ(msb(UINT64_C(0x8000000000000000)), 63);
  EXPECT_EQ(msb(UINT64_C(0xFFFFffffFFFFffff)), 63);
}


void test_msboapo()
{
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(0), 0);
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(1), 1);
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(2), 1);
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(3), 2);
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(UINT64_C(0xFFFFffffFFFFfffe)), 63);
  EXPECT_EQ(mostSignificantBitOfArgPlusOne(UINT64_C(0xFFFFffffFFFFffff)), 64);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_most_sig_bit()
{
  test_msb();
  test_msboapo();
}


// EOF
