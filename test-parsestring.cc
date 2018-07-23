// test-parsestring.cc
// Tests for 'parsestring' module.

#include "parsestring.h"               // module to test

#include "test.h"                      // EXPECT_EQ


// Test basic iteration capabilities.
static void testIter()
{
  ParseString ps("abc");
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.cur(), (int)'a');
  ps.adv();
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.cur(), (int)'b');
  ps.adv();
  EXPECT_EQ(ps.eos(), false);
  EXPECT_EQ(ps.cur(), (int)'c');
  ps.adv();
  EXPECT_EQ(ps.eos(), true);
}


// Test parsing a simple string.
static void testParse1()
{
  ParseString ps("(12,34)");
  ps.parseChar('(');
  EXPECT_EQ(ps.parseDecimalUInt(), 12);
  ps.parseChar(',');
  EXPECT_EQ(ps.parseDecimalUInt(), 34);
  ps.parseChar(')');
  ps.parseEOS();
  EXPECT_EQ(ps.eos(), true);
}


// Parse that fails.
static void testFailParse1()
{
  try {
    ParseString ps("(12!34)");
    ps.parseChar('(');
    EXPECT_EQ(ps.parseDecimalUInt(), 12);
    ps.parseChar(',');
    xfailure("should have failed");
  }
  catch (XParseString &x) {
    EXPECT_EQ(x.m_str, "(12!34)");
    EXPECT_EQ(x.m_offset, 3);
    EXPECT_EQ(x.m_conflict, "found '!', expected ','");
    EXPECT_EQ(x.cond(),
      "at location 3 in \"(12!34)\": found '!', expected ','");
  }
}


void test_parsestring()
{
  testIter();
  testParse1();
  testFailParse1();

  cout << "test_parsestring PASSED" << endl;
}


// EOF
