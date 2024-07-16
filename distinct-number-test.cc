// distinct-number-test.cc
// Tests for `distinct-number`.

#include "distinct-number-ops.h"       // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ, VPVAL
#include "xassert.h"                   // xassert

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class TestTag1;
typedef DistinctNumber<TestTag1, int> DI_int;

void acceptDI(DI_int)
{}


void testZero()
{
  DI_int n;
  VPVAL(n);
  xassert(n.isZero());
  xassert(!n.isNotZero());
  acceptDI(n);

  int underZero(0);
  xassert(n == underZero);
  xassert(underZero == n);

  xassert(n.get() == 0);

  // Allowed due to implicit conversion.
  xassert(n == 0);

  // Not allowed.
  #if ERRNUM == 1
    n = underZero;
  #endif

  #if ERRNUM == 2
    acceptDI(underZero);
  #endif

  #if ERRNUM == 3
    acceptDI(0);
  #endif

  #if ERRNUM == 4
    acceptDI(1);
  #endif
}


void testArith()
{
  EXPECT_EQ(DI_int(3), DI_int(1) + DI_int(2));

  DI_int n(5);
  n *= DI_int(7);
  EXPECT_EQ(n, DI_int(35));
}


void testComparison()
{
  DI_int n(2);

  // The implicit conversion also allows comparisons with the underlying
  // integer.
  xassert(n < 3);
  xassert(n == 2);
  xassert(n > 1);
}


void testIncrement()
{
  DI_int n(0);
  ++n;
  xassert(n == 1);

  xassert(++n == 2);
  xassert(n == 2);

  xassert(n++ == 2);
  xassert(n == 3);
}


void testDecrement()
{
  DI_int n(0);
  --n;
  xassert(n == -1);

  xassert(--n == -2);
  xassert(n == -2);

  xassert(n-- == -2);
  xassert(n == -3);
}


void testStringb()
{
  EXPECT_EQ("123", stringb(DI_int(123)));
}


void testGDValue()
{
  DI_int n(3);
  EXPECT_EQ(toGDValue(n), GDValue(3));
}


class TestTag2;
typedef DistinctNumber<TestTag2, int> DI2_int;

void testDI2()
{
  DI_int n1(3);
  DI2_int n2(3);

  // We can compare them due to the implicit conversion.
  xassert(n1 == n2);

  // But cannot assign them.
  #if ERRNUM == 5
    n1 = n2;
  #endif

  #if ERRNUM == 6
    acceptDI(n2);
  #endif
}


void testIter()
{
  int s = 0;

  // Here, the initial value is implicitly zero.  We cannot write the
  // usual "n = 0" since it would have to be "n = DI_int(0)".
  for (DI_int n; n < 5; ++n) {
    s += n;
  }

  xassert(s == 10);

  s = 0;

  // This also works.
  for (DI_int n(0); n < 5; ++n) {
    s += n;
  }

  xassert(s == 10);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_distinct_number()
{
  testZero();
  testArith();
  testComparison();
  testIncrement();
  testDecrement();
  testStringb();
  testGDValue();
  testDI2();
  testIter();
}


// EOF
