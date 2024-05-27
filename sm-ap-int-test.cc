// sm-ap-int-test.cc
// Tests for sm-ap-int.h.

// This file is in the public domain.

#include "sm-ap-int.h"                 // module under test

#include "exc.h"                       // EXN_CONTEXT_CALL
#include "overflow.h"                  // addWithOverflowCheck, etc.
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-random.h"                 // sm_randomPrim
#include "sm-test.h"                   // VPVAL, EXPECT_EQ, verbose

#include <cstdint>                     // std::uint8_t, etc.
#include <cstdlib>                     // std::{atoi, getenv}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Count primitive arithmetic overflows.
int overflowCount=0, nonOverflowCount=0;


// Like in `sm-ap-uint-test.cc`, abstract the word size so I can test
// with several easily.
template <typename Word>
class APIntegerTest {
public:      // types
  typedef APInteger<Word> Integer;

public:      // methods
  void testSimple()
  {
    Integer zero;
    xassert(zero.isZero());
    xassert(!zero.isNegative());
    VPVAL(zero);

    Integer one(1);
    xassert(!one.isZero());
    xassert(!one.isNegative());
    VPVAL(one);

    Integer negOne(-1);
    xassert(!negOne.isZero());
    xassert(negOne.isNegative());
    VPVAL(negOne);
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
    EXPECT_EQ(actualQuotient, Integer(expectQuotient));
    EXPECT_EQ(actualRemainder, Integer(expectRemainder));
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
    catch (XOverflow &x) {
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
      EXPECT_EQ(apSum, Integer(sum));

      apSum = apA;
      apSum += apB;
      EXPECT_EQ(apSum, Integer(sum));

      ++nonOverflowCount;
    }
    catch (XOverflow &x) {
      ++overflowCount;
    }

    try {
      PRIM diff = subtractWithOverflowCheck(a, b);

      Integer apDiff = apA - apB;
      EXPECT_EQ(apDiff, Integer(diff));

      apDiff = apA;
      apDiff -= apB;
      EXPECT_EQ(apDiff, Integer(diff));

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
      EXPECT_EQ(apProd, Integer(prod));

      apProd = apA;
      apProd *= apB;
      EXPECT_EQ(apProd, Integer(prod));

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

      EXPECT_EQ(apQuot, Integer(quot));
      EXPECT_EQ(apRem, Integer(rem));

      // Check the operator forms.
      apQuot = apA / apB;
      apRem = apA % apB;
      EXPECT_EQ(apQuot, Integer(quot));
      EXPECT_EQ(apRem, Integer(rem));

      // Check the operator= forms.
      apQuot = apA;
      apQuot /= apB;
      apRem = apA;
      apRem %= apB;
      EXPECT_EQ(apQuot, Integer(quot));
      EXPECT_EQ(apRem, Integer(rem));

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
    EXPECT_EQ(actual, expect);
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
    int iters = 100;
    if (char const *itersStr = std::getenv("SM_AP_INT_TEST_ITERS")) {
      iters = std::atoi(itersStr);
      PVAL(iters);
    }

    smbase_loopi(iters) {
      testOneRandomArithmetic<std::int8_t>();
      testOneRandomArithmetic<std::uint8_t>();
      testOneRandomArithmetic<std::int16_t>();
      testOneRandomArithmetic<std::uint16_t>();
      testOneRandomArithmetic<std::int32_t>();
      testOneRandomArithmetic<std::uint32_t>();
      testOneRandomArithmetic<std::int64_t>();
      testOneRandomArithmetic<std::uint64_t>();
    }
  }

  void testAll()
  {
    testSimple();
    testDivide();
    testUnaryOps();
    testRandomArithmetic();
  }
};


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_int()
{
  APIntegerTest<std::uint8_t>().testAll();
  APIntegerTest<std::uint16_t>().testAll();
  APIntegerTest<std::uint32_t>().testAll();

  VPVAL(overflowCount);
  VPVAL(nonOverflowCount);
}


// EOF
