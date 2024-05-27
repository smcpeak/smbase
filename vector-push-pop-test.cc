// vector-push-pop-test.cc
// Tests for `vector-push-pop.h`.

// This file is in the public domain.

#include "vector-push-pop.h"           // module under test

#include "sm-test.h"                   // EXPECT_EQ, verbose
#include "stringb.h"                   // stringb
#include "vector-utils.h"              // operator<< (vector)


// Called from unit-tests.cc.
void test_vector_push_pop()
{
  std::vector<int> vec;
  EXPECT_EQ(stringb(vec), "[]");
  VPVAL(vec);
}


// EOF
