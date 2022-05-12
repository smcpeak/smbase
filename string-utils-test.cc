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


void test_string_utils()
{
  testSplitNonEmpty();
  testDoubleQuote();
  testVectorToString();
  testStripExtension();
}


// EOF
