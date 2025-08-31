// either-test.cc
// Tests for `either` module.

#include "smbase/either.h"             // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  Either<int, float> i(3);
  EXPECT_EQ(i.isLeft(), true);
  EXPECT_EQ(i.isRight(), false);
  EXPECT_EQ(i.left(), 3);
  EXPECT_EXN_SUBSTR(i.right(),
    XAssert, "isRight()");

  Either<int, float> f(4.5f);
  EXPECT_EQ(f.isLeft(), false);
  EXPECT_EQ(f.isRight(), true);
  EXPECT_EXN_SUBSTR(f.left(),
    XAssert, "isLeft()");
  EXPECT_EQ(f.right(), 4.5f);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_either()
{
  test_basics();
}


// EOF
