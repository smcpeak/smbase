// test-vector-utils.cc
// Test code for vector-utils.h.

#include "vector-utils.h"              // module under test

#include "str.h"                       // string
#include "test.h"                      // EXPECT_EQ


void test_vector_utils()
{
  std::vector<string> v;
  EXPECT_EQ(accumulateWith(v, string("-")), "");

  v.push_back("a");
  EXPECT_EQ(accumulateWith(v, string("-")), "a");

  v.push_back("b");
  EXPECT_EQ(accumulateWith(v, string("-")), "a-b");

  cout << "test_vector_utils passed\n";
}


// EOF
