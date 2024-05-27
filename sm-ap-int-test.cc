// sm-ap-int-test.cc
// Tests for sm-ap-int.h.

#include "sm-ap-int.h"                 // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "sm-random.h"                 // sm_randomPrim
#include "sm-test.h"                   // VPVAL, EXPECT_EQ

#include <cstdint>                     // std::uint8_t, etc.

// This file is in the public domain.

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// True while developing.
bool verbose = true;


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
      ++nonOverflowCount;
    }
    catch (XOverflow &x) {
      ++overflowCount;
    }

    try {
      PRIM diff = subtractWithOverflowCheck(a, b);
      Integer apDiff = apA - apB;
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
  }

  void testRandomArithmetic()
  {
    smbase_loopi(100) {
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
