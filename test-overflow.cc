// test-overflow.cc
// Unit tests for 'overflow' module.

#include "overflow.h"                  // this module

#include "macros.h"                    // PVAL
#include "types.h"                     // int64_t, uint64_t

#include <assert.h>                    // assert
#include <iostream>                    // std::cout

using namespace std;


// Add, and expect success.
template <class NUM>
void testOneAdd(NUM a, NUM b, NUM expect)
{
  NUM actual = addWithOverflowCheck(a, b);
  assert(actual == expect);
}


// Add, and expect overflow.
template <class NUM>
void testOneAddOv(NUM a, NUM b, int verbose = 1)
{
  try {
    addWithOverflowCheck(a, b);
    PVAL(typeid(a).name());
    cout << "a: ";
    insertAsDigits(cout, a) << endl;
    cout << "b: ";
    insertAsDigits(cout, b) << endl;
    assert(!"testOneAddOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x.msg << endl;
    }
  }
}


// Multiply, and expect success.
template <class NUM>
void testOneMultiply(NUM a, NUM b, NUM expect)
{
  NUM actual = multiplyWithOverflowCheck(a, b);
  assert(actual == expect);
}


// Multiply, and expect overflow.
template <class NUM>
void testOneMultiplyOv(NUM a, NUM b, int verbose = 1)
{
  try {
    multiplyWithOverflowCheck(a, b);
    PVAL(typeid(a).name());
    cout << "a: ";
    insertAsDigits(cout, a) << endl;
    cout << "b: ";
    insertAsDigits(cout, b) << endl;
    assert(!"testOneMultiplyOv: that should have failed");
  }
  catch (XOverflow &x) {
    if (verbose) {
      cout << "As expected: " << x.msg << endl;
    }
  }
}


// Test what happens with 'a+b' using int64_t, which must be able
// to represent all the possible values.
template <class SMALL_NUM>
void testOneAddSmallUsingInt64(SMALL_NUM a, SMALL_NUM b, int verbose = 1)
{
  int64_t largeA(a);
  int64_t largeB(b);
  int64_t result(largeA + largeB);

  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  if (minValue <= result && result <= maxValue) {
    // Should not overflow.
    SMALL_NUM actual = addWithOverflowCheck(a, b);

    // Check for correctness using the larger type.
    int64_t largeActual(actual);
    assert(largeActual == result);
  }
  else {
    testOneAddOv(a, b, verbose);
  }
}


// Same but for multiplication.
template <class SMALL_NUM>
void testOneMultiplySmallUsingInt64(SMALL_NUM a, SMALL_NUM b, int verbose = 1)
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
    assert(largeActual == result);
  }
  else {
    testOneMultiplyOv(a, b, verbose);
  }
}


// Exhaustively check all pairs of SMALL_NUM.
template <class SMALL_NUM>
void testAddMultiplyAllSmallUsingInt64()
{
  int64_t minValue(numeric_limits<SMALL_NUM>::min());
  int64_t maxValue(numeric_limits<SMALL_NUM>::max());

  for (int64_t a = minValue; a <= maxValue; a++) {
    for (int64_t b = minValue; b <= maxValue; b++) {
      testOneAddSmallUsingInt64((SMALL_NUM)a, (SMALL_NUM)b, 0 /*verbose*/);
      testOneMultiplySmallUsingInt64((SMALL_NUM)a, (SMALL_NUM)b, 0 /*verbose*/);
    }
  }
}


static void testAddAndMultiply()
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

  testOneMultiplySmallUsingInt64<int8_t>(2, 3);
  testOneMultiplySmallUsingInt64<int8_t>(100, 100);
  testOneMultiplySmallUsingInt64<int8_t>(-1, 1);
  testOneMultiplyOv<int8_t>(-1, -128);

  cout << "int8_t exhaustive" << endl;
  testAddMultiplyAllSmallUsingInt64<int8_t>();

  cout << "uint8_t exhaustive" << endl;
  testAddMultiplyAllSmallUsingInt64<uint8_t>();


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


int test_overflow()
{
  testAddAndMultiply();

  cout << "test-overflow: PASSED" << endl;
  return 0;
}


// EOF
