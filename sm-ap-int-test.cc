// sm-ap-int-test.cc
// Tests for sm-ap-int.h.

#include "sm-ap-int.h"                 // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // VPVAL

// This file is in the public domain.

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


bool verbose = true;


// Like in `sm-ap-uint-test.cc`, abstract the word size so I can with
// test several easily.
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

  void testAll()
  {
    testSimple();
  }

};


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_ap_int()
{
  APIntegerTest<uint8_t>().testAll();
  APIntegerTest<uint16_t>().testAll();
  APIntegerTest<uint32_t>().testAll();
}


// EOF
