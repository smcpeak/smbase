// gdv-binary64-float-test.cc
// Tests for `gdv-binary64-float` module.

#include "smbase/gdv-binary64-float.h" // module under test

#include "smbase/compare-util.h"       // smbase::compare
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cmath>                       // std::isfinite

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

    // TODO: Compare as GDValue too.
  }
}


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

  // Smallest positive normal.
  double const minNorm =
    std::numeric_limits<double>::min();
  xassert(minNorm > 0);

  testOne_compareDoublesRepresentationally(minNorm, minNorm, 0);
  testOne_compareDoublesRepresentationally(minNorm, 0, 1);
  testOne_compareDoublesRepresentationally(-minNorm, 0, -1);
  testOne_compareDoublesRepresentationally(-minNorm, minNorm, -1);

  // Smallest positive denormal.
  double const denorm =
    std::numeric_limits<double>::denorm_min();
  xassert(denorm > 0);

  testOne_compareDoublesRepresentationally(denorm, denorm, 0);
  testOne_compareDoublesRepresentationally(denorm, 0, 1);
  testOne_compareDoublesRepresentationally(denorm, -0.0, 1);
  testOne_compareDoublesRepresentationally(-denorm, 0, -1);
  testOne_compareDoublesRepresentationally(-denorm, denorm, -1);
  testOne_compareDoublesRepresentationally(denorm, minNorm, -1);
  testOne_compareDoublesRepresentationally(-denorm, -minNorm, 1);

  // Most positive and negative.
  double const highest =
    std::numeric_limits<double>::max();
  double const lowest =
    std::numeric_limits<double>::lowest();

  testOne_compareDoublesRepresentationally(highest, highest, 0);
  testOne_compareDoublesRepresentationally(lowest, lowest, 0);
  testOne_compareDoublesRepresentationally(highest, 0, 1);
  testOne_compareDoublesRepresentationally(lowest, 0, -1);
  testOne_compareDoublesRepresentationally(lowest, highest, -1);
  testOne_compareDoublesRepresentationally(highest, minNorm, 1);
  testOne_compareDoublesRepresentationally(highest, denorm, 1);

  // Finite before non-finite.
  testOne_compareDoublesRepresentationally(INFINITY, 0, 1);
  testOne_compareDoublesRepresentationally(-INFINITY, 0, 1);
  testOne_compareDoublesRepresentationally(NAN, 0, 1);
  testOne_compareDoublesRepresentationally(INFINITY, highest, 1);
  testOne_compareDoublesRepresentationally(INFINITY, lowest, 1);
  testOne_compareDoublesRepresentationally(INFINITY, denorm, 1);
  testOne_compareDoublesRepresentationally(INFINITY, minNorm, 1);
  testOne_compareDoublesRepresentationally(INFINITY, -0.0, 1);

  // NegativeInfinity before Infinity.
  testOne_compareDoublesRepresentationally(INFINITY, -INFINITY, 1);
  testOne_compareDoublesRepresentationally(INFINITY, INFINITY, 0);
  testOne_compareDoublesRepresentationally(-INFINITY, -INFINITY, 0);

  // NAN == NAN
  testOne_compareDoublesRepresentationally(NAN, NAN, 0);

  // -1 if NAN<INFINITY as classification, +1 otherwise.
  int const nan_vs_infinity =
    compare(std::fpclassify(NAN), std::fpclassify(INFINITY));

  // NAN vs. INFINITY
  testOne_compareDoublesRepresentationally(NAN, INFINITY, nan_vs_infinity);
  testOne_compareDoublesRepresentationally(NAN, -INFINITY, nan_vs_infinity);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdv_binary64_float()
{
  test_basics();
  test_compareDoublesRepresentationally();


  // TODO: More!
}


// EOF
