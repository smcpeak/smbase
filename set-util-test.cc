// set-util-test.cc
// Tests for `set-util`.

#include "set-util.h"                  // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "stringb.h"                   // stringb
#include "xassert.h"                   // xassert

#include <cstdlib>                     // std::atoi
#include <set>                         // std::set


OPEN_ANONYMOUS_NAMESPACE


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


void testSetMapElements()
{
  std::set<char const *> strings{"1", "2", "3"};

  std::set<int> numbers = setMapElements<int>(strings,
    [](char const *s) -> int {
      return std::atoi(s);
    });

  EXPECT_EQ(stringb(numbers), "{1, 2, 3}");
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


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_set_util()
{
  testIsSubsetOf();
  testSetMapElements();
  testOstreamInsert();
}


// EOF
