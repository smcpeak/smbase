// std-functional-fwd-test.cc
// Tests for `std-functional-fwd` module.

#include "smbase/std-functional-fwd.h" // module under test

namespace {
  std::reference_wrapper<int> *prw1;
  stdfwd::reference_wrapper<int> *prw2;
}

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <functional>                  // std::reference_wrapper

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  int a = 3;
  std::reference_wrapper<int> rw(a);

  prw1 = &rw;
  prw2 = &rw;

  EXPECT_EQ(*prw1, 3);
  EXPECT_EQ(*prw2, 3);

  (*prw1).get() = 5;
  EXPECT_EQ(*prw1, 5);
  EXPECT_EQ(*prw2, 5);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_std_functional_fwd()
{
  test_basics();
}


// EOF
