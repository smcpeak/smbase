// vector-util-test.cc
// Test code for vector-util.h.

#include "vector-util.h"               // module under test

#include "optional-util.h"             // operator<<(std::optional)
#include "sm-test.h"                   // EXPECT_EQ
#include "string-util.h"               // doubleQuote

#include <string>                      // std::string
#include <optional>                    // std::nullopt

using std::string;


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
  std::vector<string> dest(mapElements<string>(src,
    [](string const &s) { return doubleQuote(s); }));
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));

  // I do not like that I have to specify '<string>' here, but I do not
  // know how to avoid it.
  dest = mapElements<string>(src, doubleQuote);
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));
}


static void testConvertElements()
{
  std::vector<string> src {"a", "b", "c"};

  // Convert smbase 'string' to 'std::string'.
  std::vector<std::string> dest(convertElements<std::string>(src));
  EXPECT_EQ(dest, (std::vector<std::string>{"a", "b", "c"}));
}


static void testCommonPrefixLength()
{
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v12{1,2};
  std::vector<int> v2{2};

  EXPECT_EQ(commonPrefixLength(v0, v0), 0);
  EXPECT_EQ(commonPrefixLength(v0, v1), 0);
  EXPECT_EQ(commonPrefixLength(v1, v1), 1);
  EXPECT_EQ(commonPrefixLength(v1, v12), 1);
  EXPECT_EQ(commonPrefixLength(v12, v12), 2);
  EXPECT_EQ(commonPrefixLength(v1, v2), 0);
}


static void testFirstIndexOf()
{
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v12{1,2};
  std::vector<int> v2{2};

  EXPECT_EQ(vectorFirstIndexOf(v0, 0), std::nullopt);
  EXPECT_EQ(vectorFirstIndexOf(v1, 0), std::nullopt);
  EXPECT_EQ(vectorFirstIndexOf(v1, 1), std::optional(0));
  EXPECT_EQ(vectorFirstIndexOf(v12, 1), std::optional(0));
  EXPECT_EQ(vectorFirstIndexOf(v12, 2), std::optional(1));
  EXPECT_EQ(vectorFirstIndexOf(v12, 3), std::nullopt);
}


void test_vector_util()
{
  testAccumulateWith();
  testVecErase();
  testMapElements();
  testConvertElements();
  testCommonPrefixLength();
  testFirstIndexOf();
}


// EOF
