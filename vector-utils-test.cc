// vector-utils-test.cc
// Test code for vector-utils.h.

#include "vector-utils.h"              // module under test

#include "sm-test.h"                   // EXPECT_EQ
#include "str.h"                       // string


static void testAccumulateWith()
{
  std::vector<string> v;
  EXPECT_EQ(accumulateWith(v, string("-")), "");

  v.push_back("a");
  EXPECT_EQ(accumulateWith(v, string("-")), "a");

  v.push_back("b");
  EXPECT_EQ(accumulateWith(v, string("-")), "a-b");
}


// Test 'vec_erase' and 'vec_element_set'.
static void testVecErase()
{
  std::vector<int> v{1,2,3,2,1};

  vec_erase(v, 4);
  xassert((v == std::vector<int>{1,2,3,2,1}));
  xassert((vec_element_set(v) == std::set<int>{1,2,3}));

  vec_erase(v, 2);
  xassert((v == std::vector<int>{1,3,1}));
  xassert((vec_element_set(v) == std::set<int>{1,3}));

  vec_erase(v, 3);
  xassert((v == std::vector<int>{1,1}));
  xassert((vec_element_set(v) == std::set<int>{1}));

  vec_erase(v, 1);
  xassert((v == std::vector<int>{}));
  xassert((vec_element_set(v) == std::set<int>{}));
}


void test_vector_utils()
{
  testAccumulateWith();
  testVecErase();

  cout << "test_vector_utils passed\n";
}


// EOF
