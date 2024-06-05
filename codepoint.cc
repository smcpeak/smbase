// codepoint.cc
// code for codepoint.h

#include "codepoint.h"                 // this module

#include "str.h"                       // stringf
#include "string-util.h"               // singleQuoteChar
#include "xassert.h"                   // xfailure_stringbc

#include <iostream>                    // std::ostream


void CodePoint::write(std::ostream &os) const
{
  os << valueOrN1();
}


// Return true if `c` is in [`lo`,`hi`].
static bool inRange(CodePoint c, int lo, int hi)
{
  int v = c.valueOrN1();
  return lo <= v && v <= hi;
}


bool isUppercaseLetter(CodePoint c)
{
  // TODO: This is incomplete.
  return isASCIIUppercaseLetter(c);
}


bool isLowercaseLetter(CodePoint c)
{
  // TODO: This is incomplete.
  return isASCIILowercaseLetter(c);
}


bool isLetter(CodePoint c)
{
  return isUppercaseLetter(c) || isLowercaseLetter(c);
}


bool isDecimalDigit(CodePoint c)
{
  // TODO: This might be incomplete.
  return isASCIIDigit(c);
}


bool isWhitespace(CodePoint c)
{
  switch (c.valueOrN1()) {
    // List from https://en.wikipedia.org/wiki/Whitespace_character.
    case 0x9: // tab
    case 0xA: // line feed
    case 0xB: // vertical tab
    case 0xC: // form feed
    case 0xD: // carriage return
    case 0x20: // space
    case 0x85: // next line
    case 0xA0: // no-break space
    case 0x1680: // ogham space mark
    case 0x2000: // en quad
    case 0x2001: // em quad
    case 0x2002: // en space
    case 0x2003: // em space
    case 0x2004: // three-per-em space
    case 0x2005: // four-per-em space
    case 0x2006: // six-per-em-space
    case 0x2007: // figure space
    case 0x2008: // punctuation space
    case 0x2009: // thin space
    case 0x200A: // hair space
    case 0x2028: // line separator
    case 0x2029: // paragraph separator
    case 0x202F: // narrow no-break space
    case 0x205F: // medium mathematical space
    case 0x3000: // ideographic space
      return true;

    default:
      return false;
  }
}


bool isHighSurrogate(CodePoint c)
{
  return inRange(c, 0xD800, 0xDBFF);
}


bool isLowSurrogate(CodePoint c)
{
  return inRange(c, 0xDC00, 0xDFFF);
}


bool isCIdentifierCharacter(CodePoint c)
{
  // These do *not* use the Unicode functions defined above because C
  // identifiers are restricted to code points in [0,127], whereas my
  // intent is to expand the Unicode functions to properly recognize
  // the full sets.
  return isCIdentifierStartCharacter(c) ||
         isASCIIDigit(c);
}


bool isCIdentifierStartCharacter(CodePoint c)
{
  return isASCIILetter(c) ||
         c == '_';
}


bool isCWhitespace(CodePoint c)
{
  switch (c.valueOrN1()) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
      return true;

    default:
      return false;
  }
}


bool isASCIIPrintable(CodePoint c)
{
  return inRange(c, 32, 126);
}


bool isASCIIDigit(CodePoint c)
{
  return inRange(c, '0', '9');
}


bool isASCIIUppercaseLetter(CodePoint c)
{
  return inRange(c, 'A', 'Z');
}


bool isASCIILowercaseLetter(CodePoint c)
{
  return inRange(c, 'a', 'z');
}


bool isASCIILetter(CodePoint c)
{
  return isASCIIUppercaseLetter(c) ||
         isASCIILowercaseLetter(c);
}


bool isASCIIHexDigit(CodePoint c)
{
  return isASCIIDigit(c) ||
         inRange(c, 'A', 'F') ||
         inRange(c, 'a', 'f');
}


bool isASCIIOctDigit(CodePoint c)
{
  return inRange(c, '0', '7');
}


// I looked at the POSIX standard and the Bash manual when composing
// this list, but I'm not sure I understood it all correctly.  This
// leans to the conservative side; I might be calling something meta
// that isn't, but hopefully I didn't miss any metacharacters.
bool isShellMetacharacter(CodePoint c)
{
  switch (c.valueOrN1()) {
    // Order: Going left to right then top to bottom across a US
    // Qwerty keyboard, unshifted before shifted.
    case '`':
    case '~':
    case '!':      // inverts exit status
    // not meta: @
    case '#':
    case '$':
    case '%':      // job control
    case '^':      // history substitution (?)
    case '&':
    case '*':
    case '(':
    case ')':
    // not meta: - _ +
    case '=':      // meta if appears before command
    case '[':      // character range glob
    case '{':      // alternation glob
    case ']':
    case '}':
    case '\\':
    case '|':
    case ';':
    // not meta: :
    case '"':
    case '\'':
    // not meta: ,
    case '<':
    // not meta: .
    case '>':
    // not meta: /
    case '?':
    case '\t':
    case '\n':
    case ' ':
      return true;

    default:
      return false;
  }
}


int decodeASCIIHexDigit(CodePoint c)
{
  xassertPrecondition(c.has_value());

  if (isASCIIDigit(c)) {
    return c.value() - '0';
  }
  else if (inRange(c, 'A', 'F')) {
    return c.value() - 'A' + 10;
  }
  else if (inRange(c, 'a', 'f')) {
    return c.value() - 'a' + 10;
  }
  else {
    xfailure_stringbc("bad hex digit: " << singleQuoteChar(c));
    return 0;    // Not reached.
  }
}


CodePoint decodeSurrogatePair(
  CodePoint highSurrogate, CodePoint lowSurrogate)
{
  xassert(isHighSurrogate(highSurrogate));
  xassert(isLowSurrogate(lowSurrogate));

  return 0x10000 +
         (((highSurrogate.value() & 0x3FF) << 10) |
          (lowSurrogate.value() & 0x3FF));
}


int decodeRadixIndicatorLetter(CodePoint c)
{
  switch (c.value()) {
    case 'b':
    case 'B':
      return 2;

    case 'o':
    case 'O':
      return 8;

    case 'x':
    case 'X':
      return 16;

    default:
      return 0;
  }
}


int decodeASCIIRadixDigit(CodePoint c, int radix)
{
  xassertPrecondition(2 <= radix && radix <= 36);
  xassertPrecondition(c.has_value());

  int dv = -1;
  if (inRange(c, '0', '9')) {
    dv = c.value() - '0';
  }
  else if (inRange(c, 'A', 'Z')) {
    dv = c.value() - 'A' + 10;
  }
  else if (inRange(c, 'a', 'z')) {
    dv = c.value() - 'a' + 10;
  }

  if (dv < 0 || dv >= radix) {
    return -1;
  }
  else {
    return dv;
  }
}


bool isASCIIRadixDigit(CodePoint c, int radix)
{
  if (c.has_value()) {
    return decodeASCIIRadixDigit(c, radix) >= 0;
  }
  else {
    return false;
  }
}


char encodeRadixIndicatorLetter(int radix)
{
  switch (radix) {
    case  2:     return 'b';
    case  8:     return 'o';
    case 16:     return 'x';
    default:     return 0;
  }
}


// EOF
