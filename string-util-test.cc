// string-util-test.cc
// Test code for string-util.

#include "string-util.h"               // module under test

#include "sm-test.h"                   // EXPECT_EQ, tprintf

#include <exception>                   // std::exception
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


static void testPrefixAll()
{
  EXPECT_EQ(prefixAll(std::vector<std::string>{}, "foo"),
                      std::vector<std::string>{});

  EXPECT_EQ(prefixAll(std::vector<std::string>{"x"}, "foo"),
                      std::vector<std::string>{"foox"});

  EXPECT_EQ(prefixAll(std::vector<std::string>{"x", ""}, "foo"),
                     (std::vector<std::string>{"foox", "foo"}));

  EXPECT_EQ(prefixAll(std::vector<std::string>{"x", ""}, ""),
                     (std::vector<std::string>{"x", ""}));
}


static void testSuffixAll()
{
  EXPECT_EQ(suffixAll(std::vector<std::string>{}, "foo"),
                      std::vector<std::string>{});

  EXPECT_EQ(suffixAll(std::vector<std::string>{"x"}, "foo"),
                      std::vector<std::string>{"xfoo"});

  EXPECT_EQ(suffixAll(std::vector<std::string>{"x", ""}, "foo"),
                     (std::vector<std::string>{"xfoo", "foo"}));

  EXPECT_EQ(suffixAll(std::vector<std::string>{"x", ""}, ""),
                     (std::vector<std::string>{"x", ""}));
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


static void testOneMatchesRegex(
  char const *str,
  char const *re,
  bool expect)
{
  try {
    bool actual = matchesRegex(str, re);
    EXPECT_EQ(actual, expect);
  }
  catch (std::exception &e) {
    xfailure(stringbc(
      "testOneMatchesRegex failed:\n"
      "  str: " << doubleQuote(str) << "\n"
      "  re : " << doubleQuote(re) << "\n"
      "  e  : " << e.what()));
  }
}


static void testMatchesRegex()
{
  testOneMatchesRegex("hello", "el", true);
}


static void testInsertPossiblyEscapedChar()
{
  std::ostringstream oss;
  insertPossiblyEscapedChar(oss, 'x');
  insertPossiblyEscapedChar(oss, '\0');
  insertPossiblyEscapedChar(oss, '\n');
  EXPECT_EQ(oss.str(), std::string("x\\000\\n"));
}


static void testSingleQuoteChar()
{
  EXPECT_EQ(singleQuoteChar('x'), std::string("'x'"));
  EXPECT_EQ(singleQuoteChar('\0'), std::string("'\\000'"));
  EXPECT_EQ(singleQuoteChar('\n'), std::string("'\\n'"));
}


static void testEscapeForRegex()
{
  EXPECT_EQ(escapeForRegex("["), "\\[");
  EXPECT_EQ(escapeForRegex("(*hello*)"), "\\(\\*hello\\*\\)");
}


static void testInt64ToRadixDigits()
{
  for (int r=2; r <= 36; ++r) {
    EXPECT_EQ(int64ToRadixDigits(0, r, false), "0");
    EXPECT_EQ(int64ToRadixDigits(1, r, false), "1");
    EXPECT_EQ(int64ToRadixDigits(-1, r, false), "-1");
  }

  EXPECT_EQ(int64ToRadixDigits(0, 2, true), "0b0");
  EXPECT_EQ(int64ToRadixDigits(0, 8, true), "0o0");
  EXPECT_EQ(int64ToRadixDigits(0, 10, true), "0");
  EXPECT_EQ(int64ToRadixDigits(0, 16, true), "0x0");

  int64_t maxVal = std::numeric_limits<int64_t>::max();
  EXPECT_EQ(int64ToRadixDigits(maxVal, 16, true), "0x7FFFFFFFFFFFFFFF");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 8, true), "0o777777777777777777777");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 2, true), "0b111111111111111111111111111111111111111111111111111111111111111");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 10, true), "9223372036854775807");

  --maxVal;
  EXPECT_EQ(int64ToRadixDigits(maxVal, 16, true), "0x7FFFFFFFFFFFFFFE");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 8, true), "0o777777777777777777776");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 2, true), "0b111111111111111111111111111111111111111111111111111111111111110");
  EXPECT_EQ(int64ToRadixDigits(maxVal, 10, true), "9223372036854775806");

  int64_t minVal = std::numeric_limits<int64_t>::min();
  EXPECT_EQ(int64ToRadixDigits(minVal, 16, true), "-0x8000000000000000");
  EXPECT_EQ(int64ToRadixDigits(minVal, 8, true), "-0o1000000000000000000000");
  EXPECT_EQ(int64ToRadixDigits(minVal, 2, true), "-0b1000000000000000000000000000000000000000000000000000000000000000");
  EXPECT_EQ(int64ToRadixDigits(minVal, 10, true), "-9223372036854775808");

  ++minVal;
  EXPECT_EQ(int64ToRadixDigits(minVal, 16, true), "-0x7FFFFFFFFFFFFFFF");
  EXPECT_EQ(int64ToRadixDigits(minVal, 8, true), "-0o777777777777777777777");
  EXPECT_EQ(int64ToRadixDigits(minVal, 2, true), "-0b111111111111111111111111111111111111111111111111111111111111111");
  EXPECT_EQ(int64ToRadixDigits(minVal, 10, true), "-9223372036854775807");

  ++minVal;
  EXPECT_EQ(int64ToRadixDigits(minVal, 16, true), "-0x7FFFFFFFFFFFFFFE");
  EXPECT_EQ(int64ToRadixDigits(minVal, 8, true), "-0o777777777777777777776");
  EXPECT_EQ(int64ToRadixDigits(minVal, 2, true), "-0b111111111111111111111111111111111111111111111111111111111111110");
  EXPECT_EQ(int64ToRadixDigits(minVal, 10, true), "-9223372036854775806");

  EXPECT_EQ(int64ToRadixDigits(minVal, 16, false), "-7FFFFFFFFFFFFFFE");
}


static void testReplace()
{
  EXPECT_EQ(replace("", "", ""), "");
  EXPECT_EQ(replace("xxx", "x", "xxx"), "xxxxxxxxx");
  EXPECT_EQ(replace("xxx", "x", ""), "");
  EXPECT_EQ(replace("Just some text. Just some text.", "some", "SOME"),
                    "Just SOME text. Just SOME text.");
}


static void expRangeVector(char const *in, char const *out)
{
  tprintf("expRangeVector(%s, %s)\n", in, out);
  string result = expandRanges(in);
  xassert(result == out);
}


static void testExpandRanges()
{
  expRangeVector("abcd", "abcd");
  expRangeVector("a", "a");
  expRangeVector("a-k", "abcdefghijk");
  expRangeVector("0-9E-Qz", "0123456789EFGHIJKLMNOPQz");
}


static void trVector(char const *in, char const *srcSpec, char const *destSpec, char const *out)
{
  tprintf("trVector(%s, %s, %s, %s)\n", in, srcSpec, destSpec, out);
  string result = translate(in, srcSpec, destSpec);
  EXPECT_EQ(result, out);
}


// testcase from Hendrik Tews
static void translateAscii()
{
  char input[256];
  char underscores[256];
  char expect[256];

  for(int i=0; i<=254; i++){
    input[i] = i+1;
    underscores[i] = '_';

    // The character value stored in `input[i]`.
    int c = i+1;

    // Apply the transformation specification below.
    int c_out =
      (0001 <= c && c <= 0057) ||
      (0072 <= c && c <= 0101) ||
      (0133 <= c && c <= 0140) ||
      (0173 <= c && c <= 0377)     ? '_' : c;

    expect[i] = c_out;
  }
  input[255] = 0;
  underscores[255] = 0;
  expect[255] = 0;

  std::string actual =
    translate(input, "\001-\057\072-\101\133-\140\173-\377", underscores);
                                  // ^^^ probably should be 100, no biggie

  EXPECT_EQ(actual, expect);
}


static void testTranslate()
{
  trVector("foo", "a-z", "A-Z", "FOO");
  trVector("foo BaR", "a-z", "A-Z", "FOO BAR");
  trVector("foo BaR", "m-z", "M-Z", "fOO BaR");

  translateAscii();
}


static void testTrimWhitespace()
{
  EXPECT_EQ(trimWhitespace(""), "");
  EXPECT_EQ(trimWhitespace(" "), "");
  EXPECT_EQ(trimWhitespace("  "), "");
  EXPECT_EQ(trimWhitespace(" x"), "x");
  EXPECT_EQ(trimWhitespace(" x y "), "x y");
  EXPECT_EQ(trimWhitespace("\t x y "), "x y");
}


void test_string_utils()
{
  testSplitNonEmpty();
  testJoin();
  testPrefixAll();
  testSuffixAll();
  testDoubleQuote();
  testVectorToString();
  testStripExtension();
  testIsStrictlySortedArray();
  testStringInSortedArray();
  testBeginsWith();
  testMatchesRegex();
  testInsertPossiblyEscapedChar();
  testSingleQuoteChar();
  testEscapeForRegex();
  testInt64ToRadixDigits();
  testReplace();
  testExpandRanges();
  testTranslate();
  testTrimWhitespace();
}


// EOF
