// parsestring.cc
// code for parsestring.h

#include "parsestring.h"               // ParseString

#include "codepoint.h"                 // isASCIIDigit
#include "overflow.h"                  // multiplyWithOverflowCheck
#include "strutil.h"                   // quoteCharacter


// ------------------------- XParseString ---------------------------
static OldSmbaseString formatCondition(OldSmbaseString const &str, int offset,
  OldSmbaseString const &conflict)
{
  return stringb("at location " << offset << " in " << quoted(str) <<
                 ": " << conflict);
}


XParseString::XParseString(OldSmbaseString const &str, int offset,
                           OldSmbaseString const &conflict)
  : xFormat(formatCondition(str, offset, conflict)),
    m_str(str),
    m_offset(offset),
    m_conflict(conflict)
{}


XParseString::XParseString(XParseString const &obj)
  : xFormat(obj),
    DMEMB(m_str),
    DMEMB(m_offset),
    DMEMB(m_conflict)
{}


XParseString::~XParseString()
{}


// ------------------------- ParseString ----------------------------
ParseString::ParseString(OldSmbaseString const &str)
  : m_str(str),
    m_len(str.length()),
    m_cur(0)
{}


ParseString::~ParseString()
{}


void ParseString::throwErr(OldSmbaseString const &conflict)
{
  throw XParseString(m_str, m_cur, conflict);
}


int ParseString::cur() const
{
  xassert(!eos());
  xassert(m_cur >= 0);
  return m_str[m_cur];
}


OldSmbaseString ParseString::quoteCur() const
{
  return quoteCharacter(cur());
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
    THROWERR("found end of string, expected " << quoteCharacter(c));
  }
  if (cur() != c) {
    THROWERR("found " << quoteCur() <<
             ", expected " << quoteCharacter(c));
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


OldSmbaseString ParseString::parseCToken()
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


OldSmbaseString ParseString::parseCDelimLiteral(int delim)
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


OldSmbaseString ParseString::parseCNumberLiteral()
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


OldSmbaseString ParseString::parseCIdentifier()
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
