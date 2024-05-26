// sm-ap-uint-test.cc
// Tests for sm-ap-uint.

// This file is in the public domain.

#include "sm-ap-uint.h"                // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-test.h"                   // VPVAL, EXPECT_EQ
#include "stringb.h"                   // stringb
#include "vector-utils.h"              // operator<< (std::vector), vectorReverseOf

#include <cstdlib>                     // std::rand

#include <cstdint>                     // std::uint8_t

using namespace smbase;

using std::uint8_t;


OPEN_ANONYMOUS_NAMESPACE


// True while developing.
bool verbose = true;


#define myrandom(n) (std::rand()%(n))


typedef uint8_t Word;
typedef APUInteger<Word> Integer;
typedef Integer::Index Index;
typedef std::vector<int> DigitVector;


// Convert a digit sequence with most significant first into an AP
// integer.
Integer digitsToAP(DigitVector const &digits)
{
  Integer ap;
  for (std::size_t i=0; i < digits.size(); ++i) {
    ap.setDigit(i, digits[digits.size() - i - 1]);
  }
  return ap;
}


// Convert an AP integer to digits with most significant first.
DigitVector apToDigits(Integer const &n)
{
  DigitVector ret;

  Index i = n.size()-1;

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
std::string digitString(Integer const &n)
{
  return stringb(apToDigits(n));
}


// Check that the vector of `n` is `expect`.
void checkDigits(Integer const &n, char const *expect)
{
  std::string s = digitString(n);
  VPVAL(s);
  EXPECT_EQ(s, expect);
}


// Check that `n` equals `expecr64`.
void checkEquals(Integer const &n, uint64_t expect64)
{
  for (Index i=0; i < 8; ++i) {
    try {
      Word expectWord = (expect64 >> (8*i)) & 0xFF;
      EXPECT_EQ((int)n.getDigit(i), (int)expectWord);
    }
    catch (XBase &x) {
      x.prependContext(stringb("i=" << i));
      throw;
    }
  }
}


void checkOneAdd(DigitVector const &a,
                 DigitVector const &b,
                 DigitVector const &expect)
{
  try {
    Integer apA = digitsToAP(a);
    Integer apB = digitsToAP(b);
    Integer apS = apA + apB;

    xassert(apS >= apA);
    xassert(apS >= apB);
    xassert(apS == apS);

    DigitVector actual = apToDigits(apS);
    EXPECT_EQ(actual, expect);

    xassert(apS - apA == apB);
    xassert(apS - apB == apA);

    Integer zero;
    xassert(apA - apS == zero);
    xassert(apB - apS == zero);
  }
  catch (XBase &x) {
    x.prependContext(stringb("a=" << a << ", b=" << b));
    throw;
  }
}


void testSpecificAddSub()
{
  Integer zero;
  checkDigits(zero, "[]");
  xassert(zero == zero);
  xassert(zero+zero == zero);
  xassert(zero-zero == zero);

  Integer one(1);
  checkDigits(one, "[1]");
  xassert(zero < one);

  Integer n(one);
  checkDigits(n, "[1]");
  n += one;
  checkDigits(n, "[2]");
  xassert(zero < one);
  xassert(one < n);

  Integer two = one+one;
  checkDigits(two, "[2]");
  xassert(two == n);
  xassert(two - one == one);

  Integer n128(128);
  checkDigits(n128, "[128]");

  Integer n256 = n128+n128;
  checkDigits(n256, "[1 0]");

  xassert(n256 > n128);
  xassert(n128 > two);
  xassert(n256 == n128+n128);
  xassert(n256-n128 == n128);

  Integer big1;
  big1.setDigit(0, 0xFF);
  big1.setDigit(1, 0xFF);
  big1.setDigit(2, 0xFF);
  checkDigits(big1, "[255 255 255]");

  xassert(big1 > n256);

  Integer big2 = big1+one;
  checkDigits(big2, "[1 0 0 0]");
  big2 = one+big1;
  checkDigits(big2, "[1 0 0 0]");

  xassert(big2 > big1);
  xassert(big2-one == big1);

  checkDigits(big1+big2, "[1 255 255 255]");
  checkDigits(big1+big2+one, "[2 0 0 0]");

  checkOneAdd({   255, 255, 0, 255},
              {     1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  checkOneAdd({   255, 255, 0, 255},
              {0,   1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  // Make an integer with a redundant leading zero.
  Integer oneWithLeading = digitsToAP({0, 1});
  xassert(oneWithLeading == one);

  // Check that we trim the redundant leading digit when converting back
  // to a vector.
  EXPECT_EQ(apToDigits(oneWithLeading), DigitVector{1});
}


// This is not a very thorough test because it only lightly tests the
// carry mechanism.  The specific tests above are a bit better.
void testRandomizedAddSubMult()
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

    try {
      Integer apA;
      apA.setDigit(0, a0);
      apA.setDigit(1, a1);
      apA.setDigit(2, a2);
      checkEquals(apA, a);

      Integer apB;
      apB.setDigit(0, b0);
      apB.setDigit(1, b1);
      apB.setDigit(2, b2);
      checkEquals(apB, b);

      // Add and subtract them.
      {
        Integer apS = apA + apB;

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
        checkEquals(apS, s);

        xassert(apS - apA == apB);
        xassert(apS - apB == apA);

        Integer zero;
        xassert(apA - apS == zero);
        xassert(apB - apS == zero);
      }

      // Calculate `a * b0`.
      {
        Integer oneDigitProd(apA);
        oneDigitProd.multiplyWord(b0);

        uint64_t p = (uint64_t)a * b0;

        checkEquals(oneDigitProd, p);
      }
    }

    catch (XBase &x) {
      x.prependContext(stringb(
        "a0=" << a0 <<
        " a1=" << a1 <<
        " a2=" << a2 <<
        " a=" << a <<
        " b0=" << b0 <<
        " b1=" << b1 <<
        " b2=" << b2 <<
        " b=" << b <<
        " iter=" << i <<
        ""));
      throw;
    }
  }
}


static void testSpecificMult()
{
  Integer n;
  n.multiplyWord(0);
  xassert(n.isZero());

  n.multiplyWord(4);
  xassert(n.isZero());

  n += 1;
  checkDigits(n, "[1]");

  n.multiplyWord(100);
  checkDigits(n, "[100]");

  n.multiplyWord(16);
  n.multiplyWord(16);
  checkDigits(n, "[100 0]");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_uint()
{
  testSpecificAddSub();
  testSpecificMult();
  testRandomizedAddSubMult();
}


// EOF
