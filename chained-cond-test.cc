// chained-cond-test.cc
// Tests for `chained-cond` module.

#include "smbase/chained-cond.h"       // module under test

#include "smbase/gdvalue.h"            // GDVN_OMAP_EXPRS
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


bool lt(int a, int b)
{
  return a < b;
}

bool le(int a, int b)
{
  return a <= b;
}


// Check a 3-argument compound test.
void check3(
  char const *label,
  bool (*compoundTest)(int const &a, int const &b, int const &c),
  bool (*test1)(int a, int b),
  bool (*test2)(int b, int c))
{
  TEST_CASE(label);

  int const low = -2;
  int const high = 3;

  for (int a = low; a <= high; ++a) {
    for (int b = low; b <= high; ++b) {
      for (int c = low; c <= high; ++c) {
        bool expect = test1(a, b) && test2(b, c);
        try {
          EXPECT_EQ(compoundTest(a, b, c), expect);
        }
        catch (XMessage &x) {
          x.appendContext(GDVN_OMAP_EXPRS(0, a, b, c));
          throw x;
        }
      }
    }
  }
}


// Check a 2-argument test that acts like a 3-argument test with a fixed
// first argument of 0.
void check3_z(
  char const *label,
  bool (*compoundTest)(int const &b, int const &c),
  bool (*test1)(int a, int b),
  bool (*test2)(int b, int c))
{
  TEST_CASE(label);

  int const low = -2;
  int const high = 3;

  // This version fixes the first argument as 0.
  for (int b = low; b <= high; ++b) {
    for (int c = low; c <= high; ++c) {
      bool expect = test1(0, b) && test2(b, c);
      try {
        EXPECT_EQ(compoundTest(b, c), expect);
      }
      catch (XMessage &x) {
        x.appendContext(GDVN_OMAP_EXPRS(0, b, c));
        throw x;
      }
    }
  }
}


void test_le_lt()
{
  EXPECT_EQ(cc::le_lt(1, 2, 3), true);
  EXPECT_EQ(cc::le_lt(2, 2, 3), true);

  EXPECT_EQ(cc::le_lt(3, 2, 3), false);
  EXPECT_EQ(cc::le_lt(2, 1, 3), false);
  EXPECT_EQ(cc::le_lt(2, 2, 2), false);

  check3("le_lt",
    cc::le_lt<int>,
    le,
    lt);
}


void test_z_le_lt()
{
  EXPECT_EQ(cc::z_le_lt(1, 2), true);
  EXPECT_EQ(cc::z_le_lt(0, 2), true);

  EXPECT_EQ(cc::z_le_lt(-1, 2), false);
  EXPECT_EQ(cc::z_le_lt(0, 0), false);

  check3_z("z_le_lt",
    cc::z_le_lt<int>,
    le,
    lt);
}


void test_le_le()
{
  EXPECT_EQ(cc::le_le(1, 2, 3), true);
  EXPECT_EQ(cc::le_le(2, 2, 3), true);
  EXPECT_EQ(cc::le_le(2, 2, 2), true);

  EXPECT_EQ(cc::le_le(3, 2, 2), false);
  EXPECT_EQ(cc::le_le(2, 3, 2), false);
  EXPECT_EQ(cc::le_le(2, 2, 1), false);

  check3("le_le",
    cc::le_le<int>,
    le,
    le);
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

  check3_z("z_le_le",
    cc::z_le_le<int>,
    le,
    le);
}


void test_lt_le()
{
  EXPECT_EQ(cc::lt_le(1, 2, 3), true);
  EXPECT_EQ(cc::lt_le(1, 2, 2), true);

  EXPECT_EQ(cc::lt_le(2, 2, 2), false);
  EXPECT_EQ(cc::lt_le(2, 3, 2), false);
  EXPECT_EQ(cc::lt_le(1, 2, 1), false);

  check3("lt_le",
    cc::lt_le<int>,
    lt,
    le);
}


void test_z_lt_le()
{
  EXPECT_EQ(cc::z_lt_le(1, 2), true);
  EXPECT_EQ(cc::z_lt_le(1, 1), true);

  EXPECT_EQ(cc::z_lt_le(0, 2), false);
  EXPECT_EQ(cc::z_lt_le(3, 2), false);
  EXPECT_EQ(cc::z_lt_le(0, -1), false);

  check3_z("z_lt_le",
    cc::z_lt_le<int>,
    lt,
    le);
}


// Check a 4-argument compound test.
void check4(
  char const *label,
  bool (*compoundTest)(int const &a, int const &b, int const &c, int const &d),
  bool (*test1)(int a, int b),
  bool (*test2)(int b, int c),
  bool (*test3)(int c, int d))
{
  TEST_CASE(label);

  int const low = -2;
  int const high = 3;

  for (int a = low; a <= high; ++a) {
    for (int b = low; b <= high; ++b) {
      for (int c = low; c <= high; ++c) {
        for (int d = low; d <= high; ++d) {
          bool expect = test1(a, b) && test2(b, c) && test3(c, d);
          try {
            EXPECT_EQ(compoundTest(a, b, c, d), expect);
          }
          catch (XMessage &x) {
            x.appendContext(GDVN_OMAP_EXPRS(0, a, b, c, d));
            throw x;
          }
        }
      }
    }
  }
}


// Check a 3-argument test that acts like a 4-argument test with a fixed
// first argument of 0.
void check4_z(
  char const *label,
  bool (*compoundTest)(int const &b, int const &c, int const &d),
  bool (*test1)(int a, int b),
  bool (*test2)(int b, int c),
  bool (*test3)(int c, int d))
{
  TEST_CASE(label);

  int const low = -2;
  int const high = 3;

  // This version fixes the first argument as 0.
  for (int b = low; b <= high; ++b) {
    for (int c = low; c <= high; ++c) {
      for (int d = low; d <= high; ++d) {
        bool expect = test1(0, b) && test2(b, c) && test3(c, d);
        try {
          EXPECT_EQ(compoundTest(b, c, d), expect);
        }
        catch (XMessage &x) {
          x.appendContext(GDVN_OMAP_EXPRS(0, b, c, d));
          throw x;
        }
      }
    }
  }
}


void test_le_le_le()
{
  EXPECT_EQ(cc::le_le_le(1, 2, 3, 4), true);
  EXPECT_EQ(cc::le_le_le(1, 1, 3, 4), true);
  EXPECT_EQ(cc::le_le_le(1, 1, 1, 4), true);
  EXPECT_EQ(cc::le_le_le(1, 1, 1, 1), true);

  EXPECT_EQ(cc::le_le_le(2, 1, 1, 1), false);
  EXPECT_EQ(cc::le_le_le(1, 2, 1, 1), false);
  EXPECT_EQ(cc::le_le_le(1, 1, 2, 1), false);
  EXPECT_EQ(cc::le_le_le(1, 1, 1, 0), false);

  check4("le_le_le",
    cc::le_le_le<int>,
    le,
    le,
    le);
}


void test_z_le_le_le()
{
  EXPECT_EQ(cc::z_le_le_le(1, 2, 3), true);
  EXPECT_EQ(cc::z_le_le_le(0, 2, 3), true);
  EXPECT_EQ(cc::z_le_le_le(0, 0, 3), true);
  EXPECT_EQ(cc::z_le_le_le(0, 0, 0), true);

  EXPECT_EQ(cc::z_le_le_le(-1, 0, 0), false);
  EXPECT_EQ(cc::z_le_le_le(0, -1, 0), false);
  EXPECT_EQ(cc::z_le_le_le(0, 0, -1), false);

  EXPECT_EQ(cc::z_le_le_le(1, 0, 0), false);
  EXPECT_EQ(cc::z_le_le_le(0, 1, 0), false);
  EXPECT_EQ(cc::z_le_le_le(1, 1, 0), false);

  check4_z("z_le_le_le",
    cc::z_le_le_le<int>,
    le,
    le,
    le);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_chained_cond()
{
  test_le_lt();
  test_z_le_lt();
  test_le_le();
  test_z_le_le();
  test_lt_le();
  test_z_lt_le();
  test_le_le_le();
  test_z_le_le_le();
}


// EOF
