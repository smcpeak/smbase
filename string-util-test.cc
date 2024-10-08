// string-util-test.cc
// Test code for string-util.

#include "string-util.h"               // module under test

#include "exc.h"                       // EXN_CONTEXT
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ, tprintf

#include <exception>                   // std::exception
#include <iostream>                    // std::ostream
#include <limits>                      // std::numeric_limits


OPEN_ANONYMOUS_NAMESPACE


void testSplit()
{
  EXN_CONTEXT("testSplit");

  struct Test {
    char const *m_input;
    std::vector<std::string> m_expect;
  }
  const tests[] = {
    {
      "",
      {""},
    },
    {
      " ",
      {"", ""},
    },
    {
      "a",
      {"a"},
    },
    {
      "a  ",
      {"a", "", ""},
    },
    {
      "a bar c",
      {"a", "bar", "c"},
    },
    {
      " a  b  c",
      {"", "a", "", "b", "", "c"},
    },
  };

  for (auto const &t : tests) {
    EXN_CONTEXT(doubleQuote(t.m_input));
    std::vector<std::string> actual = split(t.m_input, ' ');
    EXPECT_EQ(actual, t.m_expect);
  }

  EXPECT_EQ(split("one\ntwo\nthree\n", '\n'),
            (std::vector<std::string>{
              "one",
              "two",
              "three",
              ""
            }));
}


void testSplitNonEmpty()
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

  for (auto const &t : tests) {
    std::vector<std::string> actual = splitNonEmpty(t.m_input, ' ');
    EXPECT_EQ(actual, t.m_expect);
  }
}


void testNumLeadingChars()
{
  EXPECT_EQ(numLeadingChars("", ' '), 0);
  EXPECT_EQ(numLeadingChars(" ", ' '), 1);
  EXPECT_EQ(numLeadingChars(" ", 'x'), 0);
  EXPECT_EQ(numLeadingChars("x ", 'x'), 1);
  EXPECT_EQ(numLeadingChars("xx xx", 'x'), 2);
}


void testJoin()
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

  for (auto const &t : tests) {
    std::string actual = join(t.m_vec, t.m_sep);
    EXPECT_EQ(actual, t.m_expect);
  }
}


void testPrefixAll()
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


void testSuffixAll()
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


void testDoubleQuote()
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

  for (auto const &t : tests) {
    std::string actual = doubleQuote(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


void testVectorToString()
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

  for (auto const &t : tests) {
    std::string actual = toString(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


void testStripExtension()
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

  for (auto const &t : tests) {
    std::string actual = stripExtension(t.m_input);
    EXPECT_EQ(actual, std::string(t.m_expect));
  }
}


void testIsStrictlySortedArray()
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


void testStringInSortedArray()
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


void testBeginsWith()
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

  for (auto const &t : tests) {
    bool actual = beginsWith(t.m_str, t.m_prefix);
    EXPECT_EQ(actual, t.m_expect);
  }
}


void testEndsWith()
{
  static struct Test {
    char const *m_str;
    char const *m_suffix;
    bool m_expect;
  }
  const tests[] = {
    // Columns: \S+ \S+ \S+ \S+ \S+
    { "",       "",    true  },
    { "",       "x",   false },
    { "x",      "",    true  },
    { "x",      "x",   true  },
    { "x",      "y",   false },
    { "xy",     "y",   true  },
    { "yx",     "y",   false },
    { "abcdef", "abc", false },
    { "defabc", "abc", true  },
    { "a\n",    "\n",  true  },
  };

  for (auto const &t : tests) {
    bool actual = endsWith(t.m_str, t.m_suffix);
    EXPECT_EQ(actual, t.m_expect);
  }
}


void testOneMatchesRegex(
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


void testMatchesRegex()
{
  testOneMatchesRegex("hello", "el", true);

  // In the regex, if ']' is not escaped, then Clang libc++ throws an
  // exception with the cryptic text: "The parser did not consume the
  // entire regular expression.".  GNU libc++ does not require that to
  // be escaped.
  testOneMatchesRegex(
    "Unexpected end of file while looking for ']' at end of sequence.",
    "end of file.*looking for '\\]' at end of sequence",
    true);

  testOneMatchesRegex(
    "Unexpected end of file while looking for 'x' at end of sequence.",
    "end of file.*looking for '\\]' at end of sequence",
    false);
}


void testOneInvalidRegex(char const *badRE)
{
  DIAG("badRE: " << badRE);

  try {
    matchesRegex("foo", badRE);
    DIAG("no exception!");
  }
  catch (std::exception &e) {
    DIAG("exn: " << e.what());
  }
}


void testInvalidRegex()
{
  testOneInvalidRegex("unclosed [bracket");
  testOneInvalidRegex("unclosed (paren");
  testOneInvalidRegex("unclosed {brace");

  // This should be rejected, but GCC 9.3 libc++ does not.
  testOneInvalidRegex("imbalanced ]bracket"); // ok?

  testOneInvalidRegex("imbalanced )paren");

  // Similarly, GCC 9.3 does not reject this either.
  testOneInvalidRegex("imbalanced }brace");
}


void testInsertPossiblyEscapedChar()
{
  std::ostringstream oss;
  insertPossiblyEscapedChar(oss, 'x');
  insertPossiblyEscapedChar(oss, '\0');
  insertPossiblyEscapedChar(oss, '\n');
  EXPECT_EQ(oss.str(), std::string("x\\000\\n"));
}


void expectSingleQuoteChar(int c, string const &expect)
{
  string actual = singleQuoteChar(c);
  EXPECT_EQ(actual, expect);
}

void testSingleQuoteChar()
{
  EXPECT_EQ(singleQuoteChar('x'), std::string("'x'"));
  EXPECT_EQ(singleQuoteChar('\0'), std::string("'\\000'"));
  EXPECT_EQ(singleQuoteChar('\n'), std::string("'\\n'"));

  expectSingleQuoteChar(0,    "'\\000'");
  expectSingleQuoteChar(1,    "'\\001'");
  expectSingleQuoteChar(31,   "'\\037'");
  expectSingleQuoteChar(32,   "' '");
  expectSingleQuoteChar('"',  "'\"'");
  expectSingleQuoteChar('\'', "'\\''");
  expectSingleQuoteChar('A',  "'A'");
  expectSingleQuoteChar('\\', "'\\\\'");
  expectSingleQuoteChar(126,  "'~'");
  expectSingleQuoteChar(127,  "'\\177'");
  expectSingleQuoteChar(128,  "'\\200'");
  expectSingleQuoteChar(255,  "'\\377'");

  expectSingleQuoteChar(256,    "'\\u{100}'");
  expectSingleQuoteChar(0xFFFF, "'\\u{FFFF}'");

  expectSingleQuoteChar(0x10000,  "'\\u{10000}'");
  expectSingleQuoteChar(0x10FFFF, "'\\u{10FFFF}'");
}


void testEscapeForRegex()
{
  EXPECT_EQ(escapeForRegex("["), "\\[");
  EXPECT_EQ(escapeForRegex("(*hello*)"), "\\(\\*hello\\*\\)");
}


void testInt64ToRadixDigits()
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


void testReplaceAll()
{
  EXPECT_EQ(replaceAll("", "x", ""), "");
  EXPECT_EQ(replaceAll("", "x", "y"), "");
  EXPECT_EQ(replaceAll("xxx", "x", "xxx"), "xxxxxxxxx");
  EXPECT_EQ(replaceAll("xxx", "x", ""), "");
  EXPECT_EQ(replaceAll("Just some text. Just some text.", "some", "SOME"),
                       "Just SOME text. Just SOME text.");
}


void expRangeVector(char const *in, char const *out)
{
  tprintf("expRangeVector(%s, %s)\n", in, out);
  string result = expandRanges(in);
  xassert(result == out);
}


void testExpandRanges()
{
  expRangeVector("abcd", "abcd");
  expRangeVector("a", "a");
  expRangeVector("a-k", "abcdefghijk");
  expRangeVector("0-9E-Qz", "0123456789EFGHIJKLMNOPQz");
}


void trVector(char const *in, char const *srcSpec, char const *destSpec, char const *out)
{
  tprintf("trVector(%s, %s, %s, %s)\n", in, srcSpec, destSpec, out);
  string result = translate(in, srcSpec, destSpec);
  EXPECT_EQ(result, out);
}


// testcase from Hendrik Tews
void translateAscii()
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


void testTranslate()
{
  trVector("foo", "a-z", "A-Z", "FOO");
  trVector("foo BaR", "a-z", "A-Z", "FOO BAR");
  trVector("foo BaR", "m-z", "M-Z", "fOO BaR");

  translateAscii();
}


void testTrimWhitespace()
{
  EXPECT_EQ(trimWhitespace(""), "");
  EXPECT_EQ(trimWhitespace(" "), "");
  EXPECT_EQ(trimWhitespace("  "), "");
  EXPECT_EQ(trimWhitespace(" x"), "x");
  EXPECT_EQ(trimWhitespace(" x y "), "x y");
  EXPECT_EQ(trimWhitespace("\t x y "), "x y");
}


void testRemoveSuffix()
{
  EXPECT_EQ(removeSuffix("", ""), "");
  EXPECT_EQ(removeSuffix("xyz", "z"), "xy");
  EXPECT_EQ(removeSuffix("xyz", "Z"), "xyz");
  EXPECT_EQ(removeSuffix("xyz", "xyz"), "");
}


void testEncodeWithEscapes()
{
  EXPECT_EQ(encodeWithEscapes(""), "");
  EXPECT_EQ(encodeWithEscapes("abc"), "abc");
  EXPECT_EQ(encodeWithEscapes("\r\n"), "\\r\\n");
  EXPECT_EQ(encodeWithEscapes(std::string("a\0b", 3)), "a\\000b");
}


void expectIndexOfSubstring(std::string const &haystack,
  std::string const &needle, int expect)
{
  int actual = indexOfSubstring(haystack, needle);
  EXPECT_EQ(actual, expect);

  // Make sure 'hasSubstring' agrees.
  EXPECT_EQ(hasSubstring(haystack, needle), expect != -1);
}


void testIndexOfSubstring()
{
  expectIndexOfSubstring("", "", 0);
  expectIndexOfSubstring("", "x", -1);
  expectIndexOfSubstring("x", "", 0);
  expectIndexOfSubstring("x", "x", 0);
  expectIndexOfSubstring("abcdcde", "c", 2);
  expectIndexOfSubstring("abcdcde", "e", 6);
  expectIndexOfSubstring("abcdcde", "cd", 2);
  expectIndexOfSubstring("abcdcde", "ce", -1);
  expectIndexOfSubstring("foofoobar", "foobar", 3);
  expectIndexOfSubstring("foofoofoobar", "foofoobar", 3);
  expectIndexOfSubstring("foofoofooba", "foofoobar", -1);
}


void testReplaceAllRegex()
{
  EXPECT_EQ(replaceAllRegex("", "x", ""), "");
  EXPECT_EQ(replaceAllRegex("x", "x", ""), "");
  EXPECT_EQ(replaceAllRegex("xx", "x", ""), "");
  EXPECT_EQ(replaceAllRegex("xyz", "x", ""), "yz");
  EXPECT_EQ(replaceAllRegex("xyz", "x", "yz"), "yzyz");
  EXPECT_EQ(replaceAllRegex("SOME text SAMPLE", "[a-z]", "Q"), "SOME QQQQ SAMPLE");
  EXPECT_EQ(replaceAllRegex("code // d: comment", " *// d: .*", ""), "code");
  EXPECT_EQ(replaceAllRegex("code // comment", " *// d: *", ""), "code // comment");
}


void testStringVectorFromPointerArray()
{
  EXPECT_EQ(stringVectorFromPointerArray(0, nullptr),
            std::vector<std::string>{});

  {
    char const *argv[] = {"a"};
    EXPECT_EQ(stringVectorFromPointerArray(0, argv),
              std::vector<std::string>{});
    EXPECT_EQ(stringVectorFromPointerArray(TABLESIZE(argv), argv),
              std::vector<std::string>{"a"});
  }

  {
    char const *argv[] = {"a", "b", "c"};
    EXPECT_EQ(stringVectorFromPointerArray(0, argv),
              std::vector<std::string>{});
    EXPECT_EQ(stringVectorFromPointerArray(TABLESIZE(argv), argv),
              (std::vector<std::string>{"a", "b", "c"}));
  }
}


void testRemoveTestCaseIndentation()
{
  EXPECT_EQ(removeTestCaseIndentation(R"(
    one
    two

    four
  )"),
  "one\ntwo\n\nfour\n");

  EXPECT_EQ(removeTestCaseIndentation(R"(
    a
  )"),
  "a\n");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_string_util()
{
  testSplit();
  testSplitNonEmpty();
  testNumLeadingChars();
  testJoin();
  testPrefixAll();
  testSuffixAll();
  testDoubleQuote();
  testVectorToString();
  testStripExtension();
  testIsStrictlySortedArray();
  testStringInSortedArray();
  testBeginsWith();
  testEndsWith();
  testMatchesRegex();
  testInvalidRegex();
  testInsertPossiblyEscapedChar();
  testSingleQuoteChar();
  testEscapeForRegex();
  testInt64ToRadixDigits();
  testReplaceAll();
  testExpandRanges();
  testTranslate();
  testTrimWhitespace();
  testRemoveSuffix();
  testEncodeWithEscapes();
  testIndexOfSubstring();
  testReplaceAllRegex();
  testStringVectorFromPointerArray();
  testRemoveTestCaseIndentation();
}


// EOF
