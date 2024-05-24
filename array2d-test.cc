// array2d-test.cc
// test array2d.h

#include "array2d.h"         // module to test
#include "sm-test.h"         // USUAL_MAIN

#include "sm-iostream.h"     // cout

#include <stdlib.h>          // exit


// Called from unit-tests.cc.
void test_array2d()
{
  Array2D<int> zero(3,5);
  zero.setAll(0);

  xassert(zero.getRows() == 3);
  xassert(zero.getColumns() == 5);

  // Test setting via 'eltRef'.
  Array2D<int> arr1(3,5);
  FOREACH_ARRAY2D_COORD(arr1, i,j) {
    arr1.eltRef(i,j) = i*10 + j;
  }

  xassert(arr1 != zero);
  xassert(!( arr1 == zero ));

  // Test setting via 'setElt'.
  Array2D<int> arr2(3,5);
  FOREACH_ARRAY2D_COORD(arr2, i,j) {
    arr2.setElt(i,j, i*10 + j);
  }

  xassert(arr1 == arr2);

  // Test getting.
  xassert(arr1.getElt(1,1) == 11);
  xassert(arr1.eltRef(1,1) == 11);
  xassert(arr1.eltRefC(1,1) == 11);

  arr1.eltRef(1,1) = 55;

  xassert(arr1.getElt(1,1) != 11);
  xassert(arr1.getElt(1,1) == 55);

  // Nearby elements unchanged despite setting (1,1).
  xassert(arr1.getElt(2,1) == 21);
  xassert(arr1.getElt(0,1) == 1);
  xassert(arr1.getElt(1,2) == 12);
  xassert(arr1.getElt(1,0) == 10);

  xassert(arr1 != arr2);

  arr2.setElt(1,1, 55);

  xassert(arr1 == arr2);

  // Test copy ctor.
  {
    Array2D<int> arr3(arr1);
    xassert(arr1 == arr3);
    xassert(arr2 == arr3);

    arr3.setElt(1,1, 555);
    xassert(arr1 != arr3);
  }

  // Test operator=.
  {
    Array2D<int> arr3(0,0);
    arr3 = arr2;

    xassert(arr1 == arr3);
    xassert(arr2 == arr3);

    arr3.setElt(1,1, 555);
    xassert(arr1 != arr3);
  }

  // Test just-in-bounds access.
  arr1.eltRef(0,0) = 0;
  arr1.eltRef(2,0) = 0;
  arr1.eltRef(2,4) = 0;
  arr1.eltRef(0,4) = 0;

  // Test out of bounds access.
  #define SHOULD_FAIL(expr)                          \
    try {                                            \
      expr;                                          \
                                                     \
      cout << "should have failed: " #expr "\n";     \
      exit(4);                                       \
    }                                                \
    catch (...) {                                    \
      cout << "failed as expected: " #expr "\n";     \
    }

  SHOULD_FAIL( arr1.eltRef(-1,0) );
  SHOULD_FAIL( arr1.eltRef(0,-1) );

  SHOULD_FAIL( arr1.eltRef(3,0) );
  SHOULD_FAIL( arr1.eltRef(2,-1) );

  SHOULD_FAIL( arr1.eltRef(3,4) );
  SHOULD_FAIL( arr1.eltRef(2,5) );

  SHOULD_FAIL( arr1.eltRef(-1,4) );
  SHOULD_FAIL( arr1.eltRef(0,5) );

  cout << "array2d seems to work\n";
}


// EOF
