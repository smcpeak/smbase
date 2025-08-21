// iter-and-end-test.cc
// Tests for `iter-and-end` module.

#include "smbase/iter-and-end.h"       // module under test

#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/gdvalue.h"            // needed for TEST_CASE_EXPRS
#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, TEST_CASE_EXPRS
#include "smbase/xassert.h"            // xassert

#include <functional>                  // std::{less, greater}
#include <list>                        // std::list
#include <vector>                      // std::vector

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// ---------------------------- IterAndEnd -----------------------------
void test_read()
{
  std::vector<int> v{1,2,3};

  int expect=1;
  IterAndEnd<std::vector<int>> ita(v.begin(), v.end());
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);
    ++expect;
    ++ita;
  }
  EXPECT_EQ(expect, 4);
}


void test_write()
{
  std::vector<int> v{1,2,3};

  int expect=1;
  IterAndEnd<std::vector<int>> ita(v.begin(), v.end());
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);
    *ita += 10;
    ++expect;
    ++ita;
  }
  EXPECT_EQ(expect, 4);

  expect = 11;
  for (int i : iterAndEnd(v)) {
    EXPECT_EQ(i, expect);
    ++expect;
  }
  EXPECT_EQ(expect, 14);
}


void test_copy_and_assign()
{
  std::vector<int> v{10,20,30};

  IterAndEnd<std::vector<int>> orig(v.begin(), v.end());

  // Copy constructor.
  IterAndEnd<std::vector<int>> copy(orig);
  xassert(copy.m_iter == orig.m_iter);
  xassert(copy.m_end == orig.m_end);
  xassert(copy == orig);

  // Modify copy and check inequality
  ++copy;
  xassert(copy != orig);

  // Assignment operator
  IterAndEnd<std::vector<int>> assign(orig);
  xassert(assign == orig);
  xassert(assign != copy);
  assign = copy;
  xassert(assign != orig);
  xassert(assign == copy);
}


void test_begin_end_and_empty()
{
  std::vector<int> v{4,5,6};

  // Iterate without range syntax.
  {
    IterAndEnd<std::vector<int>> ita(v.begin(), v.end());

    int expect = 4;
    for (; !ita.empty(); ++ita) {
      EXPECT_EQ(*ita, expect);
      ++expect;
    }
    EXPECT_EQ(expect, 7);
  }

  // Iterate with range syntax.
  {
    IterAndEnd<std::vector<int>> ita(v.begin(), v.end());

    int expect = 4;
    for (int i : ita) {
      EXPECT_EQ(i, expect);
      ++expect;
    }
    EXPECT_EQ(expect, 7);
  }
}


void test_increment_operators()
{
  std::vector<int> v{7,8,9};
  IterAndEnd<std::vector<int>> ita(v.begin(), v.end());

  // Pre-increment
  EXPECT_EQ(*ita, 7);
  IterAndEnd<std::vector<int>> ita2 = ++ita;
  EXPECT_EQ(*ita, 8);
  EXPECT_EQ(*ita2, 8);

  // Post-increment
  IterAndEnd<std::vector<int>> ita3 = ita++;
  EXPECT_EQ(*ita3, 8);
  EXPECT_EQ(*ita, 9);
}


void test_iterAndEnd()
{
  std::list<int> lst{100, 200, 300};
  auto ita = iterAndEnd(lst); // deduced type IterAndEnd<std::list<int>>

  int expect = 100;
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);
    expect += 100;
    ++ita;
  }
  EXPECT_EQ(expect, 400);
}


// -------------------------- ConstIterAndEnd --------------------------
void testc_read()
{
  std::vector<int> v{1,2,3};

  int expect=1;
  ConstIterAndEnd<std::vector<int>> ita(v.begin(), v.end());
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);

    // Does not compile because the iterator is const.
    //*ita = 0;

    ++expect;
    ++ita;
  }
  EXPECT_EQ(expect, 4);
}


void testc_convertToConst()
{
  std::vector<int> v{1,2,3};

  // Start with non-const.
  IterAndEnd<std::vector<int>> itaNC(v.begin(), v.end());

  // Convert to const.
  int expect=1;
  ConstIterAndEnd<std::vector<int>> ita(itaNC);
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);
    ++expect;
    ++ita;
  }
  EXPECT_EQ(expect, 4);
}


void testc_copy_and_assign()
{
  std::vector<int> v{10,20,30};

  ConstIterAndEnd<std::vector<int>> orig(v.begin(), v.end());

  // Copy constructor.
  ConstIterAndEnd<std::vector<int>> copy(orig);
  xassert(copy.m_iter == orig.m_iter);
  xassert(copy.m_end == orig.m_end);
  xassert(copy == orig);

  // Modify copy and check inequality
  ++copy;
  xassert(copy != orig);

  // Assignment operator
  ConstIterAndEnd<std::vector<int>> assign(orig);
  xassert(assign == orig);
  xassert(assign != copy);
  assign = copy;
  xassert(assign != orig);
  xassert(assign == copy);
}


void testc_begin_end_and_empty()
{
  std::vector<int> v{4,5,6};

  // Iterate without range syntax.
  {
    ConstIterAndEnd<std::vector<int>> ita(v.begin(), v.end());

    int expect = 4;
    for (; !ita.empty(); ++ita) {
      EXPECT_EQ(*ita, expect);
      ++expect;
    }
    EXPECT_EQ(expect, 7);
  }

  // Iterate with range syntax.
  {
    ConstIterAndEnd<std::vector<int>> ita(v.begin(), v.end());

    int expect = 4;
    for (int i : ita) {
      EXPECT_EQ(i, expect);
      ++expect;
    }
    EXPECT_EQ(expect, 7);
  }
}


void testc_increment_operators()
{
  std::vector<int> v{7,8,9};
  ConstIterAndEnd<std::vector<int>> ita(v.begin(), v.end());

  // Pre-increment
  EXPECT_EQ(*ita, 7);
  ConstIterAndEnd<std::vector<int>> ita2 = ++ita;
  EXPECT_EQ(*ita, 8);
  EXPECT_EQ(*ita2, 8);

  // Post-increment
  ConstIterAndEnd<std::vector<int>> ita3 = ita++;
  EXPECT_EQ(*ita3, 8);
  EXPECT_EQ(*ita, 9);
}


void testc_iterAndEnd()
{
  std::list<int> lst{100, 200, 300};
  auto ita = constIterAndEnd(lst); // deduced type ConstIterAndEnd<std::list<int>>

  int expect = 100;
  while (!ita.empty()) {
    EXPECT_EQ(*ita, expect);
    expect += 100;
    ++ita;
  }
  EXPECT_EQ(expect, 400);
}


// ------------------------------- Other -------------------------------
void test_compareIterAndEnds()
{
  std::vector<int> a{};
  std::vector<int> b{1};
  std::vector<int> c{2};

  {
    std::less<int> isLessThan;

    {
      auto itea = iterAndEnd(a);
      auto iteb = iterAndEnd(b);
      auto itec = iterAndEnd(c);

      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, itea), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, iteb), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, iteb), +1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itea), -1);

      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itec), -1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itec, iteb), +1);
    }

    {
      auto itea = constIterAndEnd(a);
      auto iteb = constIterAndEnd(b);
      auto itec = constIterAndEnd(c);

      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, itea), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, iteb), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, iteb), +1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itea), -1);

      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itec), -1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itec, iteb), +1);
    }
  }

  {
    // Swap the order.
    std::greater<int> isLessThan;

    {
      auto itea = iterAndEnd(a);
      auto iteb = iterAndEnd(b);
      auto itec = iterAndEnd(c);

      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, itea), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, iteb), 0);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itea, iteb), +1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itea), -1);

      // Here is where the swapped order is relevant.
      EXPECT_EQ(compareIterAndEnds(isLessThan, iteb, itec), +1);
      EXPECT_EQ(compareIterAndEnds(isLessThan, itec, iteb), -1);
    }
  }
}


template <typename COMPARATOR>
void testOne_compareLexicographicallyIAE(
  COMPARATOR const &isLessThan,
  std::vector<int> const &a,
  std::vector<int> const &b,
  int expectRes,
  int expectPos)
{
  std::string_view comparatorName = GetTypeName<COMPARATOR>::name();
  TEST_CASE_EXPRS("testOne_compareLexicographicallyIAE",
    comparatorName, a, b);

  auto itea = constIterAndEnd(a);
  auto iteb = constIterAndEnd(b);

  int actual = compareLexicographicallyIAE(
                 isLessThan, itea /*INOUT*/, iteb /*INOUT*/);
  EXPECT_EQ(actual, expectRes);

  int actualPos = itea.m_iter - a.begin();
  EXPECT_EQ(actualPos, expectPos);
  xassert(iteb.m_iter - b.begin() == actualPos);
}


// Test symmetry and reflexivity.
template <typename COMPARATOR>
void testOneSym_compareLexicographcallyIAE(
  COMPARATOR const &isLessThan,
  std::vector<int> const &a,
  std::vector<int> const &b,
  int expectRes,
  int expectPos)
{
  testOne_compareLexicographicallyIAE(isLessThan,
    a, b, expectRes, expectPos);
  testOne_compareLexicographicallyIAE(isLessThan,
    b, a, -expectRes, expectPos);

  testOne_compareLexicographicallyIAE(isLessThan,
    a, a, 0, a.size());
  testOne_compareLexicographicallyIAE(isLessThan,
    b, b, 0, b.size());
}


void test_compareLexicographicallyIAE()
{
  std::less<int> isLessThan;

  testOneSym_compareLexicographcallyIAE(isLessThan,
    {},
    {},
    0,
    0);

  testOneSym_compareLexicographcallyIAE(isLessThan,
    {1},
    {},
    -1,
    0);

  testOneSym_compareLexicographcallyIAE(isLessThan,
    {1},
    {2},
    -1,
    0);

  testOneSym_compareLexicographcallyIAE(isLessThan,
    {2, 1},
    {2},
    -1,
    1);

  testOneSym_compareLexicographcallyIAE(isLessThan,
    {1,    3},
    {1, 2, 3},
    +1,
    1);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_iter_and_end()
{
  test_read();
  test_write();
  test_copy_and_assign();
  test_begin_end_and_empty();
  test_increment_operators();
  test_iterAndEnd();

  testc_read();
  // no write
  testc_convertToConst();
  testc_copy_and_assign();
  testc_begin_end_and_empty();
  testc_increment_operators();
  testc_iterAndEnd();

  test_compareIterAndEnds();
  test_compareLexicographicallyIAE();
}


// EOF
