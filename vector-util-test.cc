// vector-util-test.cc
// Test code for vector-util.h.

#include "smbase/optional-opll-iface.h"          // operator<<(std::optional)

#include "smbase/vector-util.h"                  // module under test

#include "smbase/optional-opll.h"                // operator<<(std::optional)
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/string-util.h"                  // doubleQuote

#include <string>                                // std::string
#include <optional>                              // std::nullopt


using std::string;


OPEN_ANONYMOUS_NAMESPACE


void test_vecAccumulateWith()
{
  std::vector<string> v;
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "");

  v.push_back("a");
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "a");

  v.push_back("b");
  EXPECT_EQ(vecAccumulateWith(v, string("-")), "a-b");
}


// Test 'vecEraseAll', 'vecToElementSet', and `vecFindIndex`.
void test_vecEraseAll()
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


void test_vecMapElements()
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


void test_vecConvertElements()
{
  std::vector<string> src {"a", "b", "c"};

  // Convert smbase 'string' to 'std::string'.
  std::vector<std::string> dest(vecConvertElements<std::string>(src));
  EXPECT_EQ(dest, (std::vector<std::string>{"a", "b", "c"}));
}


void test_vecCommonPrefixLength()
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


void test_vecFindIndex()
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


void test_vecAppendByMoving()
{
  std::vector<std::string> a{"a", "b", "c"};
  std::vector<std::string> b{"d", "e", "f"};

  vecAppendByMoving(a, std::move(b));

  EXPECT_EQ(stringb(a), R"(["a", "b", "c", "d", "e", "f"])");
  EXPECT_EQ(b.size(), 0);
}


void test_vecSum()
{
  EXPECT_EQ(vecSum(std::vector<int>{}), 0);
  EXPECT_EQ(vecSum(std::vector<int>{0}), 0);
  EXPECT_EQ(vecSum(std::vector<int>{1, 2, 3, 4, 5}), 15);
  EXPECT_EQ(vecSum(std::vector<float>{1.5, 2.5, 3.5}), 7.5);
}


void test_vecSumSlice()
{
  std::vector<int> v{0,1,2,3,4};
  EXPECT_EQ(vecSumSlice(v, 0, 0), 0);
  EXPECT_EQ(vecSumSlice(v, 5, 0), 0);
  EXPECT_EQ(vecSumSlice(v, 0, 3), 3);
  EXPECT_EQ(vecSumSlice(v, 3, 2), 7);
  EXPECT_EQ(vecSumSlice(v, 0, 5), 10);
}


void test_vecSlice()
{
  std::vector<int> v{0,1,2,3,4};

  xassert(vecSlice(v, 0, 0) == (std::vector<int>{}));
  xassert(vecSlice(v, 2, 2) == (std::vector<int>{2,3}));
  xassert(vecSlice(v, 0, 5) == (std::vector<int>{0,1,2,3,4}));

  xassert(vecSlice(v, 0) == (std::vector<int>{0,1,2,3,4}));
  xassert(vecSlice(v, 3) == (std::vector<int>{3,4}));
  xassert(vecSlice(v, 5) == (std::vector<int>{}));
}


void test_vecContains()
{
  std::vector<int> v{0,1,2,3,4};
  xassert(vecContains(v, 0));
  xassert(vecContains(v, 4));
  xassert(!vecContains(v, -1));
  xassert(!vecContains(v, 5));

  xassert(!vecContains(std::vector<int>{}, 0));
}


void test_vecEraseFirstN()
{
  std::vector<int> v{0,1,2,3,4};

  vecEraseFirstN(v, 0);
  xassert(v == (std::vector<int>{0,1,2,3,4}));

  vecEraseFirstN(v, 2);
  xassert(v == (std::vector<int>{2,3,4}));

  vecEraseFirstN(v, 3);
  xassert(v == (std::vector<int>{}));
}


void test_vecForAllElements()
{
  std::vector<int> v{0,2,4};

  EXPECT_TRUE(vecForAllElements(v,
    [](int i) -> bool {
      return i % 2 == 0;
    }));

  EXPECT_FALSE(vecForAllElements(v,
    [](int i) -> bool {
      return i < 3;
    }));
}


void test_vecArrayToCRefs()
{
  int arr[] = {1, 2, 3};
  std::vector<std::reference_wrapper<int const>> refs =
    vecArrayToCRefs(arr);

  int sum = 0;
  for (int const &n : refs) {
    sum += n;
  }
  EXPECT_EQ(sum, 6);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_vector_util()
{
  test_vecAccumulateWith();
  test_vecEraseAll();
  test_vecMapElements();
  test_vecConvertElements();
  test_vecCommonPrefixLength();
  test_vecFindIndex();
  test_vecAppendByMoving();
  test_vecSum();
  test_vecSumSlice();
  test_vecSlice();
  test_vecContains();
  test_vecEraseFirstN();
  test_vecForAllElements();
  test_vecArrayToCRefs();
}


// EOF
