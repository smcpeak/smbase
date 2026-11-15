// ordered-flat-map-test.cc
// Tests for `ordered-flat-map` module.

// This file is in the public domain.

#include "smbase/ordered-flat-map.h"   // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN_SUBSTR

#include <string>                      // std::string
#include <utility>                     // std::move

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  OrderedFlatMap<int, std::string> m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.valueRefAtKey(1) = "one";
  m.valueRefAtKey(3) = "three";
  m.valueRefAtKey(2) = "two";

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

  // Key-based lookup uses the intrinsic order.
  EXPECT_EQ(m.valueAtKey(1), "one");
  EXPECT_EQ(m.valueAtKey(2), "two");
  EXPECT_EQ(m.valueAtKey(3), "three");

  // Index-based lookup uses the extrinsic order.
  EXPECT_EQ(m.keyAtIndex(0), 1);
  EXPECT_EQ(m.valueAtIndex(0), "one");

  EXPECT_EQ(m.keyAtIndex(1), 3);
  EXPECT_EQ(m.valueAtIndex(1), "three");

  EXPECT_EQ(m.keyAtIndex(2), 2);
  EXPECT_EQ(m.valueAtIndex(2), "two");

  EXPECT_EQ(m.contains(0), false);
  EXPECT_EQ(m.contains(1), true);
  EXPECT_EQ(m.contains(2), true);
  EXPECT_EQ(m.contains(3), true);
  EXPECT_EQ(m.contains(4), false);
  m.selfCheck();

  // Copy ctor.
  {
    OrderedFlatMap<int, std::string> m2(m);
    EXPECT_EQ(m2.size(), 3);
    EXPECT_EQ(m2.valueAtKey(2), "two");
    m2.selfCheck();
  }

  // Invalid access.
  EXPECT_EXN(m.valueAtKey(4), XAssert);
  EXPECT_EXN(m.keyAtIndex(3), XAssert);
  EXPECT_EXN(m.valueAtIndex(3), XAssert);

  // Check default-constructed value.
  EXPECT_EQ(m.valueRefAtKey(4), "");

  // Move ctor.
  OrderedFlatMap<int, std::string> m2(std::move(m));
  EXPECT_EQ(m2.size(), 4);
  EXPECT_EQ(m2.valueAtKey(2), "two");
  m2.selfCheck();

  // The invariants of `m` should hold, even if its state is otherwise
  // indeterminate.
  m.selfCheck();
}


class CompareMod10 {
public:
  bool operator()(int a, int b) const
  {
    return (a % 10) < (b % 10);
  }
};


void test_differentCompare()
{
  OrderedFlatMap<int, std::string, CompareMod10> m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.valueRefAtKey(1) = "one";
  m.valueRefAtKey(3) = "three";
  m.valueRefAtKey(2) = "two";

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

  EXPECT_EQ(m.valueAtKey(1), "one");
  EXPECT_EQ(m.valueAtKey(2), "two");
  EXPECT_EQ(m.valueAtKey(3), "three");

  // mod 10 behavior
  EXPECT_EQ(m.valueAtKey(13), "three");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_flat_map()
{
  test_basics();
  test_differentCompare();
}


// EOF
