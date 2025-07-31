// sm-span-test.cc
// Tests for `sm-span`.

#include "sm-span.h"                   // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, TABLESIZE
#include "smbase/sm-test.h"            // EXPECT_EQ, TEST_CASE

#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Verify that the values in `view` start at `startValue` and increase by 1
// with each successive element.
void checkAscending(Span<int const> view, int startValue, int size)
{
  EXPECT_EQ(view.size(), size);
  EXPECT_EQ(view.empty(), size==0);

  int expect = startValue;
  for (int actual : view) {
    EXPECT_EQ(actual, expect);
    ++expect;
  }
}


// Create a read-only view onto an array via pointer+size.
void test_readOnlyArray()
{
  int arr[3] = {1,2,3};
  Span<int const> av(arr, TABLESIZE(arr));

  EXPECT_EQ(av.empty(), false);
  EXPECT_EQ(av.size(), 3);
  EXPECT_EQ(av[0], 1);
  EXPECT_EQ(av[1], 2);
  EXPECT_EQ(av[2], 3);

  checkAscending(av, 1, 3);
}


// Create a read/write view onto an array directly.
void test_readWriteArray()
{
  int arr[3] = {1,2,3};
  Span<int> av(arr);

  checkAscending(av, 1, 3);

  av[0] = 4;
  av[1] = 5;
  av[2] = 6;

  checkAscending(av, 4, 3);
}


// Create a read/write view onto a vector.
void test_readWriteVector()
{
  std::vector<int> vec = {1,2,3};
  Span<int> av(vec);

  checkAscending(av, 1, 3);

  av[0] = 4;
  av[1] = 5;
  av[2] = 6;

  checkAscending(av, 4, 3);
}


// Create a read/write view onto a vector.
void test_readOnlyVector()
{
  std::vector<int> const vec = {1,2,3};
  Span<int const> av(vec);

  checkAscending(av, 1, 3);
}


void test_subspan()
{
  TEST_CASE("subspan");

  int arr[10] = {1,2,3,4,5,6,7,8,9,10};
  Span<int> whole(arr);
  checkAscending(whole, 1, 10);

  Span<int> mid5 = whole.subspan(3, 5);
  checkAscending(mid5, 4, 5);

  checkAscending(mid5.subspan(3), 7, 2);

  checkAscending(mid5.subspan(3, 0), 7, 0);
}


void test_arrayOfConst()
{
  int const arr[] = {1,2,3};
  Span<int const> av(arr);

  checkAscending(av, 1, 3);

  // Note: It is not possible to have `std::vector` of a const type.
}


void test_defaultCtor()
{
  Span<int> sp;
  EXPECT_EQ(sp.empty(), true);
  EXPECT_EQ(sp.size(), 0);
  xassert(sp.data() == nullptr);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_span()
{
  test_readOnlyArray();
  test_readWriteArray();
  test_readWriteVector();
  test_readOnlyVector();
  test_subspan();
  test_arrayOfConst();
  test_defaultCtor();
}


// EOF
