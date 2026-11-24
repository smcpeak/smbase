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


class HasInteger {
public:
  int m_i;
  HasInteger(int i) : m_i(i) {}
};

// Test using the explicit construction choices to deal with a case
// where overload resolution would pick the wrong one.
void test_constructionAmbiguity()
{
  using ResOrError = Either<HasInteger, int>;

  // Leave the choice implicit: it picks the right-hand (possibly
  // wrong!) alternative.
  {
    ResOrError roe(3);
    EXPECT_EQ(roe.isRight(), true);
  }

  // Explicitly pick left-hand.
  {
    ResOrError roe(ResOrError::makeLeft(3));
    EXPECT_EQ(roe.isLeft(), true);
  }

  // Explicitly pick right-hand.
  {
    ResOrError roe(ResOrError::makeRight(3));
    EXPECT_EQ(roe.isRight(), true);
  }
}


// Test when using exactly the same types on both sides.
void test_sameTypes()
{
  using IOI = Either<int, int>;

  // Doesn't compile, as expected.
  //IOI(3);

  EXPECT_EQ(IOI::makeLeft(3).isLeft(), true);
  EXPECT_EQ(IOI::makeRight(3).isRight(), true);

  // Still permits implicit conversions.
  EXPECT_EQ(IOI::makeLeft(3.1).isLeft(), true);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_either()
{
  test_basics();
  test_constructionAmbiguity();
  test_sameTypes();
}


// EOF
