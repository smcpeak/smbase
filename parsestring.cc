// parsestring.cc
// code for parsestring.h

#include "parsestring.h"               // ParseString

#include "codepoint.h"                 // isASCIIDigit
#include "overflow.h"                  // multiplyWithOverflowCheck
#include "strutil.h"                   // quoteCharacter


// ------------------------- XParseString ---------------------------
static string formatCondition(string const &str, int offset,
  string const &conflict)
{
  return stringb("at location " << offset << " in " << quoted(str) <<
                 ": " << conflict);
}


XParseString::XParseString(string const &str, int offset,
                           string const &conflict)
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
  return quoteCharacter(cur());
}


void ParseString::adv()
{
  xassert(!eos());
  m_cur++;
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


// EOF
