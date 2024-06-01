// codepoint.cc
// code for codepoint.h

#include "codepoint.h"                 // this module

#include "str.h"                       // stringf
#include "string-utils.h"              // singleQuoteChar
#include "xassert.h"                   // xfailure_stringbc


bool isUppercaseLetter(int c)
{
  return 'A' <= c && c <= 'Z';
}


bool isLowercaseLetter(int c)
{
  return 'a' <= c && c <= 'z';
}


bool isLetter(int c)
{
  return isUppercaseLetter(c) || isLowercaseLetter(c);
}


bool isDecimalDigit(int c)
{
  return '0' <= c && c <= '9';
}


bool isWhitespace(int c)
{
  switch (c) {
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


bool isHighSurrogate(int c)
{
  return 0xD800 <= c && c < 0xDC00;
}


bool isLowSurrogate(int c)
{
  return 0xDC00 <= c && c < 0xE000;
}


bool isCIdentifierCharacter(int c)
{
  // These do *not* use the Unicode functions defined above because C
  // identifiers are restricted to code points in [0,127], whereas my
  // intent is to expand the Unicode functions to properly recognize
  // the full sets.
  return isCIdentifierStartCharacter(c) ||
          ('0' <= c && c <= '9');
}


bool isCIdentifierStartCharacter(int c)
{
  return (('A' <= c && c <= 'Z') ||
          ('a' <= c && c <= 'z') ||
          c == '_');
}


bool isASCIIPrintable(int c)
{
  return 32 <= c && c <= 126;
}


bool isASCIIDigit(int c)
{
  return '0' <= c && c <= '9';
}


bool isASCIIHexDigit(int c)
{
  return isASCIIDigit(c) ||
         ('A' <= c && c <= 'F') ||
         ('a' <= c && c <= 'f');
}


bool isASCIIOctDigit(int c)
{
  return '0' <= c && c <= '7';
}


// I looked at the POSIX standard and the Bash manual when composing
// this list, but I'm not sure I understood it all correctly.  This
// leans to the conservative side; I might be calling something meta
// that isn't, but hopefully I didn't miss any metacharacters.
bool isShellMetacharacter(int c)
{
  switch (c) {
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


int decodeASCIIHexDigit(int c)
{
  if (isASCIIDigit(c)) {
    return c - '0';
  }
  else if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  }
  else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  }
  else {
    xfailure_stringbc("bad hex digit: " << singleQuoteChar(c));
    return 0;    // Not reached.
  }
}


int decodeSurrogatePair(int highSurrogate, int lowSurrogate)
{
  xassert(isHighSurrogate(highSurrogate));
  xassert(isLowSurrogate(lowSurrogate));

  return 0x10000 +
         (((highSurrogate & 0x3FF) << 10) |
          (lowSurrogate & 0x3FF));
}


int decodeRadixIndicatorLetter(int c)
{
  switch (c) {
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


int decodeASCIIRadixDigit(int c, int radix)
{
  xassertPrecondition(2 <= radix && radix <= 36);

  int dv = -1;
  if ('0' <= c && c <= '9') {
    dv = c - '0';
  }
  else if ('A' <= c && c <= 'Z') {
    dv = c - 'A' + 10;
  }
  else if ('a' <= c && c <= 'z') {
    dv = c - 'a' + 10;
  }

  if (dv < 0 || dv >= radix) {
    return -1;
  }
  else {
    return dv;
  }
}


bool isASCIIRadixDigit(int c, int radix)
{
  return decodeASCIIRadixDigit(c, radix) >= 0;
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
