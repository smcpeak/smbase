// set-util-test.cc
// Tests for `set-util`.

#include "set-util.h"                  // module under test

#include "exc.h"                       // smbase::XAssert
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "stringb.h"                   // stringb
#include "vector-util.h"               // operator<<(vector)
#include "xassert.h"                   // xassert

#include <cstdlib>                     // std::atoi
#include <set>                         // std::set

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testSetInsert()
{
  std::set<int> s;

  bool b = setInsert(s, 1);
  xassert(b);

  b = setInsert(s, 1);
  xassert(!b);

  EXPECT_EQ(stringb(s), "{1}");
}


void testSetInsertUnique()
{
  std::set<int> s;

  setInsertUnique(s, 1);
  EXPECT_EQ(stringb(s), "{1}");

  bool ok = true;
  try {
    setInsertUnique(s, 1);
    ok = false;
  }
  catch (XAssert &x) {}
  xassert(ok);

  EXPECT_EQ(stringb(s), "{1}");
}


void testSetInsertAll()
{
  std::set<int> s;
  setInsertAll(s, std::set<int>{1,2,3});
  EXPECT_EQ(stringb(s), "{1, 2, 3}");
}


void testIsSubsetOf()
{
  std::set<int> s0;
  xassert(isSubsetOf(s0, s0));

  std::set<int> s1{1};
  xassert(isSubsetOf(s0, s1));
  xassert(isSubsetOf(s1, s1));
  xassert(!isSubsetOf(s1, s0));

  std::set<int> s2{1,2};
  xassert(isSubsetOf(s0, s2));
  xassert(isSubsetOf(s1, s2));
  xassert(!isSubsetOf(s2, s1));
}


void testIsSubsetOf_getExtra()
{
  std::set<int> s1{1};
  std::set<int> s2{1,2};

  int extra = 0;
  xassert(isSubsetOf_getExtra(extra /*OUT*/, s1, s2));
  xassert(extra == 0);

  xassert(!isSubsetOf_getExtra(extra /*OUT*/, s2, s1));
  xassert(extra == 2);
}


void testSetMapElements()
{
  std::set<char const *> strings{"1", "2", "3"};

  std::set<int> numbers = setMapElements<int>(strings,
    [](char const *s) -> int {
      return std::atoi(s);
    });

  EXPECT_EQ(stringb(numbers), "{1, 2, 3}");
}


void testSetToVector()
{
  std::vector<int> v = setToVector(std::set<int>{1,2,3});
  EXPECT_EQ(stringb(v), "[1 2 3]");
}


void testOstreamInsert()
{
  std::set<int> s;
  EXPECT_EQ(stringb(s), "{}");

  s.insert(1);
  EXPECT_EQ(stringb(s), "{1}");

  s.insert(2);
  EXPECT_EQ(stringb(s), "{1, 2}");
}


void testSetWriter()
{
  std::set<int> s{1,2};
  auto printElement = [](std::ostream &os, int i) -> void {
    os << "(" << i << ")";
  };
  EXPECT_EQ(stringb(setWriter(s, printElement)), "{(1), (2)}");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_set_util()
{
  testSetInsert();
  testSetInsertUnique();
  testSetInsertAll();
  testIsSubsetOf();
  testIsSubsetOf_getExtra();
  testSetMapElements();
  testSetToVector();
  testOstreamInsert();
  testSetWriter();
}


// EOF
