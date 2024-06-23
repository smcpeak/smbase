// sm-ap-int-test.cc
// Tests for sm-ap-int.h.

// This file is in the public domain.

#include "sm-ap-int.h"                 // module under test

#include "exc.h"                       // EXN_CONTEXT_CALL
#include "get-type-name.h"             // smbase::GetTypeName
#include "overflow.h"                  // addWithOverflowCheck, etc.
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-random.h"                 // sm_randomPrim
#include "sm-test.h"                   // VPVAL, EXPECT_EQ, verbose

#include <cstdint>                     // std::uint8_t, etc.
#include <cstdlib>                     // std::{atoi, getenv}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Check that `actual` equals `expect` and also check the invariants on
// `actual`.
#define SC_EXPECT_EQ(actual, expect) \
  EXPECT_EQ(actual, expect);         \
  actual.selfCheck();


// Count primitive arithmetic overflows.  This is done just to give me a
// sense of how many of my randomly-generated test inputs get stopped
// due to overflow before being used to exercise the AP integer class.
int overflowCount=0, nonOverflowCount=0;


// Like in `sm-ap-uint-test.cc`, abstract the word size so I can test
// with several easily.
template <typename Word, typename EmbeddedInt>
class APIntegerTest {
public:      // types
  typedef APInteger<Word, EmbeddedInt> Integer;

public:      // methods
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

    apA.selfCheck();
    apB.selfCheck();

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
    try {
      testSimple();
      testDivide();
      testUnaryOps();
      testRandomArithmetic();
    }
    catch (XBase &x) {
      x.prependContext(stringb(
        "Word=" << GetTypeName<Word>::value <<
        " EmbeddedInt=" << GetTypeName<EmbeddedInt>::value));
      throw;
    }
  }
};


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_int()
{
  // Test a specific case that has an issue.
  typedef APInteger<std::uint8_t, std::int8_t> Integer;
  Integer a(19);
  Integer b(200);
  Integer r = a%b;
  EXPECT_EQ(r, Integer(19));

  // Exercise a range of choices for the types.
  APIntegerTest<std:: uint8_t, std:: int8_t>().testAll();
  APIntegerTest<std::uint16_t, std:: int8_t>().testAll();
  APIntegerTest<std::uint32_t, std:: int8_t>().testAll();
  APIntegerTest<std::uint32_t, std::int16_t>().testAll();
  APIntegerTest<std::uint32_t, std::int32_t>().testAll();

  VPVAL(overflowCount);
  VPVAL(nonOverflowCount);
}


// EOF
