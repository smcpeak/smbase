// container-util-test.cc
// Tests for `container-util` module.

#include "smbase/container-util.h"     // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <map>
#include <set>

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_contains()
{
  {
    std::set<int> s{1,2,3};
    xassert(!contains(s, 0));
    xassert(contains(s, 2));
    xassert(!contains(s, 4));
  }

  {
    std::map<int, int> m{ {1,2}, {3,4} };
    xassert(contains(m, 1));
    xassert(!contains(m, 2));
    xassert(contains(m, 3));
  }
}


void test_insertUnique()
{
  {
    std::set<int> s;
    insertUnique(s, 1);
    EXPECT_EXN(insertUnique(s, 1), XAssert);
    insertUnique(s, 2);
    EXPECT_EXN(insertUnique(s, 2), XAssert);
    xassert(s == (std::set<int>{1,2}));
  }

  {
    std::map<int, int> s;
    insertUnique(s, std::make_pair(1,2));
    EXPECT_EXN(insertUnique(s, std::make_pair(1,2)), XAssert);
    insertUnique(s, std::make_pair(3,4));
    EXPECT_EXN(insertUnique(s, std::make_pair(3,4)), XAssert);
    xassert(s == (std::map<int, int>{ {1,2}, {3,4 } }));
  }
}


void test_reverseIterRange()
{
  std::set<int> s{1,2,3};

  int expect = 3;
  for (int n : reverseIterRange(s)) {
    EXPECT_EQ(n, expect--);
  }

  std::set<int> const &cs = s;

  expect = 3;
  for (int n : reverseIterRange(cs)) {
    EXPECT_EQ(n, expect--);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_container_util()
{
  test_contains();
  test_insertUnique();
  test_reverseIterRange();
}


// EOF
