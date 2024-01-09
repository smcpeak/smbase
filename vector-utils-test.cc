// vector-utils-test.cc
// Test code for vector-utils.h.

#include "vector-utils.h"              // module under test

#include "sm-test.h"                   // EXPECT_EQ
#include "str.h"                       // string
#include "strutil.h"                   // quoted

#include <string>                      // std::string


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

  // Also test 'vec_find_index'.
  xassert(vec_find_index(v, 1) == 0);
  xassert(vec_find_index(v, 2) == 1);
  xassert(vec_find_index(v, 3) == 2);
  xassert(vec_find_index(v, 4) == -1);

  vec_erase(v, 2);
  xassert((v == std::vector<int>{1,3,1}));
  xassert((vec_element_set(v) == std::set<int>{1,3}));
  xassert(vec_find_index(v, 3) == 1);

  vec_erase(v, 3);
  xassert((v == std::vector<int>{1,1}));
  xassert((vec_element_set(v) == std::set<int>{1}));

  vec_erase(v, 1);
  xassert((v == std::vector<int>{}));
  xassert((vec_element_set(v) == std::set<int>{}));
  xassert(vec_find_index(v, 1) == -1);
}


static void testMapElements()
{
  std::vector<string> src {"a", "b"};
  std::vector<string> dest(mapElements<string>(src, [](string const &s) { return quoted(s); }));
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));

  // I do not like that I have to specify '<string>' here, but I do not
  // know how to avoid it.
  dest = mapElements<string>(src, quoted);
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));
}


static void testConvertElements()
{
  std::vector<string> src {"a", "b", "c"};

  // Convert smbase 'string' to 'std::string'.
  std::vector<std::string> dest(convertElements<std::string>(src));
  EXPECT_EQ(dest, (std::vector<std::string>{"a", "b", "c"}));
}


void test_vector_utils()
{
  testAccumulateWith();
  testVecErase();
  testMapElements();
  testConvertElements();

  cout << "test_vector_utils passed\n";
}


// EOF
