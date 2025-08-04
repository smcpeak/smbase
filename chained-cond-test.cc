// chained-cond-test.cc
// Tests for `chained-cond` module.

#include "smbase/chained-cond.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_le_lt()
{
  EXPECT_EQ(cc::le_lt(1, 2, 3), true);
  EXPECT_EQ(cc::le_lt(2, 2, 3), true);

  EXPECT_EQ(cc::le_lt(3, 2, 3), false);
  EXPECT_EQ(cc::le_lt(2, 1, 3), false);
  EXPECT_EQ(cc::le_lt(2, 2, 2), false);
}


void test_z_le_lt()
{
  EXPECT_EQ(cc::z_le_lt(1, 2), true);
  EXPECT_EQ(cc::z_le_lt(0, 2), true);

  EXPECT_EQ(cc::z_le_lt(-1, 2), false);
  EXPECT_EQ(cc::z_le_lt(0, 0), false);
}


void test_le_le()
{
  EXPECT_EQ(cc::le_le(1, 2, 3), true);
  EXPECT_EQ(cc::le_le(2, 2, 3), true);
  EXPECT_EQ(cc::le_le(2, 2, 2), true);

  EXPECT_EQ(cc::le_le(3, 2, 2), false);
  EXPECT_EQ(cc::le_le(2, 3, 2), false);
  EXPECT_EQ(cc::le_le(2, 2, 1), false);
}


void test_z_le_le()
{
  EXPECT_EQ(cc::z_le_le(1, 2), true);
  EXPECT_EQ(cc::z_le_le(0, 2), true);
  EXPECT_EQ(cc::z_le_le(1, 1), true);
  EXPECT_EQ(cc::z_le_le(0, 0), true);

  EXPECT_EQ(cc::z_le_le(-1, 2), false);
  EXPECT_EQ(cc::z_le_le(3, 2), false);
  EXPECT_EQ(cc::z_le_le(3, 0), false);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_chained_cond()
{
  test_le_lt();
  test_z_le_lt();
  test_le_le();
  test_z_le_le();
}


// EOF
