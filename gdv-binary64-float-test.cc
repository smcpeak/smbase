// gdv-binary64-float-test.cc
// Tests for `gdv-binary64-float` module.

#include "smbase/gdv-binary64-float.h" // module under test

#include "smbase/compare-util.h"       // smbase::compare
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cmath>                       // std::{isfinite, nextafter}
#include <limits>                      // std::numeric_limits
#include <string>                      // std::memcmp

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDVBinary64Float f;
  EXPECT_EQ(f.getValue(), 0.0);
}


void testOne_compareDoublesRepresentationally(
  double a,
  double b,
  int expect)
{
  EXN_CONTEXT_EXPR(a);
  EXN_CONTEXT_EXPR(b);

  EXPECT_EQ(compareDoublesRepresentationally(a, b), expect);
  EXPECT_EQ(compareDoublesRepresentationally(b, a), -expect);

  if (std::isfinite(a) && std::isfinite(b)) {
    // Compare them as GDVBinary64Float objects, expecting the same
    // result.
    GDVBinary64Float va(a);
    GDVBinary64Float vb(b);

    EXPECT_EQ(compare(va, vb), expect);
    EXPECT_EQ(compare(vb, va), -expect);
  }
}


// Smallest positive normal.
double constexpr minNorm =
  std::numeric_limits<double>::min();
static_assert(minNorm > 0);

// Smallest positive denormal.
double constexpr minDenorm =
  std::numeric_limits<double>::denorm_min();
static_assert(minDenorm > 0);

// Most positive and negative.
double constexpr highest =
  std::numeric_limits<double>::max();
double constexpr lowest =
  std::numeric_limits<double>::lowest();


void test_compareDoublesRepresentationally()
{
  testOne_compareDoublesRepresentationally(0, 0, 0);
  testOne_compareDoublesRepresentationally(1, 0, 1);
  testOne_compareDoublesRepresentationally(-1, 0, -1);
  testOne_compareDoublesRepresentationally(-1, 1, -1);

  // Negative zero.
  testOne_compareDoublesRepresentationally(-0.0, 0, -1);
  testOne_compareDoublesRepresentationally(-0.0, 1, -1);
  testOne_compareDoublesRepresentationally(-0.0, -1, 1);

  testOne_compareDoublesRepresentationally(minNorm, minNorm, 0);
  testOne_compareDoublesRepresentationally(minNorm, 0, 1);
  testOne_compareDoublesRepresentationally(-minNorm, 0, -1);
  testOne_compareDoublesRepresentationally(-minNorm, minNorm, -1);

  testOne_compareDoublesRepresentationally(minDenorm, minDenorm, 0);
  testOne_compareDoublesRepresentationally(minDenorm, 0, 1);
  testOne_compareDoublesRepresentationally(minDenorm, -0.0, 1);
  testOne_compareDoublesRepresentationally(-minDenorm, 0, -1);
  testOne_compareDoublesRepresentationally(-minDenorm, minDenorm, -1);
  testOne_compareDoublesRepresentationally(minDenorm, minNorm, -1);
  testOne_compareDoublesRepresentationally(-minDenorm, -minNorm, 1);

  testOne_compareDoublesRepresentationally(highest, highest, 0);
  testOne_compareDoublesRepresentationally(lowest, lowest, 0);
  testOne_compareDoublesRepresentationally(highest, 0, 1);
  testOne_compareDoublesRepresentationally(lowest, 0, -1);
  testOne_compareDoublesRepresentationally(lowest, highest, -1);
  testOne_compareDoublesRepresentationally(highest, minNorm, 1);
  testOne_compareDoublesRepresentationally(highest, minDenorm, 1);

  // Finite before non-finite.
  testOne_compareDoublesRepresentationally(INFINITY, 0, 1);
  testOne_compareDoublesRepresentationally(-INFINITY, 0, 1);
  testOne_compareDoublesRepresentationally(NAN, 0, 1);
  testOne_compareDoublesRepresentationally(INFINITY, highest, 1);
  testOne_compareDoublesRepresentationally(INFINITY, lowest, 1);
  testOne_compareDoublesRepresentationally(INFINITY, minDenorm, 1);
  testOne_compareDoublesRepresentationally(INFINITY, minNorm, 1);
  testOne_compareDoublesRepresentationally(INFINITY, -0.0, 1);

  // NegativeInfinity before Infinity.
  testOne_compareDoublesRepresentationally(INFINITY, -INFINITY, 1);
  testOne_compareDoublesRepresentationally(INFINITY, INFINITY, 0);
  testOne_compareDoublesRepresentationally(-INFINITY, -INFINITY, 0);

  // NAN == NAN
  testOne_compareDoublesRepresentationally(NAN, NAN, 0);

  // -1 if NAN < INFINITY as classification, +1 otherwise.
  int const nan_vs_infinity =
    compare(std::fpclassify(NAN), std::fpclassify(INFINITY));

  // NAN vs. INFINITY
  testOne_compareDoublesRepresentationally(NAN, INFINITY, nan_vs_infinity);
  testOne_compareDoublesRepresentationally(NAN, -INFINITY, nan_vs_infinity);
}


void testOneValue_serialization(char const *name, double n)
{
  if (std::isfinite(n)) {
    GDVBinary64Float v(n);
    std::string s = v.toString();
    DIAG(name << s);

    GDVBinary64Float after = GDVBinary64Float::parseString(s);
    xassert(v == after);
    xassert(s == after.toString());

    // Verify bitwise identity.
    double afterDouble = after.getValue();
    xassert(0==std::memcmp(&afterDouble, &n, sizeof(n)));

    // Require that the serialized form have a decimal or exponent.
    xassert(s.find_first_of("eE.") != string::npos);
  }
  else {
    DIAG(name << "(not finite)");
  }
}


double upULP(double n)
{
  return std::nextafter(n, std::numeric_limits<double>::infinity());
}

double downULP(double n)
{
  return std::nextafter(n, -std::numeric_limits<double>::infinity());
}


void testOne_serialization(char const *name, double n)
{
  DIAG("testOne_serialization: " << name);

  testOneValue_serialization("   n    : ", n);
  testOneValue_serialization("   n+ulp: ", upULP(n));
  testOneValue_serialization("   n-ulp: ", downULP(n));

  n = -n;
  testOneValue_serialization("  -n    : ", n);
  testOneValue_serialization("  -n+ulp: ", upULP(n));
  testOneValue_serialization("  -n-ulp: ", downULP(n));
}


#define TEST_SERIALIZE(n) \
  testOne_serialization(#n, n)


void test_serialization()
{
  GDVBinary64Float posZero(0);
  GDVBinary64Float negZero(-0.0);

  VPVAL(posZero);
  VPVAL(negZero);

  VPVAL(GDVBinary64Float(9));
  VPVAL(GDVBinary64Float(10));
  VPVAL(GDVBinary64Float(11));


  double const euler = exp(1.0);
  double const pi = atan(1) * 4;

  TEST_SERIALIZE(0);
  TEST_SERIALIZE(minDenorm);
  TEST_SERIALIZE(minNorm);
  TEST_SERIALIZE(1);
  TEST_SERIALIZE(euler);
  TEST_SERIALIZE(pi);
  TEST_SERIALIZE(9);
  TEST_SERIALIZE(10);
  TEST_SERIALIZE(11);
  TEST_SERIALIZE(100);
  TEST_SERIALIZE(1e10);
  TEST_SERIALIZE(1e11);
  TEST_SERIALIZE(1e12);
  TEST_SERIALIZE(1e13);
  TEST_SERIALIZE(1e14);
  TEST_SERIALIZE(1e15);
  TEST_SERIALIZE(1e16);
  TEST_SERIALIZE(1e17);
  TEST_SERIALIZE(1e18);
  TEST_SERIALIZE(1e19);
  TEST_SERIALIZE(1e20);
  TEST_SERIALIZE(1e100);
  TEST_SERIALIZE(1e200);
  TEST_SERIALIZE(1e300);
  TEST_SERIALIZE(pi*1e10);
  TEST_SERIALIZE(pi*1e100);
  TEST_SERIALIZE(pi*1e300);
  TEST_SERIALIZE(1e-10);
  TEST_SERIALIZE(1e-100);
  TEST_SERIALIZE(1e-200);
  TEST_SERIALIZE(1e-300);
  TEST_SERIALIZE(pi*1e-10);
  TEST_SERIALIZE(pi*1e-100);
  TEST_SERIALIZE(pi*1e-300);
  TEST_SERIALIZE(highest);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdv_binary64_float()
{
  test_basics();
  test_compareDoublesRepresentationally();
  test_serialization();
}


// EOF
