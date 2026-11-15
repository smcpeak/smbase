// ordered-flat-map-test.cc
// Tests for `ordered-flat-map` module.

// This file is in the public domain.

#include "smbase/ordered-flat-map.h"   // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <string>                      // std::string

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
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_flat_map()
{
  test_basics();
}


// EOF
