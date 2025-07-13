// parsestring-test.cc
// Tests for 'parsestring' module.

#include "parsestring.h"               // module to test

#include "sm-test.h"                   // EXPECT_EQ


// Test basic iteration capabilities.
static void testIter()
{
  ParseString ps("abc");
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.curByte(), (int)'a');
  ps.adv();
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.curByte(), (int)'b');
  ps.adv();
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.curByte(), (int)'c');
  ps.adv();
  EXPECT_EQ(ps.eos(), true);
}


// Test parsing a simple string.
static void testParse1()
{
  ParseString ps("(12,34)");
  ps.parseByte('(');
  EXPECT_EQ(ps.parseDecimalUInt(), 12);
  ps.parseByte(',');
  EXPECT_EQ(ps.parseDecimalUInt(), 34);
  ps.parseByte(')');
  ps.parseEOS();
  EXPECT_EQ(ps.eos(), true);
}


// Parse that fails.
static void testFailParse1()
{
  try {
    ParseString ps("(12!34)");
    ps.parseByte('(');
    EXPECT_EQ(ps.parseDecimalUInt(), 12);
    ps.parseByte(',');
    xfailure("should have failed");
  }
  catch (XParseString &x) {
    EXPECT_EQ(x.m_str, "(12!34)");
    EXPECT_EQ(x.m_offset, 3);
    EXPECT_EQ(x.m_conflict, "found '!', expected ','");
    EXPECT_EQ(x.getMessage(),
      "at location 3 in \"(12!34)\": found '!', expected ','");
  }
}


static void testParsingCText()
{
  ParseString ps("int x 0x123 'a' \"hello\"");
  EXPECT_EQ(ps.parseCToken(), "int");
  ps.skipWS();
  EXPECT_EQ(ps.parseCToken(), "x");
  ps.skipWS();
  EXPECT_EQ(ps.parseCToken(), "0x123");
  ps.skipWS();
  EXPECT_EQ(ps.parseCToken(), "'a'");
  ps.skipWS();
  EXPECT_EQ(ps.parseCToken(), "\"hello\"");
  ps.skipWS();
  ps.parseEOS();
}


static void test_getUpToByte()
{
  {
    ParseString ps("abcdef");
    EXPECT_EQ(ps.getUpToByte('a'), "a");
    EXPECT_EQ(ps.getUpToByte('c'), "bc");
    EXPECT_EQ(ps.getUpToByte('g'), "def");
    xassert(ps.eos());
  }

  {
    ParseString ps("abc");
    EXPECT_EQ(ps.getUpToByte('c'), "abc");
    xassert(ps.eos());
  }
}


static void test_getUpToSize()
{
  {
    ParseString ps("abcdef");
    EXPECT_EQ(ps.curOffset(), 0);
    EXPECT_EQ(ps.getUpToSize(0), "");
    EXPECT_EQ(ps.curOffset(), 0);
    EXPECT_EQ(ps.getUpToSize(1), "a");
    EXPECT_EQ(ps.curOffset(), 1);
    EXPECT_EQ(ps.getUpToSize(2), "bc");
    EXPECT_EQ(ps.curOffset(), 3);
    EXPECT_EQ(ps.getUpToSize(999), "def");
    EXPECT_EQ(ps.curOffset(), 6);
    xassert(ps.eos());
  }

  {
    ParseString ps("abc");
    EXPECT_EQ(ps.curOffset(), 0);
    EXPECT_EQ(ps.getUpToSize(3), "abc");
    EXPECT_EQ(ps.curOffset(), 3);
    xassert(ps.eos());
  }
}


void test_parsestring()
{
  testIter();
  testParse1();
  testFailParse1();
  testParsingCText();
  test_getUpToByte();
  test_getUpToSize();
}


// EOF
