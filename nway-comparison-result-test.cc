// nway-comparison-result-test.cc
// Tests for `nway-comparison-result` module.

#include "smbase/nway-comparison-result.h"       // module under test

#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/xassert.h"                      // xassert, XAssert

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  NWayComparisonResult nullCR;
  xassert(nullCR.isNull());
  xassert(!nullCR.hasSmallest());
  xassert(!nullCR.hasEqualIndices());

  NWayComparisonResult smallestCR(3);
  xassert(!smallestCR.isNull());
  xassert(smallestCR.hasSmallest());
  xassert(!smallestCR.hasEqualIndices());
  EXPECT_EQ(smallestCR.getSmallest(), 3);

  NWayComparisonResult equalCR(4, 5);
  xassert(!equalCR.isNull());
  xassert(!equalCR.hasSmallest());
  xassert(equalCR.hasEqualIndices());
  EXPECT_EQ(equalCR.getEqualIndices().first, 4);
  EXPECT_EQ(equalCR.getEqualIndices().second, 5);

  xassert(nullCR == nullCR);
  xassert(nullCR == NWayComparisonResult());
  xassert(nullCR != smallestCR);
  xassert(nullCR != equalCR);

  xassert(smallestCR == smallestCR);
  xassert(smallestCR == NWayComparisonResult(3));
  xassert(smallestCR != NWayComparisonResult(4));
  xassert(smallestCR != equalCR);

  xassert(equalCR == equalCR);
  xassert(equalCR == NWayComparisonResult(4, 5));
  xassert(equalCR != NWayComparisonResult(3, 5));
  xassert(equalCR != NWayComparisonResult(4, 6));
  xassert(equalCR != NWayComparisonResult(5, 4));

  NWayComparisonResult cr(nullCR);
  xassert(cr.isNull());

  cr = smallestCR;
  xassert(cr.hasSmallest());
  EXPECT_EQ(cr.getSmallest(), 3);

  cr = equalCR;
  xassert(cr.hasEqualIndices());
  EXPECT_EQ(cr.getEqualIndices().first, 4);
  EXPECT_EQ(cr.getEqualIndices().second, 5);

  cr = nullCR;
  xassert(cr.isNull());
}


void test_edges()
{
  // Smallest index is 0.
  NWayComparisonResult smallest0(0);
  xassert(smallest0.hasSmallest());
  EXPECT_EQ(smallest0.getSmallest(), 0);

  // Equal indices starting at 0.
  NWayComparisonResult equal01(0, 1);
  xassert(equal01.hasEqualIndices());
  EXPECT_EQ(equal01.getEqualIndices().first, 0);
  EXPECT_EQ(equal01.getEqualIndices().second, 1);
}


void test_preconditions()
{
  // Bad accessor.
  EXPECT_EXN_SUBSTR(NWayComparisonResult().getSmallest(),
    XAssert, "hasSmallest");

  // Non-distinct indices.
  EXPECT_EXN_SUBSTR(NWayComparisonResult(2, 2),
    XAssert, "eqIndex1 != eqIndex2");

  // Negative indices.
  EXPECT_EXN_SUBSTR(NWayComparisonResult(-1),
    XAssert, "smallestIndex >= 0");
  EXPECT_EXN_SUBSTR(NWayComparisonResult(-2, 2),
    XAssert, "eqIndex1 >= 0");
  EXPECT_EXN_SUBSTR(NWayComparisonResult(2, -2),
    XAssert, "eqIndex2 >= 0");
}


void test_selfAssign()
{
  NWayComparisonResult selfAssign(7);

  // Clang does not like to see "x = x", so hide the self-assign behind
  // a reference.
  NWayComparisonResult &hideIt = selfAssign;
  selfAssign = hideIt;

  xassert(selfAssign.hasSmallest());
  EXPECT_EQ(selfAssign.getSmallest(), 7);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_nway_comparison_result()
{
  test_basics();
  test_edges();
  test_preconditions();
  test_selfAssign();
}


// EOF
