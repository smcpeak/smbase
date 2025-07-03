// vector-util-test.cc
// Test code for vector-util.h.

#include "vector-util.h"               // module under test

#include "optional-util.h"             // operator<<(std::optional)
#include "sm-test.h"                   // EXPECT_EQ
#include "string-util.h"               // doubleQuote

#include <string>                      // std::string
#include <optional>                    // std::nullopt

using std::string;


static void testVecAccumulateWith()
{
  std::vector<string> v;
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "");

  v.push_back("a");
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "a");

  v.push_back("b");
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "a-b");
}


// Test 'vecEraseAll', 'vecToElementSet', and `vecFindIndex`.
static void testVecEraseAll()
{
  std::vector<int> v{1,2,3,2,1};

  vecEraseAll(v, 4);
  xassert((v == std::vector<int>{1,2,3,2,1}));
  xassert((vecToElementSet(v) == std::set<int>{1,2,3}));

  // Also test 'vecFindIndex'.
  xassert(*vecFindIndex(v, 1) == 0);
  xassert(*vecFindIndex(v, 2) == 1);
  xassert(*vecFindIndex(v, 3) == 2);
  xassert(vecFindIndex(v, 4) == std::nullopt);

  vecEraseAll(v, 2);
  xassert((v == std::vector<int>{1,3,1}));
  xassert((vecToElementSet(v) == std::set<int>{1,3}));
  xassert(*vecFindIndex(v, 3) == 1);

  vecEraseAll(v, 3);
  xassert((v == std::vector<int>{1,1}));
  xassert((vecToElementSet(v) == std::set<int>{1}));

  vecEraseAll(v, 1);
  xassert((v == std::vector<int>{}));
  xassert((vecToElementSet(v) == std::set<int>{}));
  xassert(vecFindIndex(v, 1) == std::nullopt);
}


static void testVecMapElements()
{
  std::vector<string> src {"a", "b"};
  std::vector<string> dest(vecMapElements<string>(src,
    [](string const &s) { return doubleQuote(s); }));
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));

  // I do not like that I have to specify '<string>' here, but I do not
  // know how to avoid it.
  dest = vecMapElements<string>(src, doubleQuote_string);
  EXPECT_EQ(dest, (std::vector<string>{"\"a\"", "\"b\""}));
}


static void testVecConvertElements()
{
  std::vector<string> src {"a", "b", "c"};

  // Convert smbase 'string' to 'std::string'.
  std::vector<std::string> dest(vecConvertElements<std::string>(src));
  EXPECT_EQ(dest, (std::vector<std::string>{"a", "b", "c"}));
}


static void testVecCommonPrefixLength()
{
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v12{1,2};
  std::vector<int> v2{2};

  EXPECT_EQ(vecCommonPrefixLength(v0, v0), 0);
  EXPECT_EQ(vecCommonPrefixLength(v0, v1), 0);
  EXPECT_EQ(vecCommonPrefixLength(v1, v1), 1);
  EXPECT_EQ(vecCommonPrefixLength(v1, v12), 1);
  EXPECT_EQ(vecCommonPrefixLength(v12, v12), 2);
  EXPECT_EQ(vecCommonPrefixLength(v1, v2), 0);
}


static void testVecFindIndex()
{
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v12{1,2};
  std::vector<int> v2{2};

  EXPECT_EQ(vecFindIndex(v0, 0), std::nullopt);
  EXPECT_EQ(vecFindIndex(v1, 0), std::nullopt);
  EXPECT_EQ(vecFindIndex(v1, 1), std::optional(0));
  EXPECT_EQ(vecFindIndex(v12, 1), std::optional(0));
  EXPECT_EQ(vecFindIndex(v12, 2), std::optional(1));
  EXPECT_EQ(vecFindIndex(v12, 3), std::nullopt);
}


void test_vector_util()
{
  testVecAccumulateWith();
  testVecEraseAll();
  testVecMapElements();
  testVecConvertElements();
  testVecCommonPrefixLength();
  testVecFindIndex();
}


// EOF
