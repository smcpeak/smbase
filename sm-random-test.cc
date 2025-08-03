// sm-random-test.cc
// Tests for `sm-random` module.

#include "smbase/sm-random.h"          // module under test

#include "smbase/sm-env.h"             // smbase::envAsIntOr
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <algorithm>                   // std::min
#include <iomanip>                     // std::setw
#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Test `sm_random` for range and distribution bias.
//
// For the naive definition of `rand() % n`, the maximum useful range is
// 32768, as the maximum possible output value is 32767.
//
// Furthermore, within that range, there is strong bias toward lower
// values when using a range like 20000 that does not evenly divide
// 32768.
//
void test_distributionBias()
{
  // Size of the entire distribution.
  int size = envAsIntOr(10, "RANDOM_TEST_SIZE");

  // Print a few numbers just to get a sense for the range.
  VPVAL(sm_random(size));
  VPVAL(sm_random(size));
  VPVAL(sm_random(size));
  VPVAL(sm_random(size));
  VPVAL(sm_random(size));

  // Keep track of the number of times up to 10 values on the ends are
  // returned by the generator.
  int endCount = std::min(10, size / 2);
  std::vector<int> lowCounts(endCount /*size*/, 0 /*initial value*/);
  std::vector<int> highCounts(endCount /*size*/, 0 /*initial value*/);

  // Repeatedly generate numbers and add to the counts.
  int iters = envAsIntOr(2000, "RANDOM_TEST_ITERS");
  for (int i=0; i < iters; ++i) {
    int n = sm_random(size);
    if (n < endCount) {
      ++lowCounts.at(n);
    }
    else if (n > size-1-endCount) {
      ++highCounts.at(n - size + endCount);
    }
  }

  // Calculate the average count we recorded.
  float sum = 0;
  for (int i=0; i < endCount; ++i) {
    sum += lowCounts.at(i);
    sum += highCounts.at(i);
  }
  int avg = static_cast<int>(sum / (endCount*2));
  VPVAL(avg);

  // Print out the counts and how they compare to the average.
  int lowDiffSum = 0;
  for (int i=0; i < endCount; ++i) {
    int ct = lowCounts.at(i);
    int diff = ct - avg;
    lowDiffSum += diff;
    DIAG("counts[" << std::setw(5) << i <<
         "]: " << std::setw(10) << ct << "  " <<
         std::setw(5) << diff);
  }
  VPVAL(lowDiffSum);
  int highDiffSum = 0;
  for (int i=0; i < endCount; ++i) {
    int ct = highCounts.at(i);
    int diff = ct - avg;
    highDiffSum += diff;
    DIAG("counts[" << std::setw(5) << (size - endCount + i) <<
         "]: " << std::setw(10) << ct << "  " <<
         std::setw(5) << diff);
  }
  VPVAL(highDiffSum);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_random()
{
  test_distributionBias();
}


// EOF
