// parsestring.cc
// code for parsestring.h

#include "smbase/parsestring.h"        // this module

#include "smbase/codepoint.h"          // isASCIIDigit
#include "smbase/overflow.h"           // multiplyWithOverflowCheck
#include "smbase/string-util.h"        // doubleQuote, singleQuoteChar
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert

#include <utility>                     // std::move

using namespace smbase;

using std::string;


// ------------------------- XParseString ---------------------------
static string formatCondition(
  string const &str,
  std::size_t offset,
  string const &conflict)
{
  return stringb("at location " << offset << " in " << doubleQuote(str) <<
                 ": " << conflict);
}


XParseString::XParseString(
  string const &str,
  std::size_t offset,
  string const &conflict)
  : XFormat(formatCondition(str, offset, conflict)),
    m_str(str),
    m_offset(offset),
    m_conflict(conflict)
{}


XParseString::XParseString(XParseString const &obj)
  : XFormat(obj),
    DMEMB(m_str),
    DMEMB(m_offset),
    DMEMB(m_conflict)
{}


XParseString::~XParseString()
{}


// ------------------------- ParseString ----------------------------
ParseString::~ParseString()
{}


ParseString::ParseString(string const &str)
  : m_str(str),
    m_curOffset(0)
{}


ParseString::ParseString(string &&str)
  : m_str(std::move(str)),
    m_curOffset(0)
{}


void ParseString::throwErr(string const &conflict)
{
  throw XParseString(m_str, m_curOffset, conflict);
}


int ParseString::curByte() const
{
  xassert(!eos());
  xassert(m_curOffset >= 0);

  // The cast ensures the result is non-negative.
  return static_cast<unsigned char>(m_str[m_curOffset]);
}


char ParseString::curByteAsChar() const
{
  return static_cast<char>(curByte());
}


string ParseString::quoteCurByte() const
{
  return singleQuoteChar(curByte());
}


void ParseString::adv()
{
  xassert(!eos());
  ++m_curOffset;
}


void ParseString::skipWS()
{
  while (!eos() && isWhitespace(curByte())) {
    adv();
  }
}


#define THROWERR(msg) throwErr(stringb(msg))


void ParseString::parseByte(int c)
{
  if (eos()) {
    THROWERR("found end of string, expected " << singleQuoteChar(c));
  }
  if (curByte() != c) {
    THROWERR("found " << quoteCurByte() <<
             ", expected " << singleQuoteChar(c));
  }
  adv();
}


void ParseString::parseString(char const *s)
{
  while (*s) {
    parseByte(*s);
    s++;
  }
}


void ParseString::parseEOS()
{
  if (!eos()) {
    THROWERR("found " << quoteCurByte() <<
             ", expected end of string");
  }
}


int ParseString::parseDecimalUInt()
{
  if (!isASCIIDigit(curByte())) {
    THROWERR("found " << quoteCurByte() <<
             ", expected digit");
  }

  int ret = 0;
  try {
    while (isASCIIDigit(curByte())) {
      ret = multiplyWithOverflowCheck(ret, 10);
      ret = addWithOverflowCheck(ret, curByte() - '0');
      adv();
    }
  }
  catch (XOverflow &) {
    THROWERR("integer is too large to represent");
  }

  return ret;
}


string ParseString::parseCToken()
{
  int c = curByte();
  if (c == '"') {
    return parseCDelimLiteral(c);
  }
  else if (c == '\'') {
    return parseCDelimLiteral(c);
  }
  else if (isASCIIDigit(c)) {
    return parseCNumberLiteral();
  }
  else if (isCIdentifierCharacter(c)) {
    return parseCIdentifier();
  }
  else {
    THROWERR("found " << quoteCurByte() << ", expected C token");
    return ""; // Not reached.
  }
}


string ParseString::parseCDelimLiteral(int delim)
{
  stringBuilder sb;

  parseByte(delim);
  sb << (char)delim;

  while (curByte() != delim) {
    sb << curByteAsChar();
    if (curByte() == '\\') {
      // Treat the next character as not special.
      adv();
      sb << curByteAsChar();
    }
    adv();
  }

  parseByte(delim);
  sb << (char)delim;

  return sb.str();
}


string ParseString::parseCNumberLiteral()
{
  stringBuilder sb;

  if (curByte() == '0') {
    sb << curByteAsChar();
    adv();

    if (!eos() && curByte() == 'x') {
      sb << curByteAsChar();
      adv();

      while (!eos() && isASCIIHexDigit(curByte())) {
        sb << curByteAsChar();
        adv();
      }
    }
    else {
      while (!eos() && isASCIIOctDigit(curByte())) {
        sb << curByteAsChar();
        adv();
      }
    }
  }
  else if (isASCIIDigit(curByte())) {
    sb << curByteAsChar();
    adv();

    while (!eos() && isASCIIDigit(curByte())) {
      sb << curByteAsChar();
      adv();
    }
  }
  else {
    THROWERR("found " << quoteCurByte() << ", expected digit");
  }

  return sb.str();
}


string ParseString::parseCIdentifier()
{
  stringBuilder sb;

  if (!isCIdentifierCharacter(curByte())) {
    THROWERR("found " << quoteCurByte() << ", expected C identifier");
  }

  while (!eos() && isCIdentifierCharacter(curByte())) {
    sb << curByteAsChar();
    adv();
  }

  return sb.str();
}


std::string ParseString::getUpToByte(int c)
{
  std::ostringstream res;

  while (!eos()) {
    res << curByteAsChar();

    // We found, and included, `c`.
    if (curByte() == c) {
      adv();
      break;
    }

    adv();
  }

  return res.str();
}


std::string ParseString::getUpToSize(std::size_t size)
{
  xassert(m_curOffset <= m_str.size());

  if (m_curOffset + size >= m_str.size()) {
    // Return entire remainder.
    std::string ret(m_str.substr(m_curOffset, m_str.size() - m_curOffset));
    m_curOffset = m_str.size();
    return ret;
  }

  else {
    // Get the next `size` bytes, but some will remain.
    std::string ret(m_str.substr(m_curOffset, size));
    m_curOffset += size;
    return ret;
  }
}


// EOF
