// parsestring.cc
// code for parsestring.h

#include "parsestring.h"               // ParseString

#include "codepoint.h"                 // isASCIIDigit
#include "overflow.h"                  // multiplyWithOverflowCheck
#include "string-util.h"               // doubleQuote, singleQuoteChar
#include "xassert.h"                   // xassert

using namespace smbase;


// ------------------------- XParseString ---------------------------
static string formatCondition(string const &str, int offset,
  string const &conflict)
{
  return stringb("at location " << offset << " in " << doubleQuote(str) <<
                 ": " << conflict);
}


XParseString::XParseString(string const &str, int offset,
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
ParseString::ParseString(string const &str)
  : m_str(str),
    m_len(str.length()),
    m_cur(0)
{}


ParseString::~ParseString()
{}


void ParseString::throwErr(string const &conflict)
{
  throw XParseString(m_str, m_cur, conflict);
}


int ParseString::cur() const
{
  xassert(!eos());
  xassert(m_cur >= 0);
  return m_str[m_cur];
}


string ParseString::quoteCur() const
{
  return singleQuoteChar(cur());
}


void ParseString::adv()
{
  xassert(!eos());
  m_cur++;
}


void ParseString::skipWS()
{
  while (!eos() && isWhitespace(cur())) {
    adv();
  }
}


#define THROWERR(msg) throwErr(stringb(msg))


void ParseString::parseChar(int c)
{
  if (eos()) {
    THROWERR("found end of string, expected " << singleQuoteChar(c));
  }
  if (cur() != c) {
    THROWERR("found " << quoteCur() <<
             ", expected " << singleQuoteChar(c));
  }
  adv();
}


void ParseString::parseString(char const *s)
{
  while (*s) {
    parseChar(*s);
    s++;
  }
}


void ParseString::parseEOS()
{
  if (!eos()) {
    THROWERR("found " << quoteCur() <<
             ", expected end of string");
  }
}


int ParseString::parseDecimalUInt()
{
  if (!isASCIIDigit(cur())) {
    THROWERR("found " << quoteCur() <<
             ", expected digit");
  }

  int ret = 0;
  try {
    while (isASCIIDigit(cur())) {
      ret = multiplyWithOverflowCheck(ret, 10);
      ret = addWithOverflowCheck(ret, cur() - '0');
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
  int c = cur();
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
    THROWERR("found " << quoteCur() << ", expected C token");
    return ""; // Not reached.
  }
}


string ParseString::parseCDelimLiteral(int delim)
{
  stringBuilder sb;

  parseChar(delim);
  sb << (char)delim;

  while (cur() != delim) {
    sb << (char)cur();
    if (cur() == '\\') {
      // Treat the next character as not special.
      adv();
      sb << (char)cur();
    }
    adv();
  }

  parseChar(delim);
  sb << (char)delim;

  return sb.str();
}


string ParseString::parseCNumberLiteral()
{
  stringBuilder sb;

  if (cur() == '0') {
    sb << (char)cur();
    adv();

    if (!eos() && cur() == 'x') {
      sb << (char)cur();
      adv();

      while (!eos() && isASCIIHexDigit(cur())) {
        sb << (char)cur();
        adv();
      }
    }
    else {
      while (!eos() && isASCIIOctDigit(cur())) {
        sb << (char)cur();
        adv();
      }
    }
  }
  else if (isASCIIDigit(cur())) {
    sb << (char)cur();
    adv();

    while (!eos() && isASCIIDigit(cur())) {
      sb << (char)cur();
      adv();
    }
  }
  else {
    THROWERR("found " << quoteCur() << ", expected digit");
  }

  return sb.str();
}


string ParseString::parseCIdentifier()
{
  stringBuilder sb;

  if (!isCIdentifierCharacter(cur())) {
    THROWERR("found " << quoteCur() << ", expected C identifier");
  }

  while (!eos() && isCIdentifierCharacter(cur())) {
    sb << (char)cur();
    adv();
  }

  return sb.str();
}


// EOF
