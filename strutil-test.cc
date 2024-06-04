// strutil-test.cc
// Tests for strutil.

#include "strutil.h"                   // module under test

#include "sm-fstream.h"                // ofstream
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ, dummy_printf

#include <assert.h>                    // assert
#include <stdlib.h>                    // getenv
#include <stdio.h>                     // printf, remove


// Silence the output when I'm not actively working on this test.
#define printf dummy_printf


OPEN_ANONYMOUS_NAMESPACE


void basenameVector(char const *in, char const *out)
{
  printf("basenameVector(%s, %s)\n", in, out);
  string result = sm_basename(in);
  xassert(result == out);
}

void dirnameVector(char const *in, char const *out)
{
  printf("dirnameVector(%s, %s)\n", in, out);
  string result = dirname(in);
  xassert(result == out);
}

void pluralVector(int n, char const *in, char const *out)
{
  printf("pluralVector(%d, %s, %s)\n", n, in, out);
  string result = plural(n, in);
  xassert(result == out);
}


static void expectSDQ(string const &s, string const &expect)
{
  string actual = shellDoubleQuote(s);
#if 0
  cout << "shellDoubleQuote:\n"
       << "  s     : " << s << "\n"
       << "  actual: " << actual << "\n"
       << "  expect: " << expect << endl;
#endif // 0
  EXPECT_EQ(actual, expect);
}

static void testShellDoubleQuote()
{
  expectSDQ("", "\"\"");

  expectSDQ("a", "a");
  expectSDQ("abc", "abc");
  expectSDQ("abczAZ01239@-_+:,./", "abczAZ01239@-_+:,./");

  expectSDQ(" ", "\" \"");
  expectSDQ(" a", "\" a\"");
  expectSDQ("x y", "\"x y\"");
  expectSDQ("$`\"\\", "\"\\$\\`\\\"\\\\\"");
  expectSDQ("\n\t ", "\"\n\t \"");
  expectSDQ("\x7F", "\"\x7F\"");
  expectSDQ("\xFF", "\"\xFF\"");
}


static void expectIndexOfSubstring(string const &haystack,
  string const &needle, int expect)
{
  int actual = indexOfSubstring(haystack, needle);
  EXPECT_EQ(actual, expect);

  // Make sure 'hasSubstring' agrees.
  EXPECT_EQ(hasSubstring(haystack, needle), expect != -1);
}

static void testIndexOfSubstring()
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


static void expectQuoteCharacter(int c, string const &expect)
{
  string actual = quoteCharacter(c);
  EXPECT_EQ(actual, expect);
}

static void testQuoteCharacter()
{
  expectQuoteCharacter(0,    "\\x00");
  expectQuoteCharacter(1,    "\\x01");
  expectQuoteCharacter(31,   "\\x1F");
  expectQuoteCharacter(32,   "' '");
  expectQuoteCharacter('"',  "'\"'");
  expectQuoteCharacter('\'', "'\\''");
  expectQuoteCharacter('A',  "'A'");
  expectQuoteCharacter('\\', "'\\\\'");
  expectQuoteCharacter(126,  "'~'");
  expectQuoteCharacter(127,  "\\x7F");
  expectQuoteCharacter(128,  "\\x80");
  expectQuoteCharacter(255,  "\\xFF");

  expectQuoteCharacter(256,    "\\u0100");
  expectQuoteCharacter(0xFFFF, "\\uFFFF");

  expectQuoteCharacter(0x10000,         "\\U00010000");
  expectQuoteCharacter(0x7FFFFFFF,      "\\U7FFFFFFF");
  expectQuoteCharacter(-0x7FFFFFFF - 1, "\\U80000000");
  expectQuoteCharacter(-0xFFFF,         "\\UFFFF0001");
  expectQuoteCharacter(-0xFF,           "\\UFFFFFF01");
  expectQuoteCharacter(-1,              "\\UFFFFFFFF");
  expectQuoteCharacter((unsigned)-1,    "\\UFFFFFFFF");
}


static void testReadLinesFromFile()
{
  ArrayStack<string> lines;
  readLinesFromFile(lines, "test/trlff.txt");
  EXPECT_EQ(lines.length(), 4);
  EXPECT_EQ(lines[0], "This is test input for strutil.cc, testReadLinesFromFile().");
  EXPECT_EQ(lines[1], "This line is longer than 80 characters in order to exercise that code in readLine.  It is sort of weird to have done it that way but whatever.  This is long enough that it will have to iterate more than once.  Also the next line is blank.");
  EXPECT_EQ(lines[2], "");
  EXPECT_EQ(lines[3], "Final line.  This file has exactly four lines.");

  lines.clear();
  readLinesFromFile(lines, "test/trlff2.txt");
  EXPECT_EQ(lines.length(), 1);
  EXPECT_EQ(lines[0], "One line, no newline terminator.");

  lines.clear();
  readLinesFromFile(lines, "test/trlff2.txt", false /*chomp*/);
  EXPECT_EQ(lines.length(), 1);
  EXPECT_EQ(lines[0], "One line, no newline terminator.");

  lines.clear();
  readLinesFromFile(lines, "test/trlff3.txt", false /*chomp*/);
  EXPECT_EQ(lines.length(), 2);
  EXPECT_EQ(lines[0], "Two lines.\n");
  EXPECT_EQ(lines[1], "Last line has no newline terminator.");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_strutil()
{
  basenameVector("a/b/c", "c");
  basenameVector("abc", "abc");
  basenameVector("/", "");
  basenameVector("a/b/c/", "c");

  dirnameVector("a/b/c", "a/b");
  dirnameVector("a/b/c/", "a/b");
  dirnameVector("/a", "/");
  dirnameVector("abc", ".");
  dirnameVector("/", "/");

  pluralVector(1, "path", "path");
  pluralVector(2, "path", "paths");
  pluralVector(1, "fly", "fly");
  pluralVector(2, "fly", "flies");
  pluralVector(2, "was", "were");

  {
    string x("x");
    string y("y");
    xassert(compareStringPtrs(&x, &y) < 0);
    xassert(compareStringPtrs(&y, &y) == 0);
    xassert(compareStringPtrs(&y, &x) > 0);
  }

  testShellDoubleQuote();
  testIndexOfSubstring();
  testQuoteCharacter();
  testReadLinesFromFile();
}


// EOF
