// string-utils-test.cc
// Test code for string-utils.

#include "string-utils.h"              // module under test

#include "sm-test.h"                   // EXPECT_EQ

#include <iostream>                    // std::ostream


static void testSplitNonEmpty()
{
  struct Test {
    char const *m_input;
    std::vector<std::string> m_expect;
  }
  const tests[] = {
    {
      "",
      {},
    },
    {
      " ",
      {},
    },
    {
      "a",
      {"a"},
    },
    {
      "a  ",
      {"a"},
    },
    {
      "a bar c",
      {"a", "bar", "c"},
    },
    {
      "   a    b    c   ",
      {"a", "b", "c"},
    },
  };

  for (auto t : tests) {
    std::vector<std::string> actual = splitNonEmpty(t.m_input, ' ');
    EXPECT_EQ(actual, t.m_expect);
  }
}


static void testJoin()
{
  struct Test {
    std::vector<std::string> m_vec;
    char const *m_sep;
    char const *m_expect;
  }
  const tests[] = {
    {
      { "" },
      "",
      ""
    },
    {
      { "" },
      "x",
      ""
    },
    {
      { "a" },
      " ",
      "a"
    },
    {
      { "a", "b" },
      " ",
      "a b"
    },
    {
      { "a", "b" },
      "",
      "ab"
    },
  };

  for (auto t : tests) {
    std::string actual = join(t.m_vec, t.m_sep);
    EXPECT_EQ(actual, t.m_expect);
  }
}


static void testDoubleQuote()
{
  static struct Test {
    std::string m_input;
    char const *m_expect;
  }
  const tests[] = {
    { "", "\"\"" },
    { "x", "\"x\"" },
    { "quick brown foxes!", "\"quick brown foxes!\"" },
    { std::string("a\0b\0c", 5), "\"a\\000b\\000c\"" },
    { std::string("a\0001\0002", 5), "\"a\\0001\\0002\"" },
    { "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
      "\"\\001\\002\\003\\004\\005\\006\\a\\b\\t\\n\\v\\f\\r\\016\\017\"" },
    { "\036\037\040\041",
      "\"\\036\\037 !\"" },
    { "\176\177\200\201",
      "\"~\\177\\200\\201\"" },
    { "'\"?\\\a\b\f\n\r\t\v",
      "\"'\\\"?\\\\\\a\\b\\f\\n\\r\\t\\v\"" },
  };

  for (auto t : tests) {
    std::string actual = doubleQuote(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


static void testVectorToString()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;
  }
  const tests[] = {
    {
      {},
      "[]",
    },
    {
      { "a" },
      "[\"a\"]",
    },
    {
      { "" },
      "[\"\"]",
    },
    {
      { "a", "b", "c" },
      "[\"a\", \"b\", \"c\"]",
    },
    {
      { "a", "b" },
      "[\"a\", \"b\"]",
    },
    {
      { "\"", "\\" },
      "[\"\\\"\", \"\\\\\"]",
    },
  };

  for (auto t : tests) {
    std::string actual = toString(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


static void testStripExtension()
{
  static struct Test {
    char const *m_input;
    char const *m_expect;
  } const tests[] = {
    { "", "" },
    { "foo.txt", "foo" },
    { "foo.bar.txt", "foo.bar" },
    { "foobar", "foobar" }
  };

  for (auto t : tests) {
    std::string actual = stripExtension(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


static void testIsStrictlySortedArray()
{
  // Basic ordering and rejection of repetition.
  static char const * const arr1[] = {
    "a", "b", "b", "a"
  };
  xassert(isStrictlySortedStringArray(arr1, 0));
  xassert(isStrictlySortedStringArray(arr1, 1));
  xassert(isStrictlySortedStringArray(arr1, 2));
  xassert(!isStrictlySortedStringArray(arr1, 3));
  xassert(!isStrictlySortedStringArray(arr1, 4));
  xassert(!isStrictlySortedStringArray(arr1+2, 2));
  xassert(isStrictlySortedStringArray(arr1+2, 1));

  // ASCII order versus traditional collation order.
  static char const * const arr2[] = {
    "A", "B", "a", "b", "C"
  };
  xassert(isStrictlySortedStringArray(arr2, 4));
  xassert(!isStrictlySortedStringArray(arr2, 5));
}


static void testStringInSortedArray()
{
  static char const * const arr1[] = {
    "baz",
    "foo",
    "foobar",
  };
  xassert(stringInSortedArray("foo", arr1, TABLESIZE(arr1)));
  xassert(stringInSortedArray("foobar", arr1, TABLESIZE(arr1)));
  xassert(!stringInSortedArray("foobaz", arr1, TABLESIZE(arr1)));
  xassert(!stringInSortedArray("goo", arr1, TABLESIZE(arr1)));
  xassert(!stringInSortedArray("fo", arr1, TABLESIZE(arr1)));
}


static void testBeginsWith()
{
  static struct Test {
    char const *m_str;
    char const *m_prefix;
    bool m_expect;
  }
  const tests[] = {
    { "", "", true },
    { "", "x", false },
    { "x", "", true },
    { "x", "x", true },
    { "x", "y", false },
    { "xy", "y", false },
    { "abcdef", "abc", true },
    { "defabc", "abc", false },
  };

  for (auto t : tests) {
    bool actual = beginsWith(t.m_str, t.m_prefix);
    EXPECT_EQ(actual, t.m_expect);
  }
}


void test_string_utils()
{
  testSplitNonEmpty();
  testJoin();
  testDoubleQuote();
  testVectorToString();
  testStripExtension();
  testIsStrictlySortedArray();
  testStringInSortedArray();
  testBeginsWith();
}


// EOF
