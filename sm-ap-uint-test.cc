// sm-ap-uint-test.cc
// Tests for sm-ap-uint.

// This file is in the public domain.

#include "sm-ap-uint.h"                // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-test.h"                   // VPVAL, EXPECT_EQ, EXPECT_MATCHES_REGEX
#include "stringb.h"                   // stringb
#include "vector-utils.h"              // operator<< (std::vector), vectorReverseOf

#include <cstdlib>                     // std::rand

#include <cstdint>                     // std::uint8_t, UINT64_C

using namespace smbase;

using std::uint8_t;


OPEN_ANONYMOUS_NAMESPACE


// True while developing.
bool verbose = true;


#define myrandom(n) (std::rand()%(n))


typedef uint8_t Word;
typedef APUInteger<Word> Integer;
typedef Integer::Index Index;
typedef std::vector<int> WordVector;


// Convert a word sequence with most significant first into an AP
// integer.
Integer wordsToAP(WordVector const &words)
{
  Integer ap;
  for (std::size_t i=0; i < words.size(); ++i) {
    ap.setWord(i, words[words.size() - i - 1]);
  }
  return ap;
}


// Convert an AP integer to words with most significant first.
WordVector apToWords(Integer const &n)
{
  WordVector ret;

  Index i = n.size()-1;

  // Skip high zeroes.
  while (i >= 0 && n.getWord(i) == 0) {
    --i;
  }

  // Copy what's left.
  while (i >= 0) {
    ret.push_back(n.getWord(i));
    --i;
  }

  return ret;
}


// Get the words of `n` with most significant first.
std::string wordsString(Integer const &n)
{
  return stringb(apToWords(n));
}


// Check that the vector of `n` is `expect`.
void checkWords(Integer const &n, char const *expect)
{
  std::string s = wordsString(n);
  VPVAL(s);
  EXPECT_EQ(s, expect);
}


std::string bitsString(Integer const &n)
{
  std::ostringstream oss;
  for (Index i = n.maxBitIndex(); i >= 0; --i) {
    oss << (n.getBit(i)? "1" : "0");
  }
  return oss.str();
}


void checkBits(Integer const &n, char const *expect)
{
  std::string s = bitsString(n);
  EXPECT_EQ(s, expect);
}


// Check that `n` equals `expecr64`.
void checkEquals(Integer const &n, uint64_t expect64)
{
  for (Index i=0; i < 8; ++i) {
    try {
      Word expectWord = (expect64 >> (8*i)) & 0xFF;
      EXPECT_EQ((int)n.getWord(i), (int)expectWord);
    }
    catch (XBase &x) {
      x.prependContext(stringb("i=" << i));
      throw;
    }
  }
}


void checkOneAdd(WordVector const &a,
                 WordVector const &b,
                 WordVector const &expect)
{
  try {
    Integer apA = wordsToAP(a);
    Integer apB = wordsToAP(b);
    Integer apS = apA + apB;

    xassert(apS >= apA);
    xassert(apS >= apB);
    xassert(apS == apS);

    WordVector actual = apToWords(apS);
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
  checkWords(zero, "[]");
  xassert(zero == zero);
  xassert(zero+zero == zero);
  xassert(zero-zero == zero);
  xassert(zero.isZero());
  EXPECT_EQ(zero.maxBitIndex(), (Index)-1);
  checkBits(zero, "");

  Integer one;
  one.setWord(0, 1);
  checkWords(one, "[1]");
  xassert(zero < one);
  xassert(!one.isZero());
  EXPECT_EQ(one.maxBitIndex(), (Index)0);
  checkBits(one, "1");

  Integer n(one);
  checkWords(n, "[1]");
  n += one;
  checkWords(n, "[2]");
  xassert(zero < one);
  xassert(one < n);

  Integer two = one+one;
  checkWords(two, "[2]");
  xassert(two == n);
  xassert(two - one == one);
  EXPECT_EQ(two.maxBitIndex(), (Index)1);
  checkBits(two, "10");

  Integer n128;
  n128.setWord(0, 128);
  checkWords(n128, "[128]");
  EXPECT_EQ(n128.maxBitIndex(), (Index)7);
  checkBits(n128, "10000000");

  Integer n256 = n128+n128;
  checkWords(n256, "[1 0]");
  EXPECT_EQ(n256.maxBitIndex(), (Index)8);
  checkBits(n256, "100000000");

  xassert(n256 > n128);
  xassert(n128 > two);
  xassert(n256 == n128+n128);
  xassert(n256-n128 == n128);

  Integer big1;
  big1.setWord(0, 0xFF);
  big1.setWord(1, 0xFF);
  big1.setWord(2, 0xFF);
  checkWords(big1, "[255 255 255]");
  EXPECT_EQ(big1.maxBitIndex(), (Index)23);
  checkBits(big1, "111111111111111111111111");

  xassert(big1 > n256);

  Integer big2 = big1+one;
  checkWords(big2, "[1 0 0 0]");
  big2 = one+big1;
  checkWords(big2, "[1 0 0 0]");
  EXPECT_EQ(big2.maxBitIndex(), (Index)24);
  checkBits(big2, "1000000000000000000000000");

  xassert(big2 > big1);
  xassert(big2-one == big1);

  checkWords(big1+big2, "[1 255 255 255]");
  checkWords(big1+big2+one, "[2 0 0 0]");

  big2.setBit(0, 1);
  checkBits(big2, "1000000000000000000000001");

  big2.setBit(0, 1);
  checkBits(big2, "1000000000000000000000001");

  big2.setBit(7, 0);
  checkBits(big2, "1000000000000000000000001");

  big2.setBit(7, 1);
  checkBits(big2, "1000000000000000010000001");

  big2.setBit(8, 1);
  checkBits(big2, "1000000000000000110000001");

  big2.setBit(23, 1);
  checkBits(big2, "1100000000000000110000001");
  checkWords(big2, "[1 128 1 129]");

  big2.leftShiftByBits(3);
  checkBits(big2, "1100000000000000110000001000");

  big2.rightShiftOneBit();
  checkBits(big2, "110000000000000011000000100");

  big2.rightShiftOneBit();
  checkBits(big2, "11000000000000001100000010");

  big2.rightShiftOneBit();
  checkBits(big2, "1100000000000000110000001");

  big2.rightShiftOneBit();
  checkBits(big2, "110000000000000011000000");

  big2.rightShiftOneBit();
  checkBits(big2, "11000000000000001100000");

  big2.leftShiftByBits(20);
  checkBits(big2, "1100000000000000110000000000000000000000000");

  checkOneAdd({   255, 255, 0, 255},
              {     1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  checkOneAdd({   255, 255, 0, 255},
              {0,   1,   1, 0,   3},
              {1,   1,   0, 1,   2});

  // Make an integer with a redundant leading zero.
  Integer oneWithLeading = wordsToAP({0, 1});
  xassert(oneWithLeading == one);

  // Check that we trim the redundant leading word when converting back
  // to a vector.
  EXPECT_EQ(apToWords(oneWithLeading), WordVector{1});
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

    uint64_t a = (a2 << 16) + (a1 << 8) + a0;
    uint64_t b = (b2 << 16) + (b1 << 8) + b0;

    try {
      Integer apA;
      apA.setWord(0, a0);
      apA.setWord(1, a1);
      apA.setWord(2, a2);
      checkEquals(apA, a);

      Integer apB;
      apB.setWord(0, b0);
      apB.setWord(1, b1);
      apB.setWord(2, b2);
      checkEquals(apB, b);

      // Add and subtract them.
      {
        Integer apS = apA + apB;

        uint64_t s = a+b;

        int s0 = s & 0xFF;
        int s1 = (s >> 8) & 0xFF;
        int s2 = (s >> 16) & 0xFF;
        int s3 = (s >> 24) & 0xFF;

        // Check the result words.
        EXPECT_EQ((int)apS.getWord(0), s0);
        EXPECT_EQ((int)apS.getWord(1), s1);
        EXPECT_EQ((int)apS.getWord(2), s2);
        EXPECT_EQ((int)apS.getWord(3), s3);

        xassert(apS.size() <= 4);
        checkEquals(apS, s);

        xassert(apS - apA == apB);
        xassert(apS - apB == apA);

        Integer zero;
        xassert(apA - apS == zero);
        xassert(apB - apS == zero);

        // Test +=
        Integer apS2 = apA;
        apS2 += apB;
        xassert(apS2 == apS);

        // Test -=
        Integer apA2 = apS;
        apA2 -= apB;
        xassert(apA2 == apA);
      }

      // Calculate `a * b0`.
      {
        Integer oneWordProd(apA);
        oneWordProd.multiplyWord(b0);

        uint64_t p = a * b0;

        checkEquals(oneWordProd, p);
      }

      // Calculate `a * b`.
      {
        Integer prod = apA * apB;
        checkEquals(prod, a * b);

        // Test *=
        Integer prod2 = apA;
        prod2 *= apB;
        xassert(prod2 == prod);
      }

      // Calculate `a/b` and `a%b`.
      {
        uint64_t q = a/b;
        uint64_t r = a%b;

        Integer apQ = apA/apB;
        Integer apR = apA%apB;

        checkEquals(apQ, q);
        checkEquals(apR, r);

        xassert(Integer() <= apR);
        xassert(             apR < apB);

        xassert(apB * apQ + apR == apA);

        // Test /=
        Integer apQ2 = apA;
        apQ2 /= apB;
        xassert(apQ2 == apQ);

        // Test %=
        Integer apR2 = apA;
        apR2 %= apB;
        xassert(apR2 == apR);
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

  Integer one;
  one.setWord(0, 1);
  n += one;
  checkWords(n, "[1]");

  n.multiplyWord(100);
  checkWords(n, "[100]");

  n.multiplyWord(16);
  n.multiplyWord(16);
  checkWords(n, "[100 0]");
}


static void testLeftShift()
{
  Integer n = wordsToAP({1,2,3});

  n.leftShiftByWords(0);
  checkWords(n, "[1 2 3]");

  n.leftShiftByWords(3);
  checkWords(n, "[1 2 3 0 0 0]");

  Integer one;
  one.setWord(0, 1);
  n += one;
  checkWords(n, "[1 2 3 0 0 1]");

  n.leftShiftByWords(1);
  checkWords(n, "[1 2 3 0 0 1 0]");
}


static void testReadWriteAsHex()
{
  Integer n;
  n.setWord(0, 0xF);
  DIAG(n);
  std::string digits = stringb(n);
  EXPECT_EQ(digits, "0xF");
  EXPECT_EQ(Integer::fromDigits(digits), n);

  Integer h12;
  h12.setWord(0, 0x12);
  DIAG(h12);
  digits = stringb(h12);
  EXPECT_EQ(digits, "0x12");
  EXPECT_EQ(Integer::fromDigits(digits), h12);

  // Leading zero for those after the first (here, "0F").
  n.setWord(1, 0x45);
  digits = stringb(n);
  EXPECT_EQ(digits, "0x450F");
  EXPECT_EQ(Integer::fromDigits(digits), n);

  // No leading zero for the first.
  n.setWord(2, 0x3);
  digits = stringb(n);
  EXPECT_EQ(digits, "0x3450F");
  EXPECT_EQ(Integer::fromDigits(digits), n);

  digits = stringb(Integer());
  EXPECT_EQ(digits, "0x0");
  EXPECT_EQ(Integer::fromDigits(digits), Integer());
}


template <typename PRIM>
static void expectFailConvert(
  Integer const &n,
  char const *expectRegex)
{
  bool failed = true;
  try {
    n.getAs<PRIM>();
    failed = false;
  }
  catch (XBase &x) {
    DIAG(x);
    EXPECT_MATCHES_REGEX(x.getMessage(), expectRegex);
  }

  if (!failed) {
    xfailure("should have failed!");
  }
}


static void testConstructFromPrim()
{
  Integer n(0);
  EXPECT_EQ(n, Integer());
  EXPECT_EQ(n.getAs<int>(), 0);

  Integer three;
  three.setWord(0, 3);
  EXPECT_EQ(Integer(3), three);
  EXPECT_EQ(three.getAs<int>(), 3);
  EXPECT_EQ(three.getAs<uint8_t>(), (uint8_t)3);

  Integer h1234;
  h1234.setWord(1, 0x12);
  h1234.setWord(0, 0x34);
  EXPECT_EQ(Integer(0x1234), h1234);
  EXPECT_EQ(h1234.getAs<int>(), 0x1234);

  Integer h12345678(h1234);
  h12345678.leftShiftByWords(2);
  h12345678.setWord(1, 0x56);
  h12345678.setWord(0, 0x78);
  EXPECT_EQ(Integer(0x12345678), h12345678);
  EXPECT_EQ(h12345678.getAs<int>(), 0x12345678);

  uint64_t big64 = UINT64_C(0x1234567812345678);
  Integer big(big64);
  EXPECT_EQ(big, Integer::fromDigits("0x1234567812345678"));
  EXPECT_EQ(big.getAs<uint64_t>(), big64);

  uint64_t biggest64 = UINT64_C(0xFFFFFFFFFFFFFFFF);
  Integer biggest(biggest64);
  EXPECT_EQ(biggest, Integer::fromDigits("0xFFFFFFFFFFFFFFFF"));
  EXPECT_EQ(biggest.getAs<uint64_t>(), biggest64);

  xassert(biggest.getAsOpt<int64_t>() == std::nullopt);

  expectFailConvert<int64_t>(biggest,
    "value 0xFFFF.* to a signed 64-bit integer");
  expectFailConvert<uint8_t>(biggest,
    "value 0xFFFF.* to an unsigned 8-bit integer");

  try {
    Integer n(-1);
    xfailure("should have failed");
  }
  catch (XBase &x) {
    EXPECT_MATCHES_REGEX(x.getMessage(),
      "from negative value -1");
  }

  uint8_t hff = 0xFF;
  Integer small(hff);
  EXPECT_EQ(small, Integer::fromDigits("0xFF"));
  EXPECT_EQ(small.getAs<uint8_t>(), hff);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_uint()
{
  testSpecificAddSub();
  testSpecificMult();
  testRandomizedAddSubMult();
  testLeftShift();
  testReadWriteAsHex();
  testConstructFromPrim();
}


// EOF
