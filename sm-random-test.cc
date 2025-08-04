// sm-random-test.cc
// Tests for `sm-random` module.

#include "smbase/sm-random.h"          // module under test

#include "smbase/gdv-ordered-map.h"    // gdv::GDVOrderedMap
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/most-sig-bit.h"       // smbase::mostSignificantBitOfArgPlusOne
#include "smbase/optional-util.h"      // optAccumulateMax
#include "smbase/sm-env.h"             // smbase::envAsIntOr
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, TEST_CASE_EXPRS

#include <algorithm>                   // std::min
#include <cstdint>                     // std::uint64_t
#include <iomanip>                     // std::setw
#include <limits>                      // std::numeric_limits
#include <vector>                      // std::vector

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


using std::uint64_t;


// Interface for generating a 64-bit random number.
class Rng64 {
public:
  virtual uint64_t generate(uint64_t maxValue) = 0;
};


void measureBias(Rng64 &rng, uint64_t maxValue, int numIters)
{
  TEST_CASE_EXPRS("measureBias", maxValue, numIters);

  xassertPrecondition(numIters > 0);

  // Print a few numbers just to get a sense for the range.
  VPVAL(rng.generate(maxValue));
  VPVAL(rng.generate(maxValue));
  VPVAL(rng.generate(maxValue));
  VPVAL(rng.generate(maxValue));
  VPVAL(rng.generate(maxValue));

  // Keep track of the number of times up to 10 values on the ends are
  // returned by the generator.
  int endCount = static_cast<int>(std::min(static_cast<uint64_t>(10), maxValue / 2));
  std::vector<int> lowCounts(endCount /*size*/, 0 /*initial value*/);
  std::vector<int> highCounts(endCount /*size*/, 0 /*initial value*/);

  // Number of times the result has a particular bit set.
  int numBits = mostSignificantBitOfArgPlusOne(maxValue);
  xassert(numBits <= 64);
  std::vector<int> bitCounts(numBits /*size*/, 0 /*initialValue*/);

  // Largest bit we saw set.
  std::optional<int> maxBit;

  // Repeatedly generate numbers and add to the counts.
  for (int i=0; i < numIters; ++i) {
    uint64_t n = rng.generate(maxValue);
    if (n < static_cast<uint64_t>(endCount)) {
      ++lowCounts.at(n);
    }
    else if (n > maxValue-endCount) {
      ++highCounts.at(n - (maxValue-endCount) - 1);
    }

    uint64_t const one_64 = 1;
    for (int bit=0; bit<numBits; ++bit) {
      if (n & (one_64 << bit)) {
        ++bitCounts.at(bit);
        optAccumulateMax(maxBit, bit);
      }
    }
  }
  xassert(maxBit.has_value());
  int maxBitPlusOne = *maxBit + 1;

  // Calculate the average count we recorded.
  float perValueSum = 0;
  for (int i=0; i < endCount; ++i) {
    perValueSum += lowCounts.at(i);
    perValueSum += highCounts.at(i);
  }
  int perValueAvg = static_cast<int>(perValueSum / (endCount*2));
  VPVAL(perValueAvg);

  // Print out the counts and how they compare to the average.
  int lowDiffSum = 0;
  for (int i=0; i < endCount; ++i) {
    int ct = lowCounts.at(i);
    int diff = ct - perValueAvg;
    lowDiffSum += diff;
    DIAG("counts[" << std::setw(5) << i <<
         "]: " << std::setw(10) << ct << "  " <<
         std::setw(5) << diff);
  }
  VPVAL(lowDiffSum);
  int highDiffSum = 0;
  for (int i=0; i < endCount; ++i) {
    int ct = highCounts.at(i);
    int diff = ct - perValueAvg;
    highDiffSum += diff;
    DIAG("counts[" << std::setw(5) << (maxValue - endCount + i + 1) <<
         "]: " << std::setw(10) << ct << "  " <<
         std::setw(5) << diff);
  }
  VPVAL(highDiffSum);

  // Average count per bit.
  int perBitSum = 0;
  for (int bit=0; bit < maxBitPlusOne; ++bit) {
    perBitSum += bitCounts.at(bit);
  }
  int perBitAvg = static_cast<int>(perBitSum / maxBitPlusOne);
  VPVAL(perBitAvg);

  for (int bit=0; bit < maxBitPlusOne; ++bit) {
    int ct = bitCounts.at(bit);
    int diff = ct - perBitAvg;
    DIAG("bitCount[" << std::setw(2) << bit <<
         "]: " << std::setw(10) << ct << "  " <<
         std::setw(5) << diff);
  }
}


class SMRandomRng64 : public Rng64 {
public:      // methods
  virtual uint64_t generate(uint64_t maxValue) override
  {
    maxValue = std::min(maxValue,
      static_cast<uint64_t>(std::numeric_limits<int>::max() - 1));
    return sm_random(static_cast<int>(maxValue + 1));
  }
};


int numIters = envAsIntOr(2000, "RANDOM_TEST_ITERS");


// Test `sm_random` for range and distribution bias.
//
// For the naive definition of `rand() % n`, the maximum useful range is
// 32768, as the maximum possible output value is 32767.
//
// Furthermore, within that range, there is strong bias toward lower
// values when using a range like 20000 that does not evenly divide
// 32768.
//
void test_smRandomDistributionBias()
{
  SMRandomRng64 rng;
  int size = envAsIntOr(10, "RANDOM_TEST_SIZE");

  measureBias(rng, size-1, numIters);
}


class RandomPrimRng : public Rng64 {
public:      // methods
  virtual uint64_t generate(uint64_t maxValue) override
  {
    // This just ignores `maxValue`.
    return sm_randomPrim<uint64_t>();
  }
};


void test_randomPrimDistributionBias()
{
  RandomPrimRng rng;
  measureBias(rng, UINT64_C(0xFFFFffffFFFFffff), numIters);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_random()
{
  test_smRandomDistributionBias();
  test_randomPrimDistributionBias();
}


// EOF
