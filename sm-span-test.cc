// sm-span-test.cc
// Tests for `sm-span`.

#include "sm-span.h"                   // module under test

#include "smbase/gdvalue.h"            // gdv::toGDValue
#include "smbase/gdvalue-span.h"       // gdv::toGDValue(Span)
#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, TABLESIZE
#include "smbase/sm-test.h"            // EXPECT_EQ[_GDV], TEST_CASE

#include <algorithm>                   // std::sort
#include <vector>                      // std::vector

using namespace gdv;
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


void test_sort()
{
  int arr[5] = { 10, 4, 19, 25, 2 };
  Span<int> sp(arr);

  // For this to work, `Span::iterator` has to not only satisfy the
  // requirements of a random access iterator, but also advertise itself
  // as such with its `iterator_category`.  The latter is unfortunate
  // since it requires #including <iterator>, which is a large header.
  std::sort(sp.begin(), sp.end());

  EXPECT_EQ_GDV(sp, (std::vector<int>{ 2, 4, 10, 19, 25 }));
}


void test_iterator()
{
  int arr[] = {10, 20, 30, 40, 50};
  Span<int> s(arr);

  auto it = s.begin();
  EXPECT_EQ(*it, 10);

  // Test ++ and --
  ++it; EXPECT_EQ(*it, 20);
  it++; EXPECT_EQ(*it, 30);
  --it; EXPECT_EQ(*it, 20);
  it--; EXPECT_EQ(*it, 10);

  // Test += and -=
  it += 2; EXPECT_EQ(*it, 30);
  it -= 1; EXPECT_EQ(*it, 20);

  // Test + and -
  auto it2 = it + 3; EXPECT_EQ(*it2, 50);
  auto it3 = 2 + s.begin(); EXPECT_EQ(*it3, 30);
  auto it4 = it2 - 2; EXPECT_EQ(*it4, 30);

  // Test difference
  EXPECT_EQ((s.end() - s.begin()), 5);
  EXPECT_EQ((it2 - it), 3);

  // Test []
  EXPECT_EQ(s.begin()[0], 10);
  EXPECT_EQ(s.begin()[4], 50);

  // Test comparisons
  xassert(s.begin() < s.end());
  xassert(s.begin() <= s.begin());
  xassert(s.end() > s.begin());
  xassert(!(s.begin() > s.end()));
  xassert(s.begin() != s.end());
  xassert(!(s.begin() == s.end()));

  // Test traversal via loop
  int idx = 0;
  for (auto it = s.begin(); it != s.end(); ++it) {
    EXPECT_EQ(*it, arr[idx++]);
  }
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
  test_sort();
  test_iterator();
}


// EOF
