// sm-random-test.cc
// Tests for `sm-random` module.

#include "smbase/sm-random.h"          // module under test

#include "smbase/chained-cond.h"       // smbase::cc::le_lt
#include "smbase/gdv-ordered-map.h"    // gdv::GDVOrderedMap
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/most-sig-bit.h"       // smbase::mostSignificantBitOfArgPlusOne
#include "smbase/optional-util.h"      // optAccumulateMax
#include "smbase/save-restore.h"       // SET_RESTORE
#include "smbase/sm-env.h"             // smbase::envAsIntOr
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-sized-int.h"       // SM_FOREACH_SIZED_INT
#include "smbase/sm-test.h"            // EXPECT_EQ, TEST_CASE_EXPRS

#include <algorithm>                   // std::min
#include <cstdint>                     // std::uint64_t
#include <cstdlib>                     // std::rand
#include <iomanip>                     // std::setw
#include <limits>                      // std::numeric_limits
#include <string_view>                 // std::string_view
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


int const numIters = envAsIntOr(2000, "RANDOM_TEST_ITERS");
int const testSize = envAsIntOr(10, "RANDOM_TEST_SIZE");


// Test `sm_random` for range and distribution bias.
void test_smRandomDistributionBias()
{
  SMRandomRng64 rng;
  measureBias(rng, testSize-1, numIters);
}


// This is how `sm_random` was previously defined.  I keep it here to be
// able to do bias comparisons.
int old_sm_random(int n)
{
  return std::rand() % n;
}


class OldSMRandomRng64 : public Rng64 {
public:      // methods
  virtual uint64_t generate(uint64_t maxValue) override
  {
    maxValue = std::min(maxValue,
      static_cast<uint64_t>(std::numeric_limits<int>::max() - 1));
    return old_sm_random(static_cast<int>(maxValue + 1));
  }
};


// This tests the naive definition of `rand() % n`, for which the
// maximum useful range is 32768, as the maximum possible output value
// is 32767.
//
// Furthermore, within that range, there is strong bias toward lower
// values when using a range like 20000 that does not evenly divide
// 32768.
//
void test_oldSMRandomDistributionBias()
{
  OldSMRandomRng64 rng;
  measureBias(rng, testSize-1, numIters);
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


template <typename PRIM>
void exerciseRandomPrim()
{
  std::string_view typeName = GetTypeName<PRIM>::value;
  TEST_CASE_EXPRS("exerciseRandomPrim", typeName);
  for (int i=0; i < 10; ++i) {
    DIAG("  " << i << ": " << +sm_randomPrim<PRIM>());
  }
}


void test_sm_randomPrim()
{
  #define CALL_EXERCISE(type) \
    exerciseRandomPrim<type>();

  SM_FOREACH_SIZED_INT(CALL_EXERCISE)

  #undef CALL_EXERCISE
}


// Value for the interceptor to return.
int testInterceptorValue = 0;

int testInterceptorFunction(int n)
{
  // Here, it's up to the interceptor user to ensure the value is within
  // range.
  xassert(cc::z_le_lt(testInterceptorValue, n));

  return testInterceptorValue;
}


void test_sm_random_intercept()
{
  xassert(sm_random_intercept == nullptr);
  SET_RESTORE(sm_random_intercept, &testInterceptorFunction);

  testInterceptorValue = 10;
  EXPECT_EQ(sm_random(1000), 10);

  testInterceptorValue = 20;
  EXPECT_EQ(sm_random(1000), 20);
}


void test_RandomChoice()
{
  SET_RESTORE(sm_random_intercept, &testInterceptorFunction);

  {
    // When checking one at a time, the first 3 options are skipped, and
    // the fourth is active.
    testInterceptorValue = 3;

    RandomChoice choice(100);
    xassert(choice.remains());

    // The first 3 do not hit.
    xassert(!choice.check(1));
    xassert(!choice.check(1));
    xassert(!choice.check(1));
    xassert(choice.remains());

    // The 4th does.
    xassert(choice.check(1));
    xassert(!choice.remains());

    // And no more after that.
    xassert(!choice.check(1));
    xassert(!choice.remains());
  }

  {
    // Now do somewhat larger sections.
    testInterceptorValue = 20;

    RandomChoice choice(100);
    xassert(choice.remains());

    // Consume the first 18, no hit.
    xassert(!choice.check(7));
    xassert(!choice.check(10));
    xassert(!choice.check(1));

    // Hit on the next 5.
    xassert(choice.remains());
    xassert(choice.check(5));
    xassert(!choice.remains());

    // No more hits.
    xassert(!choice.check(5));
    xassert(!choice.remains());
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_random()
{
  test_smRandomDistributionBias();
  test_oldSMRandomDistributionBias();
  test_randomPrimDistributionBias();
  test_sm_randomPrim();
  test_sm_random_intercept();
  test_RandomChoice();
}


// EOF
