// codepoint-test.cc
// Test code for 'codepoint' module.

#include "codepoint.h"                 // module to test

#include "xassert.h"                   // xassert

#include <iostream>                    // std::{cout, endl}


// Called from unit-tests.cc.
void test_codepoint()
{
  // This does not test everything yet.

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

  std::cout << "test-codepoint passed" << std::endl;
}


// EOF
