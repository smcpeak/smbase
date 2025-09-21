// sm-macros-test.cc
// Tests for `sm-macros` module.

#include "smbase/sm-macros.h"          // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <utility>                     // std::swap

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


struct Data {
  int x;
  int y;

  void swapWith(Data &obj)
  {
    using std::swap;

    SWAP_MEMB(x);
    SWAP_MEMB(y);
  }
};


void test_SWAP_MEMB()
{
  Data d1{1,2};
  Data d2{3,4};

  EXPECT_EQ(d1.x, 1);
  EXPECT_EQ(d1.y, 2);
  EXPECT_EQ(d2.x, 3);
  EXPECT_EQ(d2.y, 4);

  d1.swapWith(d2);

  EXPECT_EQ(d2.x, 1);
  EXPECT_EQ(d2.y, 2);
  EXPECT_EQ(d1.x, 3);
  EXPECT_EQ(d1.y, 4);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_macros()
{
  test_SWAP_MEMB();
}


// EOF
