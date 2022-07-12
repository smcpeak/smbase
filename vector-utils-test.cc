// vector-utils-test.cc
// Test code for vector-utils.h.

#include "vector-utils.h"              // module under test

#include "sm-test.h"                   // EXPECT_EQ
#include "str.h"                       // string


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
