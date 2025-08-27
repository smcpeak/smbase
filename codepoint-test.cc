// codepoint-test.cc
// Test code for 'codepoint' module.

#include "codepoint.h"                 // module to test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "xassert.h"                   // xassert

#include <iostream>                    // std::{cout, endl}


OPEN_ANONYMOUS_NAMESPACE


void test_various()
{
  xassert(isWhitespace(' '));
  xassert(isWhitespace('\t'));
  xassert(isWhitespace('\f'));
  xassert(isWhitespace('\v'));
  xassert(isWhitespace('\n'));
  xassert(isWhitespace('\r'));
  xassert(!isWhitespace('x'));
  xassert(!isWhitespace('3'));

  xassert(isASCIIDigit('0'));
  xassert(isASCIIDigit('1'));
  xassert(isASCIIDigit('8'));
  xassert(isASCIIDigit('9'));
  xassert(!isASCIIDigit('a'));

  xassert(isASCIIHexDigit('0'));
  xassert(isASCIIHexDigit('9'));
  xassert(isASCIIHexDigit('A'));
  xassert(isASCIIHexDigit('F'));
  xassert(isASCIIHexDigit('a'));
  xassert(isASCIIHexDigit('f'));
  xassert(!isASCIIHexDigit('g'));
  xassert(!isASCIIHexDigit('-'));

  xassert(isASCIIOctDigit('0'));
  xassert(isASCIIOctDigit('7'));
  xassert(!isASCIIOctDigit('8'));

  EXPECT_EQ(decodeRadixIndicatorLetter('b'), 2);
  EXPECT_EQ(decodeRadixIndicatorLetter('O'), 8);
  EXPECT_EQ(decodeRadixIndicatorLetter('x'), 16);
  EXPECT_EQ(decodeRadixIndicatorLetter('t'), 0);
  EXPECT_EQ(decodeRadixIndicatorLetter(0), 0);

  EXPECT_EQ(decodeASCIIRadixDigit('F', 16), 15);
  EXPECT_EQ(decodeASCIIRadixDigit('F', 10), -1);
  EXPECT_EQ(decodeASCIIRadixDigit('z', 36), 35);
  EXPECT_EQ(decodeASCIIRadixDigit('7', 16),  7);

  EXPECT_EQ(isASCIIRadixDigit('A', 16), true);
  EXPECT_EQ(isASCIIRadixDigit('A', 10), false);

  EXPECT_EQ(encodeRadixIndicatorLetter(10), '\0');
  EXPECT_EQ(encodeRadixIndicatorLetter(25), '\0');
  EXPECT_EQ(encodeRadixIndicatorLetter(16), 'x');
  EXPECT_EQ(encodeRadixIndicatorLetter(8), 'o');
  EXPECT_EQ(encodeRadixIndicatorLetter(2), 'b');

  EXPECT_EQ(decodeSurrogatePair(0xD800, 0xDC00), CodePoint(0x10000));
  EXPECT_EQ(decodeSurrogatePair(0xDBFF, 0xDFFF), CodePoint(0x10FFFF));

  xassert(isCIdentifierCharacter('x'));
  xassert(isCIdentifierCharacter('Q'));
  xassert(isCIdentifierCharacter('9'));
  xassert(isCIdentifierCharacter('_'));
  xassert(!isCIdentifierCharacter(','));

  xassert(isCIdentifierStartCharacter('x'));
  xassert(isCIdentifierStartCharacter('Q'));
  xassert(!isCIdentifierStartCharacter('9'));
  xassert(isCIdentifierStartCharacter('_'));
  xassert(!isCIdentifierStartCharacter(','));

  xassert(isCWhitespace(' '));
  xassert(isCWhitespace('\t'));
  xassert(!isCWhitespace('x'));

  // Check that the implicit conversion from `char` to `CodePoint` works
  // as intended when it is negative.
  {
    char c = -1;
    xassert(CodePoint(c).value() == 255);
  }

  {
    signed char c = -1;
    xassert(CodePoint(c).value() == 255);
  }

  xassert(!CodePoint(-1 /*int*/).has_value());
}


void test_isShellMetaCharacter()
{
  EXPECT_EQ(isShellMetacharacter('`'), true);
  EXPECT_EQ(isShellMetacharacter('a'), false);
  EXPECT_EQ(isShellMetacharacter(' '), true);
  EXPECT_EQ(isShellMetacharacter('='), true);
  EXPECT_EQ(isShellMetacharacter('=', true /*afterProgram*/), false);
}


void test_isSlashOrBackslash()
{
  EXPECT_EQ(isSlashOrBackslash(0), false);
  EXPECT_EQ(isSlashOrBackslash('x'), false);
  EXPECT_EQ(isSlashOrBackslash('/'), true);
  EXPECT_EQ(isSlashOrBackslash('\\'), true);
}


void test_isSpaceOrTab()
{
  EXPECT_EQ(isSpaceOrTab(0), false);
  EXPECT_EQ(isSpaceOrTab('x'), false);
  EXPECT_EQ(isSpaceOrTab(' '), true);
  EXPECT_EQ(isSpaceOrTab('\t'), true);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_codepoint()
{
  // This does not test everything yet.

  test_various();
  test_isShellMetaCharacter();
  test_isSlashOrBackslash();
  test_isSpaceOrTab();
}


// EOF
