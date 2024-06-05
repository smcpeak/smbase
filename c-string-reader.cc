// c-string-reader.cc
// Code for `c-string-reader` module.

#include "c-string-reader.h"           // this module

#include "codepoint.h"                 // isASCIIHexDigit, decodeASCIIHexDigit, decodeASCIIOctDigit
#include "exc.h"                       // xformat
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "string-util.h"               // singleQuoteChar

#include <sstream>                     // std::{istringstream, ostringstream}


OPEN_NAMESPACE(smbase)


// --------------------------- CStringReader ---------------------------
int CStringReader::readEscapeSequence()
{
  // Read the character after the backslash.
  int c = readNotEOFCharOrErr(
    "looking for next character after backslash at start of escape sequence");

  switch (c) {
    case 'a':    return '\a';
    case 'b':    return '\b';
    case 'f':    return '\f';
    case 'n':    return '\n';
    case 'r':    return '\r';
    case 't':    return '\t';
    case 'v':    return '\v';
    case '\\':   return '\\';
    case '?':    return '?';
    case '"':    return '"';
    case '\'':   return '\'';

    case 'x':
      // Hex escape.
      return decodeHexOrOctal(true /*hex*/);

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      // Octal escape.
      putback(c);
      return decodeHexOrOctal(false /*hex*/);

    default:
      // Other values are implementation-defined in C/C++, and in
      // practice compilers seem to treat "\c" the same as "c".
      return c;
  }
}


int CStringReader::decodeHexOrOctal(bool hex)
{
  // The first digit can safely be treated as hex since, even if octal,
  // the intepretation is the same and no error is possible.
  int c = readChar();
  if (!isASCIIHexDigit(c)) {
    unexpectedCharErr(c,
      "looking for a hexadecimal digit after \"\\x\"");
  }
  int decoded = decodeASCIIHexDigit(c);

  // Count the digits so we can limit ourselves to three octal digits.
  // There is no analogous limit for hex digits.
  int numDigitsRead = 1;

  while (!( hex==false && numDigitsRead>=3 )) {
    c = readChar();
    if (!( hex? isASCIIHexDigit(c) : isASCIIOctDigit(c) )) {
      // Stop when we run out of appropriate digits.
      putback(c);
      break;
    }

    // We can use `decodeASCIIHexDigit` for both radices.
    decoded = decoded*(hex?16:8) + decodeASCIIHexDigit(c);

    if (decoded > 0x10FFFF) {
      if (allowTooLargeCodePoints()) {
        // Clamp it.
        decoded = 0x10FFFF;
      }
      else {
        err(hex? "Hex escape sequence denotes value larger than 0x10FFFF." :
                 "Octal escape sequence denotes value larger than 0x10FFFF.");
      }
    }

    ++numDigitsRead;
  }

  return decoded;
}


void CStringReader::unquotedDelimErr()
{
  err(stringb(
    "unescaped delimiter (" << singleQuoteChar(m_delim) << ")"
  ));
}


void CStringReader::unquotedNewlineErr()
{
  err("unescaped newline (unterminated string)");
}


CStringReader::~CStringReader()
{}


CStringReader::CStringReader(
  std::istream &is,
  char delim,
  CStringReaderFlags flags)
  : Reader(is),
    m_delim(delim),
    m_flags(flags)
{}


// --------------------------- global funcs ----------------------------
void decodeCStringEscapesToStream(
  std::ostream &os,
  std::string const &str,
  char delim,
  CStringReaderFlags flags)
{
  std::istringstream iss(str);
  CStringReader reader(iss, delim, flags);

  int c;
  while (c = reader.readCodePoint(), c >= 0) {
    // TODO: At some point I should properly encode `c` as UTF-8.  But
    // for now I am just doing what the old `decodeEscapes` code did.
    os << (char)(unsigned char)c;
  }
}


std::string decodeCStringEscapesToString(
  std::string const &str,
  char delim,
  CStringReaderFlags flags)
{
  std::ostringstream oss;
  decodeCStringEscapesToStream(oss, str, delim, flags);
  return oss.str();
}


std::string parseQuotedCString(
  std::string const &text,
  char delim,
  CStringReaderFlags flags)
{
  if (!( text.size() >= 2 &&
         text[0] == delim &&
         text[text.size()-1] == delim )) {
    xformat(stringb("quoted string is missing quotes: " << text));
  }

  // Strip the quotes.
  std::string noQuotes = text.substr(1, text.size()-2);

  // Decode escapes.
  return decodeCStringEscapesToString(noQuotes, delim, flags);
}


CLOSE_NAMESPACE(smbase)


// EOF
