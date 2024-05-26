// sm-ap-uint-test.cc
// Tests for sm-ap-uint.

// This file is in the public domain.

#include "sm-ap-uint.h"                // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-test.h"                   // VPVAL, EXPECT_EQ
#include "stringb.h"                   // stringb
#include "vector-utils.h"              // operator<< (std::vector), vectorReverseOf

#include <cstdlib>                     // std::rand

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// True while developing.
bool verbose = true;


#define myrandom(n) (std::rand()%(n))


// Convert a digit sequence with most significant first into an AP
// integer.
APUInteger<unsigned char> digitsToAP(std::vector<int> const &digits)
{
  APUInteger<unsigned char> ap;
  for (std::size_t i=0; i < digits.size(); ++i) {
    ap.setDigit(i, digits[digits.size() - i - 1]);
  }
  return ap;
}


// Convert an AP integer to digits with most significant first.
std::vector<int> apToDigits(APUInteger<unsigned char> const &n)
{
  std::vector<int> ret;

  int i = n.size()-1;

  // Skip high zeroes.
  while (i >= 0 && n.getDigit(i) == 0) {
    --i;
  }

  // Copy what's left.
  while (i >= 0) {
    ret.push_back(n.getDigit(i));
    --i;
  }

  return ret;
}


// Get the digits of `n` with most significant first.
std::string digitString(APUInteger<unsigned char> const &n)
{
  return stringb(apToDigits(n));
}


// Check that the vector of `n` is `expect`.
void checkDigits(APUInteger<unsigned char> const &n, char const *expect)
{
  std::string s = digitString(n);
  VPVAL(s);
  EXPECT_EQ(s, expect);
}


void checkOneAdd(std::vector<int> const &a,
                 std::vector<int> const &b,
                 std::vector<int> const &expect)
{
  APUInteger<unsigned char> apA = digitsToAP(a);
  APUInteger<unsigned char> apB = digitsToAP(b);
  APUInteger<unsigned char> apS = apA + apB;

  xassert(apS >= apA);
  xassert(apS >= apB);
  xassert(apS == apS);

  std::vector<int> actual = apToDigits(apS);
  try {
    EXPECT_EQ(actual, expect);
  }
  catch (XBase &x) {
    x.prependContext(stringb("a=" << a << ", b=" << b));
    throw;
  }
}


void testSpecificAdd()
{
  APUInteger<unsigned char> zero;
  checkDigits(zero, "[]");
  xassert(zero == zero);

  APUInteger<unsigned char> one(1);
  checkDigits(one, "[1]");
  xassert(zero < one);

  APUInteger<unsigned char> n(one);
  checkDigits(n, "[1]");
  n += one;
  checkDigits(n, "[2]");
  xassert(zero < one);
  xassert(one < n);

  APUInteger<unsigned char> two = one+one;
  checkDigits(two, "[2]");
  xassert(two == n);

  APUInteger<unsigned char> n128(128);
  checkDigits(n128, "[128]");

  APUInteger<unsigned char> n256 = n128+n128;
  checkDigits(n256, "[1 0]");

  xassert(n256 > n128);
  xassert(n128 > two);
  xassert(n256 == n128+n128);

  APUInteger<unsigned char> big1;
  big1.setDigit(0, 0xFF);
  big1.setDigit(1, 0xFF);
  big1.setDigit(2, 0xFF);
  checkDigits(big1, "[255 255 255]");

  xassert(big1 > n256);

  APUInteger<unsigned char> big2 = big1+one;
  checkDigits(big2, "[1 0 0 0]");
  big2 = one+big1;
  checkDigits(big2, "[1 0 0 0]");

  xassert(big2 > big1);

  checkDigits(big1+big2, "[1 255 255 255]");
  checkDigits(big1+big2+one, "[2 0 0 0]");

  checkOneAdd({   255, 255, 0, 255},
              {     1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  checkOneAdd({   255, 255, 0, 255},
              {0,   1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  // Make an integer with a redundant leading zero.
  APUInteger<unsigned char> oneWithLeading = digitsToAP({0, 1});
  xassert(oneWithLeading == one);

  // Check that we trim the redundant leading digit when converting back
  // to a vector.
  EXPECT_EQ(apToDigits(oneWithLeading), std::vector<int>{1});
}


// This is not a very thorough test because it only lightly tests the
// carry mechanism.  The specific tests above are a bit better.
void testRandomizedAdd()
{
  smbase_loopi(1000) {
    // Get two random 3-byte integers.
    int a0 = myrandom(256);
    int a1 = myrandom(256);
    int a2 = myrandom(256);

    int b0 = myrandom(256);
    int b1 = myrandom(256);
    int b2 = myrandom(256);

    int a = (a2 << 16) + (a1 << 8) + a0;
    int b = (b2 << 16) + (b1 << 8) + b0;

    // Add them.
    APUInteger<unsigned char> apA;
    apA.setDigit(0, a0);
    apA.setDigit(1, a1);
    apA.setDigit(2, a2);

    APUInteger<unsigned char> apB;
    apB.setDigit(0, b0);
    apB.setDigit(1, b1);
    apB.setDigit(2, b2);

    APUInteger<unsigned char> apS = apA + apB;

    int s = a+b;

    int s0 = s & 0xFF;
    int s1 = (s >> 8) & 0xFF;
    int s2 = (s >> 16) & 0xFF;
    int s3 = (s >> 24) & 0xFF;

    // Check the result digits.
    EXPECT_EQ((int)apS.getDigit(0), s0);
    EXPECT_EQ((int)apS.getDigit(1), s1);
    EXPECT_EQ((int)apS.getDigit(2), s2);
    EXPECT_EQ((int)apS.getDigit(3), s3);

    xassert(apS.size() <= 4);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_uint()
{
  testSpecificAdd();
  testRandomizedAdd();
}


// EOF
