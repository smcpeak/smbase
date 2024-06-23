// overflow-test.cc
// Unit tests for 'overflow' module.

#include "overflow.h"                  // this module

#include "exc.h"                       // xassert
#include "save-restore.h"              // SET_RESTORE
#include "sm-iostream.h"               // cout
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // PVAL, DIAG, EXPECT_EQ_NUMBERS, verbose
#include "str.h"                       // streq

#include <cstdlib>                     // std::getenv
#include <optional>                    // std::optional

#include <stdint.h>                    // int64_t, uint64_t, INT64_C
#include <limits.h>                    // INT_MIN, INT_MIN

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Add, and expect success.
template <class NUM>
void testOneAdd(NUM a, NUM b, NUM expect)
{
  NUM actual = addWithOverflowCheck(a, b);
  EXPECT_EQ_NUMBERS(actual, expect);

  std::optional<NUM> actualOpt = addWithOverflowCheckOpt(a, b);
  EXPECT_EQ_NUMBERS(actualOpt.value(), expect);

  // Also test subtraction.
  NUM actualA = subtractWithOverflowCheck(expect, b);
  EXPECT_EQ_NUMBERS(actualA, a);

  std::optional<NUM> actualAOpt = subtractWithOverflowCheckOpt(expect, b);
  EXPECT_EQ_NUMBERS(actualAOpt.value(), a);

  NUM actualB = subtractWithOverflowCheck(expect, a);
  EXPECT_EQ_NUMBERS(actualB, b);

  std::optional<NUM> actualBOpt = subtractWithOverflowCheckOpt(expect, a);
  EXPECT_EQ_NUMBERS(actualBOpt.value(), b);
}


// Add, and expect overflow.
template <class NUM>
void testOneAddOv(NUM a, NUM b)
{
  try {
    addWithOverflowCheck(a, b);
    PVAL(typeid(a).name());
    PVAL(+a);
    PVAL(+b);
    xassert(!"testOneAddOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x.why() << endl;
    }
  }

  EXPECT_EQ(addWithOverflowCheckOpt(a, b).has_value(), false);
}


// Subtract, and expect overflow.
template <class NUM>
void testOneSubOv(NUM a, NUM b)
{
  try {
    subtractWithOverflowCheck(a, b);
    PVAL(typeid(a).name());
    PVAL(+a);
    PVAL(+b);
    xassert(!"testOneSubOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x.why() << endl;
    }
  }

  EXPECT_EQ(subtractWithOverflowCheckOpt(a, b).has_value(), false);
}


// Multiply, and expect success.
template <class NUM>
void testOneMultiply(NUM a, NUM b, NUM expect)
{
  NUM actual = multiplyWithOverflowCheck(a, b);
  EXPECT_EQ_NUMBERS(actual, expect);

  EXPECT_EQ_NUMBERS(multiplyWithOverflowCheckOpt(a, b).value(), expect);
}


// Multiply, and expect overflow.
template <class NUM>
void testOneMultiplyOv(NUM a, NUM b)
{
  try {
    multiplyWithOverflowCheck(a, b);
    PVAL(typeid(a).name());
    PVAL(+a);
    PVAL(+b);
    xassert(!"testOneMultiplyOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x.why() << endl;
    }
  }

  EXPECT_EQ(multiplyWithOverflowCheckOpt(a, b).has_value(), false);
}


// Divide, and expect success.
template <class NUM>
void testOneDivide(NUM a, NUM b, NUM expectQuotient, NUM expectRemainder)
{
  try {
    NUM actualQuotient, actualRemainder;
    divideWithOverflowCheck(actualQuotient, actualRemainder, a, b);
    EXPECT_EQ_NUMBERS(actualQuotient, expectQuotient);
    EXPECT_EQ_NUMBERS(actualRemainder, expectRemainder);

    xassert(divideWithOverflowCheckOpt(actualQuotient, actualRemainder, a, b));
    EXPECT_EQ_NUMBERS(actualQuotient, expectQuotient);
    EXPECT_EQ_NUMBERS(actualRemainder, expectRemainder);
  }
  catch (XBase &x) {
    x.prependContext(stringb(
      "a=" << +a << " b=" << +b));
    throw;
  }
}


// Divide, and expect overflow.
template <class NUM>
void testOneDivideOv(NUM a, NUM b)
{
  try {
    NUM q, r;
    divideWithOverflowCheck(q, r, a, b);
    PVAL(typeid(a).name());
    PVAL(+a);
    PVAL(+b);
    xassert(!"testOneDivideOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x << endl;
    }
  }

  NUM q, r;
  xassert(!divideWithOverflowCheckOpt(q, r, a, b));
}


// Test what happens with 'a+b' using int64_t, which must be able
// to represent all the possible values.
template <class SMALL_NUM>
void testOneAddSmallUsingInt64(SMALL_NUM a, SMALL_NUM b)
{
  int64_t largeA(a);
  int64_t largeB(b);
  int64_t result(largeA + largeB);

  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  // Try addition.
  if (minValue <= result && result <= maxValue) {
    // Should not overflow.
    SMALL_NUM actual = addWithOverflowCheck(a, b);

    // Check for correctness using the larger type.
    int64_t largeActual(actual);
    EXPECT_EQ_NUMBERS(largeActual, result);

    EXPECT_EQ_NUMBERS(
      static_cast<int64_t>(addWithOverflowCheckOpt(a, b).value()), result);
  }
  else {
    testOneAddOv(a, b);
  }

  // Try subtraction.
  result = largeA - largeB;
  if (minValue <= result && result <= maxValue) {
    SMALL_NUM actual = subtractWithOverflowCheck(a, b);
    int64_t largeActual(actual);
    EXPECT_EQ_NUMBERS(largeActual, result);

    EXPECT_EQ_NUMBERS(
      static_cast<int64_t>(subtractWithOverflowCheckOpt(a, b).value()), result);
  }
  else {
    testOneSubOv(a, b);
  }
}


// Same but for multiplication.
template <class SMALL_NUM>
void testOneMultiplySmallUsingInt64(SMALL_NUM a, SMALL_NUM b)
{
  int64_t largeA(a);
  int64_t largeB(b);
  int64_t result(largeA * largeB);

  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  if (minValue <= result && result <= maxValue) {
    // Should not overflow.
    SMALL_NUM actual = multiplyWithOverflowCheck(a, b);

    // Check for correctness using the larger type.
    int64_t largeActual(actual);
    EXPECT_EQ_NUMBERS(largeActual, result);

    EXPECT_EQ_NUMBERS(
      static_cast<int64_t>(multiplyWithOverflowCheckOpt(a, b).value()), result);
  }
  else {
    testOneMultiplyOv(a, b);
  }
}


// And division.
template <class SMALL_NUM>
void testOneDivideSmallUsingInt64(SMALL_NUM a, SMALL_NUM b)
{
  if (b == 0) {
    return;
  }

  int64_t largeA(a);
  int64_t largeB(b);
  int64_t expectQuotient(largeA / largeB);
  int64_t expectRemainder(largeA % largeB);

  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  if (minValue <= expectQuotient && expectQuotient <= maxValue &&
      minValue <= expectRemainder && expectRemainder <= maxValue) {
    // Should not overflow.
    SMALL_NUM actualQuotient, actualRemainder;
    divideWithOverflowCheck(actualQuotient, actualRemainder, a, b);

    // Check for correctness using the larger type.
    int64_t largeActualQuotient(actualQuotient);
    EXPECT_EQ_NUMBERS(largeActualQuotient, expectQuotient);
    int64_t largeActualRemainder(actualRemainder);
    EXPECT_EQ_NUMBERS(largeActualRemainder, expectRemainder);

    xassert(divideWithOverflowCheckOpt(actualQuotient, actualRemainder, a, b));

    largeActualQuotient = actualQuotient;
    EXPECT_EQ_NUMBERS(largeActualQuotient, expectQuotient);
    largeActualRemainder = actualRemainder;
    EXPECT_EQ_NUMBERS(largeActualRemainder, expectRemainder);
  }
  else {
    // Should overflow.
    testOneDivideOv(a, b);
  }
}


// Exhaustively check all pairs of SMALL_NUM.
template <class SMALL_NUM>
void testAddMultiplyAllSmallUsingInt64()
{
  SET_RESTORE(verbose, false);

  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  for (int64_t a = minValue; a <= maxValue; a++) {
    for (int64_t b = minValue; b <= maxValue; b++) {
      testOneAddSmallUsingInt64((SMALL_NUM)a, (SMALL_NUM)b);
      testOneMultiplySmallUsingInt64((SMALL_NUM)a, (SMALL_NUM)b);
      testOneDivideSmallUsingInt64((SMALL_NUM)a, (SMALL_NUM)b);
    }
  }
}


void testAddAndMultiply()
{
  testOneAdd<int8_t>(1, 2, 3);

  testOneAdd<int8_t>(126, 0, 126);     // 2 away from edge, stay.
  testOneAdd<int8_t>(126, 1, 127);     // 2 away, approach by 1.
  testOneAdd<int8_t>(127, -128, -1);   // At edge, move away by max.
  testOneAdd<int8_t>(127, -1, 126);    // At edge, move away by 1.
  testOneAdd<int8_t>(127, 0, 127);     // At edge, stay.
  testOneAddOv<int8_t>(127, 1);        // At edge, cross by 1.
  testOneAddOv<int8_t>(127, 2);        // At edge, cross by 2.
  testOneAddOv<int8_t>(127, 127);      // At edge, cross by max.

  testOneAdd<int8_t>(-127, 0, -127);
  testOneAdd<int8_t>(-127, -1, -128);
  testOneAdd<int8_t>(-128, 127, -1);
  testOneAdd<int8_t>(-128, 1, -127);
  testOneAdd<int8_t>(-128, 0, -128);
  testOneAddOv<int8_t>(-128, -1);
  testOneAddOv<int8_t>(-128, -2);
  testOneAddOv<int8_t>(-128, -128);

  testOneSubOv<uint8_t>(126, 127);     // 126 - 127 = -1
  testOneSubOv<uint8_t>(254, 255);     // 254 - 255 = -1
  testOneSubOv<uint8_t>(0, 1);         // 0 - 1 = -1
  testOneSubOv<uint8_t>(0, 127);       // 0 - 127 = -127
  testOneSubOv<uint8_t>(0, 255);       // 0 - 255 = -255

  testOneSubOv<int8_t>(127, -1);       // 127 - (-1) = 128
  testOneSubOv<int8_t>(-128, 1);       // -128 - 1 = -129
  testOneSubOv<int8_t>(127, -128);     // 128 - (-128) = 255
  testOneSubOv<int8_t>(0, -128);       // 0 - (-128) = 128
  testOneSubOv<int8_t>(-2, 127);       // -2 - 127 = -129

  testOneMultiplySmallUsingInt64<int8_t>(2, 3);
  testOneMultiplySmallUsingInt64<int8_t>(100, 100);
  testOneMultiplySmallUsingInt64<int8_t>(-1, 1);
  testOneMultiplyOv<int8_t>(-1, -128);

  // These are somewhat slow, taking around a second.
  bool runSlowTests = !!std::getenv("TEST_OVERFLOW_SLOW");
  if (runSlowTests) {
    DIAG("int8_t exhaustive");
    testAddMultiplyAllSmallUsingInt64<int8_t>();

    DIAG("uint8_t exhaustive");
    testAddMultiplyAllSmallUsingInt64<uint8_t>();
  }


  testOneAdd<int32_t>(1, 2, 3);
  testOneAdd<int32_t>(0x7ffffffe, 1, 0x7fffffff);
  testOneAddOv<int32_t>(0x7fffffff, 1);
  testOneMultiply<int32_t>(2, 3, 6);
  testOneMultiply<int32_t>(0x10000, 0x4000, 0x40000000);
  testOneMultiplyOv<int32_t>(0x10000, 0x8000);
  testOneMultiplyOv<int32_t>(INT_MIN, -1);

  testOneAdd<uint32_t>(1, 2, 3);

  testOneAdd<int64_t>(1, 2, 3);
  testOneAdd<int64_t>(INT64_C(0x7ffffffffffffffe), 1, INT64_C(0x7fffffffffffffff));
  testOneAddOv<int64_t>(INT64_C(0x7fffffffffffffff), 1);
  testOneMultiply<int64_t>(2, 3, 6);
  testOneMultiply<int64_t>(INT64_C(0x100000000), INT64_C(0x40000000), INT64_C(0x4000000000000000));
  testOneMultiplyOv<int64_t>(INT64_C(0x100000000), INT64_C(0x80000000));
  testOneMultiplyOv<int64_t>(INT64_MIN, -1);

  testOneAdd<uint64_t>(1, 2, 3);
  testOneAdd<uint64_t>(UINT64_C(0xfffffffffffffffe), 1, UINT64_C(0xffffffffffffffff));
  testOneAddOv<uint64_t>(UINT64_C(0xffffffffffffffff), 1);
  testOneMultiply<uint64_t>(2, 3, 6);
  testOneMultiply<uint64_t>(UINT64_C(0x100000000), UINT64_C(0x80000000), UINT64_C(0x8000000000000000));
  testOneMultiplyOv<uint64_t>(UINT64_C(0x100000000), UINT64_C(0x100000000));
}


void testDivide()
{
  // Divide by 0.
  testOneDivideOv<int8_t>(   0,  0);
  testOneDivideOv<int8_t>(   1,  0);
  testOneDivideOv<int8_t>(  -1,  0);
  testOneDivideOv<int8_t>( 127,  0);
  testOneDivideOv<int8_t>(-128,  0);

  // Divide by 1.
  testOneDivide<int8_t>(   0,    1,    0,   0);
  testOneDivide<int8_t>(   1,    1,    1,   0);
  testOneDivide<int8_t>(  -1,    1,   -1,   0);
  testOneDivide<int8_t>( 127,    1,  127,   0);
  testOneDivide<int8_t>(-128,    1, -128,   0);

  // Divide by -1.
  testOneDivide<int8_t>  (   0,   -1,    0,   0);
  testOneDivide<int8_t>  (   1,   -1,   -1,   0);
  testOneDivide<int8_t>  (  -1,   -1,    1,   0);
  testOneDivide<int8_t>  ( 127,   -1, -127,   0);
  testOneDivide<int8_t>  (-127,   -1,  127,   0);
  testOneDivideOv<int8_t>(-128,   -1);

  // Divide by 2.
  testOneDivide<int8_t>(   0,    2,    0,   0);
  testOneDivide<int8_t>(   1,    2,    0,   1);
  testOneDivide<int8_t>(   2,    2,    1,   0);
  testOneDivide<int8_t>(   3,    2,    1,   1);
  testOneDivide<int8_t>(  -1,    2,    0,  -1);     // Note truncation toward zero, not -inf.
  testOneDivide<int8_t>( 127,    2,   63,   1);
  testOneDivide<int8_t>(-127,    2,  -63,  -1);
  testOneDivide<int8_t>(-128,    2,  -64,   0);

  // Divide by -2.
  testOneDivide<int8_t>(   0,   -2,    0,   0);
  testOneDivide<int8_t>(   1,   -2,    0,   1);
  testOneDivide<int8_t>(   2,   -2,   -1,   0);
  testOneDivide<int8_t>(   3,   -2,   -1,   1);
  testOneDivide<int8_t>(  -1,   -2,    0,  -1);
  testOneDivide<int8_t>( 127,   -2,  -63,   1);
  testOneDivide<int8_t>(-127,   -2,   63,  -1);
  testOneDivide<int8_t>(-128,   -2,   64,   0);

  // Divide by 127.
  testOneDivide<int8_t>(   0,  127,    0,   0);
  testOneDivide<int8_t>(   1,  127,    0,   1);
  testOneDivide<int8_t>( 126,  127,    0, 126);
  testOneDivide<int8_t>( 127,  127,    1,   0);
  testOneDivide<int8_t>(  -1,  127,    0,  -1);
  testOneDivide<int8_t>(  -2,  127,    0,  -2);
  testOneDivide<int8_t>(-127,  127,   -1,   0);
  testOneDivide<int8_t>(-128,  127,   -1,  -1);

  // Divide by -127.
  testOneDivide<int8_t>(   0, -127,    0,   0);
  testOneDivide<int8_t>(   1, -127,    0,   1);
  testOneDivide<int8_t>( 126, -127,    0, 126);
  testOneDivide<int8_t>( 127, -127,   -1,   0);
  testOneDivide<int8_t>(  -1, -127,    0,  -1);
  testOneDivide<int8_t>(  -2, -127,    0,  -2);
  testOneDivide<int8_t>(-127, -127,    1,   0);
  testOneDivide<int8_t>(-128, -127,    1,  -1);

  // Divide by -128.
  testOneDivide<int8_t>(   0, -128,    0,   0);
  testOneDivide<int8_t>(   1, -128,    0,   1);
  testOneDivide<int8_t>( 126, -128,    0, 126);
  testOneDivide<int8_t>( 127, -128,    0, 127);
  testOneDivide<int8_t>(  -1, -128,    0,  -1);
  testOneDivide<int8_t>(  -2, -128,    0,  -2);
  testOneDivide<int8_t>(-127, -128,    0,-127);
  testOneDivide<int8_t>(-128, -128,    1,   0);
}



template <class DEST, class SRC>
void cwlSuccess(SRC src)
{
  DEST dest = 0;
  convertWithoutLoss(dest, src);
  xassert(static_cast<SRC>(dest) == src);
}


template <class DEST, class SRC>
void cwlFail(SRC src)
{
  DEST dest = 0;
  try {
    convertWithoutLoss(dest, src);
    xfailure("should have failed");
  }
  catch (XOverflow &x) {
    DIAG("as expected: " << x);
  }
}


enum SomeEnum {
  SE0,
  SE1,
  SE2,
  SE_MAX = INT_MAX,
  SE_MIN = INT_MIN
};


void testConvertWithoutLoss()
{
  cwlSuccess<int, int>(3);
  cwlFail<char, int>(12345);

  cwlSuccess<unsigned, int>(-3);
  cwlFail<unsigned char, int>(-3);
  cwlSuccess<unsigned, signed char>(-3);

  cwlSuccess<int, SomeEnum>(SE2);
  cwlSuccess<int, SomeEnum>(SE_MAX);
  cwlSuccess<int, SomeEnum>(SE_MIN);
  cwlSuccess<unsigned, SomeEnum>(SE2);
  cwlSuccess<unsigned, SomeEnum>(SE_MAX);
  cwlSuccess<unsigned, SomeEnum>(SE_MIN);

  cwlSuccess<unsigned char, SomeEnum>(SE2);
  cwlFail<unsigned char, SomeEnum>(SE_MAX);
}


template <class DEST, class SRC>
void cnSuccess(SRC src)
{
  DEST dest = convertNumber<DEST>(src);
  xassert(dest == src);
}


template <class DEST, class SRC>
void cnFail(SRC src)
{
  try {
    convertNumber<DEST>(src);
    xfailure("should have failed");
  }
  catch (XOverflow &x) {
    DIAG("as expected: " << x);
  }
}


void testConvertNumber()
{
  cnSuccess<int, int>(3);
  cnFail<char, int>(1234);
  cnFail<unsigned, int>(-1);
  cnFail<int, unsigned>(UINT_MAX);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_overflow()
{
  char const *selTest = getenv("TEST_OVERFLOW_SELTEST");

  #define RUNTEST(func)                              \
    if (selTest==nullptr || streq(selTest, #func)) { \
      DIAG(#func);                                   \
      func();                                        \
    }

  RUNTEST(testAddAndMultiply);
  RUNTEST(testDivide);
  RUNTEST(testConvertWithoutLoss);
  RUNTEST(testConvertNumber);

  #undef RUNTEST
}


// EOF
