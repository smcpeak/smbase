// codepoint.cc
// code for codepoint.h

#include "codepoint.h"                 // this module

#include "str.h"                       // stringf


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


bool isCIdentifierCharacter(int c)
{
  // These do *not* use the Unicode functions defined above because C
  // identifiers are restricted to code points in [0,127], whereas my
  // intent is to expand the Unicode functions to properly recognize
  // the full sets.
  return (('A' <= c && c <= 'Z') ||
          ('a' <= c && c <= 'z') ||
          ('0' <= c && c <= '9') ||
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


// EOF
