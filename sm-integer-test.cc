// sm-integer-test.cc
// Tests for `sm-integer` module.

// This file is in the public domain.

#include "sm-integer.h"                // module under test

#include "smbase/overflow.h"           // addWithOverflowCheck, etc.
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-random.h"          // sm_randomPrim
#include "smbase/sm-sized-int.h"       // SM_FOREACH_SIZED_INT
#include "smbase/sm-test.h"            // VPVAL, envRandomizedTestIters
#include "smbase/xoverflow.h"          // XOverflow

#include <cstdint>                     // std::[u]int{8,16,32}_t

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Check that `actual` equals `expect` and also check the invariants on
// `actual`.
#define SC_EXPECT_EQ(actual, expect) \
  EXPECT_EQ(actual, expect);         \
  actual.selfCheck();


// Count primitive arithmetic overflows.
int overflowCount=0, nonOverflowCount=0;


void testSimple()
{
  Integer zero;
  xassert(zero.isZero());
  xassert(!zero.isNegative());
  VPVAL(zero);
  zero.selfCheck();

  Integer one(1);
  xassert(!one.isZero());
  xassert(!one.isNegative());
  VPVAL(one);
  one.selfCheck();

  Integer negOne(-1);
  xassert(!negOne.isZero());
  xassert(negOne.isNegative());
  VPVAL(negOne);
  negOne.selfCheck();
}


void testOneDivide(
  int dividend,
  int divisor,
  int expectQuotient,
  int expectRemainder)
{
  Integer actualQuotient, actualRemainder;
  Integer::divide(
    actualQuotient,
    actualRemainder,
    dividend,
    divisor);
  SC_EXPECT_EQ(actualQuotient, Integer(expectQuotient));
  SC_EXPECT_EQ(actualRemainder, Integer(expectRemainder));
}


void testOneDivideOv(
  int dividend,
  int divisor)
{
  try {
    Integer actualQuotient, actualRemainder;
    Integer::divide(
      actualQuotient,
      actualRemainder,
      dividend,
      divisor);
    xfailure("should have failed");
  }
  catch (XDivideByZero &x) {
    // As expected.
    VPVAL(x);
  }
}


// Test division using the examples in the spec.
void testDivide()
{
  testOneDivide( 5,  3,  1,  2);
  testOneDivide(-5,  3, -1, -2);
  testOneDivide( 5, -3, -1,  2);
  testOneDivide(-5, -3,  1, -2);
  testOneDivideOv(-1, 0);
}


template <typename PRIM>
void testOneRandomArithmetic()
{
  PRIM a = sm_randomPrim<PRIM>();
  PRIM b = sm_randomPrim<PRIM>();

  Integer apA(a);
  Integer apB(b);

  try {
    PRIM sum = addWithOverflowCheck(a, b);

    Integer apSum = apA + apB;
    SC_EXPECT_EQ(apSum, Integer(sum));

    apSum = apA;
    apSum += apB;
    SC_EXPECT_EQ(apSum, Integer(sum));

    ++nonOverflowCount;
  }
  catch (XOverflow &x) {
    ++overflowCount;
  }

  try {
    PRIM diff = subtractWithOverflowCheck(a, b);

    Integer apDiff = apA - apB;
    SC_EXPECT_EQ(apDiff, Integer(diff));

    apDiff = apA;
    apDiff -= apB;
    SC_EXPECT_EQ(apDiff, Integer(diff));

    ++nonOverflowCount;
  }
  catch (XOverflow &x) {
    ++overflowCount;
  }
  catch (XBase &x) {
    x.prependContext(stringb(
      "computing a-b for a=" << apA << " b=" << apB));
    throw;
  }

  try {
    PRIM prod = multiplyWithOverflowCheck(a, b);

    Integer apProd = apA * apB;
    SC_EXPECT_EQ(apProd, Integer(prod));

    apProd = apA;
    apProd *= apB;
    SC_EXPECT_EQ(apProd, Integer(prod));

    ++nonOverflowCount;
  }
  catch (XOverflow &x) {
    ++overflowCount;
  }
  catch (XBase &x) {
    x.prependContext(stringb(
      "computing a*b for a=" << apA << " b=" << apB));
    throw;
  }

  try {
    PRIM quot, rem;
    divideWithOverflowCheck(quot, rem, a, b);

    Integer apQuot, apRem;
    Integer::divide(
      apQuot,
      apRem,
      apA,
      apB);

    SC_EXPECT_EQ(apQuot, Integer(quot));
    SC_EXPECT_EQ(apRem, Integer(rem));

    // Check the operator forms.
    apQuot = apA / apB;
    apRem = apA % apB;
    SC_EXPECT_EQ(apQuot, Integer(quot));
    SC_EXPECT_EQ(apRem, Integer(rem));

    // Check the operator= forms.
    apQuot = apA;
    apQuot /= apB;
    apRem = apA;
    apRem %= apB;
    SC_EXPECT_EQ(apQuot, Integer(quot));
    SC_EXPECT_EQ(apRem, Integer(rem));

    ++nonOverflowCount;
  }
  catch (XOverflow &x) {
    ++overflowCount;
  }
  catch (XBase &x) {
    x.prependContext(stringb(
      "computing a/b for a=" << apA << " b=" << apB));
    throw;
  }
}


void testOneUnary(
  Integer const &input,
  bool isPlus,
  Integer const &expect)
{
  EXN_CONTEXT_CALL(testOneUnary, (input, isPlus));

  Integer actual;
  if (isPlus) {
    actual = +input;
  }
  else {
    actual = -input;
  }
  SC_EXPECT_EQ(actual, expect);
}


void testUnaryOps()
{
  bool isPlus = true;
  testOneUnary(0, isPlus, 0);
  testOneUnary(1, isPlus, 1);
  testOneUnary(100, isPlus, 100);

  isPlus = false;
  testOneUnary(0, isPlus, 0);
  testOneUnary(1, isPlus, -1);
  testOneUnary(100, isPlus, -100);
}


void testRandomArithmetic()
{
  int const iters =
    envRandomizedTestIters(100, "SM_INTEGER_TEST_ITERS");

  smbase_loopi(iters) {
    #define CALL_TEST_ONE(type) \
      testOneRandomArithmetic<type>();

    SM_FOREACH_SIZED_INT(CALL_TEST_ONE)

    #undef CALL_TEST_ONE
  }
}


template <typename PRIM>
void testOneGetAs(Integer i, PRIM expect)
{
  EXN_CONTEXT_CALL(testOneGetAs, (i));

  std::optional<PRIM> actualOpt = i.template getAsOpt<PRIM>();
  EXPECT_EQ(actualOpt.has_value(), true);
  EXPECT_EQ_NUMBERS(*actualOpt, expect);

  PRIM actual = i.template getAs<PRIM>();
  EXPECT_EQ_NUMBERS(actual, expect);
}

template <typename PRIM>
void testOneGetAsFail(Integer i)
{
  EXN_CONTEXT_CALL(testOneGetAsFail, (i));

  std::optional<PRIM> actualOpt = i.template getAsOpt<PRIM>();
  xassert(!actualOpt.has_value());

  try {
    i.template getAs<PRIM>();
    xfailure("should have failed");
  }
  catch (XOverflow &x) {
    // Failed as expected
    VPVAL(x);
  }
}


void testGetAs()
{
  testOneGetAs<std::int8_t>(127, 127);
  testOneGetAsFail<std::int8_t>(128);

  testOneGetAs<std::int8_t>(-127, -127);
  testOneGetAs<std::int8_t>(-128, -128);
  testOneGetAsFail<std::int8_t>(-129);

  testOneGetAs<std::uint8_t>(255, 255);
  testOneGetAsFail<std::uint8_t>(256);

  testOneGetAs<std::uint8_t>(0, 0);
  testOneGetAsFail<std::uint8_t>(-1);

  testOneGetAs<std::int16_t>(0x7FFE, 0x7FFE);
  testOneGetAs<std::int16_t>(0x7FFF, 0x7FFF);
  testOneGetAsFail<std::int16_t>(0x8000);

  testOneGetAs<std::int16_t>(-0x7FFE, -0x7FFE);
  testOneGetAs<std::int16_t>(-0x7FFF, -0x7FFF);
  testOneGetAs<std::int16_t>(-0x8000, -0x8000);
  testOneGetAsFail<std::int16_t>(-0x8001);

  testOneGetAs<std::uint16_t>(0, 0);
  testOneGetAsFail<std::uint16_t>(-1);

  testOneGetAs<std::uint16_t>(0xFFFE, 0xFFFE);
  testOneGetAs<std::uint16_t>(0xFFFF, 0xFFFF);
  testOneGetAsFail<std::uint16_t>(0x10000);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_integer()
{
  testSimple();
  testDivide();
  testUnaryOps();
  testGetAs();
  testRandomArithmetic();

  VPVAL(overflowCount);
  VPVAL(nonOverflowCount);
}


// EOF
