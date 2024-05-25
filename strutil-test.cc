// strutil-test.cc
// Tests for strutil.

#include "strutil.h"                   // module under test

#include "sm-fstream.h"                // ofstream
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ

#include <assert.h>                    // assert
#include <stdlib.h>                    // getenv
#include <stdio.h>                     // printf, remove


OPEN_ANONYMOUS_NAMESPACE


void expRangeVector(char const *in, char const *out)
{
  printf("expRangeVector(%s, %s)\n", in, out);
  string result = expandRanges(in);
  xassert(result == out);
}

void trVector(char const *in, char const *srcSpec, char const *destSpec, char const *out)
{
  printf("trVector(%s, %s, %s, %s)\n", in, srcSpec, destSpec, out);
  string result = translate(in, srcSpec, destSpec);
  xassert(result == out);
}

void decodeVector(char const *in, char const *out, int outLen)
{
  printf("decodeVector: \"%s\"\n", in);
  ArrayStack<char> dest;
  decodeEscapes(dest, in, '\0' /*delim, ignored*/, false /*allowNewlines*/);
  xassert(dest.length() == outLen);
  xassert(0==memcmp(out, dest.getArray(), dest.length()));
}

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


// testcase from Hendrik Tews
void translateAscii()
{
  char ascii[256];
  char underscore[256];

  for(int i=0; i<=254; i++){
    ascii[i] = i+1;
    underscore[i] = '_';
  }
  ascii[255] = 0;
  underscore[255] = 0;

  {
    ofstream file("strutil.out");
    assert(file);
    file << "Hallo" << endl
         << ascii << endl
         << "Hallo2" << endl
         << translate(ascii, "\001-\057\072-\101\133-\140\173-\377", underscore)
                                          // ^^^ probably should be 100, no biggie
         << endl;
  }

  if (!getenv("SAVE_OUTPUT")) {
    remove("strutil.out");
  }
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
  cout << "testIndexOfSubstring" << endl;
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
  expRangeVector("abcd", "abcd");
  expRangeVector("a", "a");
  expRangeVector("a-k", "abcdefghijk");
  expRangeVector("0-9E-Qz", "0123456789EFGHIJKLMNOPQz");

  trVector("foo", "a-z", "A-Z", "FOO");
  trVector("foo BaR", "a-z", "A-Z", "FOO BAR");
  trVector("foo BaR", "m-z", "M-Z", "fOO BaR");

  decodeVector("\\r\\n", "\r\n", 2);
  decodeVector("abc\\0def", "abc\0def", 7);
  decodeVector("\\033", "\033", 1);
  decodeVector("\\x33", "\x33", 1);
  decodeVector("\\?", "?", 1);

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

  translateAscii();

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

  cout << "strutil ok" << endl;
}


// EOF
