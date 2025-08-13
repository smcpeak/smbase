// iter-and-end-test.cc
// Tests for `iter-and-end` module.

#include "smbase/iter-and-end.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/xassert.h"            // xassert

#include <vector>                      // std::vector
#include <list>                        // std::list

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


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
}


// EOF
