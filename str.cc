// str.cc            see license.txt for copyright and terms of use
// code for str.h
// Scott McPeak, 1995-2000  This file is public domain.

#include "str.h"            // this module

#include "flatten.h"        // Flatten
#include "sm-iostream.h"    // ostream << char*
#include "xassert.h"        // xassert

#include <algorithm>        // std::max

#include <assert.h>         // assert
#include <stdlib.h>         // atoi
#include <stdio.h>          // sprintf
#include <ctype.h>          // isspace
#include <string.h>         // strcmp
#include <assert.h>         // assert


// ----------------------- OldSmbaseString ---------------------
// put the empty string itself in read-only memory
static char const nul_byte = 0;

// deliberately cast away the constness; I cannot declare
// 'emptyString' to be const because it gets assigned to 's', but it
// is nevertheless the intent that I never modify 'nul_byte'
char * const OldSmbaseString::emptyString = const_cast<char*>(&nul_byte);


OldSmbaseString::OldSmbaseString(char const *src, int length, SmbaseStringFunc)
{
  s=emptyString;
  setlength(length);       // setlength already has the +1; sets final NUL
  memcpy(s, src, length);
}


// This is the same code as the above constructor.
OldSmbaseString::OldSmbaseString(char const *src, int length)
{
  s=emptyString;
  setlength(length);
  memcpy(s, src, length);
}


void OldSmbaseString::dup(char const *src)
{
  // std::string does not accept NULL pointers
  xassert(src != NULL);

  if (src[0]==0) {
    s = emptyString;
  }
  else {
    s = new char[ strlen(src) + 1 ];
    xassert(s);
    strcpy(s, src);
  }
}

void OldSmbaseString::kill()
{
  if (s != emptyString) {
    delete[] s;         // found by Coverity Prevent
  }
}


OldSmbaseString::OldSmbaseString(Flatten&)
  : s(emptyString)
{}

void OldSmbaseString::xfer(Flatten &flat)
{
  flat.xferCharString(s);
}


int OldSmbaseString::length() const
{
  xassert(s);
  return strlen(s);
}

bool OldSmbaseString::contains(char c) const
{
  xassert(s);
  return !!strchr(s, c);
}


OldSmbaseString OldSmbaseString::substring(int startIndex, int len) const
{
  xassert(startIndex >= 0 &&
          len >= 0 &&
          startIndex + len <= length());

  return ::substring(s+startIndex, len);
}


OldSmbaseString::OldSmbaseString(std::string const &src)
  : s(emptyString)
{
  dup(src.c_str());
}


OldSmbaseString &OldSmbaseString::setlength(int length)
{
  kill();
  if (length > 0) {
    s = new char[ length+1 ];
    xassert(s);
    s[length] = 0;      // final NUL in expectation of 'length' chars
    s[0] = 0;           // in case we just wanted to set allocated length
  }
  else {
    xassert(length == 0);     // negative wouldn't make sense
    s = emptyString;
  }
  return *this;
}


int OldSmbaseString::compareTo(OldSmbaseString const &src) const
{
  return compareTo(src.s);
}

int OldSmbaseString::compareTo(char const *src) const
{
  if (src == NULL) {
    src = emptyString;
  }
  return strcmp(s, src);
}


OldSmbaseString OldSmbaseString::operator+(OldSmbaseString const &tail) const
{
  OldSmbaseString dest(length() + tail.length(), SMBASE_STRING_FUNC);
  strcpy(dest.s, s);
  strcat(dest.s, tail.s);
  return dest;
}

OldSmbaseString& OldSmbaseString::operator+=(OldSmbaseString const &tail)
{
  return *this = *this + tail;
}


void OldSmbaseString::readdelim(istream &is, char const *delim)
{
  stringBuilder sb;
  sb.readdelim(is, delim);
  operator= (sb);
}


void OldSmbaseString::write(ostream &os) const
{
  os << s;     // standard char* writing routine
}


void OldSmbaseString::selfCheck() const
{}


// -------------------------- compatibility ----------------------------
void stringXfer(std::string &str, Flatten &flat)
{
  // For serialization compatibility with OldSmbaseString::xfer,
  // read and write using 'xferCharString', even though that causes an
  // extra allocation when reading.

  if (flat.reading()) {
    char *p = nullptr;
    flat.xferCharString(p);

    // This causes an extra allocation.
    str = p;

    delete[] p;
  }

  else {
    char const *p = str.c_str();

    // 'xferCharString' will not modifiy the contents of the string in
    // reading mode.
    char *q = const_cast<char*>(p);

    flat.xferCharString(q);
  }
}


bool stringEquals(std::string const &a, char const *b)
{
  return a == b;
}

bool stringEquals(std::string const &a, std::string const &b)
{
  return a == b;
}


// ----------------------- rostring ---------------------
int strcmp(rostring s1, rostring s2)
  { return strcmp(s1.c_str(), s2.c_str()); }
int strcmp(rostring s1, char const *s2)
  { return strcmp(s1.c_str(), s2); }
int strcmp(char const *s1, rostring s2)
  { return strcmp(s1, s2.c_str()); }


char const *strstr(rostring haystack, char const *needle)
{
  return strstr(haystack.c_str(), needle);
}


int atoi(rostring s)
{
  return atoi(toCStr(s));
}

string substring(char const *p, int n)
{
  return string(p, n);
}


// --------------------- stringBuilder ------------------
stringBuilder::stringBuilder(int len)
{
  init(len);
}

void stringBuilder::init(int initSize)
{
  size = initSize + EXTRA_SPACE + 1;     // +1 to be like OldSmbaseString::setlength
  s = new char[size];
  end = s;
  end[initSize] = 0;
}


void stringBuilder::dup(char const *str)
{
  int len = strlen(str);
  init(len);
  strcpy(s, str);
  end += len;
}


stringBuilder::stringBuilder(char const *str)
{
  dup(str);
}


stringBuilder::stringBuilder(char const *str, int len)
{
  init(len);
  memcpy(s, str, len);
  end += len;
}


stringBuilder& stringBuilder::operator=(char const *src)
{
#if 1
  // quarl 2006-06-01
  //    This implementation avoids re-allocation unless necessary.
  if (s != src) {
    int srclen = strlen(src)+1;
    if (srclen > size) { // need to re-allocate?
      delete s;
      s = new char[ srclen ];
    }
    xassert(s);
    memcpy(s, src, srclen); // copy string including NULL
    end = s + srclen - 1; // point to NULL
  }
  return *this;
#else
  if (s != src) {
    kill();
    dup(src);
  }
  return *this;
#endif
}


stringBuilder& stringBuilder::setlength(int newlen)
{
  kill();
  init(newlen);
  return *this;
}


void stringBuilder::adjustend(char* newend)
{
  xassert(s <= newend  &&  newend < s + size);

  end = newend;
  *end = 0;        // sm 9/29/00: maintain invariant
}


void stringBuilder::truncate(int newLength)
{
  xassert(0 <= newLength && newLength <= length());
  adjustend(s + newLength);
}


stringBuilder& stringBuilder::operator+= (char const *tail)
{
  append(tail, strlen(tail));
  return *this;
}

void stringBuilder::append(char const *tail, int len)
{
  ensure(length() + len);

  memcpy(end, tail, len);
  end += len;
  *end = 0;
}


stringBuilder& stringBuilder::indent(int amt)
{
  xassert(amt >= 0);
  ensure(length() + amt);

  memset(end, ' ', amt);
  end += amt;
  *end = 0;

  return *this;
}


void stringBuilder::grow(int newMinLength)
{
  // I want at least EXTRA_SPACE extra
  int newMinSize = newMinLength + EXTRA_SPACE + 1;         // compute resulting allocated size

  // I want to grow at the rate of at least 50% each time
  int suggest = size * 3 / 2;

  // see which is bigger
  newMinSize = std::max(newMinSize, suggest);

  // remember old length..
  int len = length();

  // realloc s to be newMinSize bytes
  char *temp = new char[newMinSize];
  assert(len+1 <= newMinSize);     // prevent overrun
  memcpy(temp, s, len+1);          // copy null too
  delete[] s;
  s = temp;

  // adjust other variables
  end = s + len;
  size = newMinSize;
}


stringBuilder& stringBuilder::operator<< (char c)
{
  ensure(length() + 1);
  *(end++) = c;
  *end = 0;
  return *this;
}


#define MAKE_LSHIFT(Argtype, fmt)                        \
  stringBuilder& stringBuilder::operator<< (Argtype arg) \
  {                                                      \
    char buf[60];      /* big enough for all types */    \
    int len = sprintf(buf, fmt, arg);                    \
    if (len >= 60) {                                     \
      abort();    /* too big */                          \
    }                                                    \
    return *this << buf;                                 \
  }

MAKE_LSHIFT(long long, "%lld")
MAKE_LSHIFT(unsigned long long, "%llu")
MAKE_LSHIFT(long, "%ld")
MAKE_LSHIFT(unsigned long, "%lu")
MAKE_LSHIFT(double, "%g")

#undef MAKE_LSHIFT


stringBuilder& stringBuilder::operator<< (void* arg)
{
  char buf[60];

#ifdef __MINGW32__
  // The MSVCRT %p specifier is annoying because it prints all of the
  // leading zeroes.
  int len = sprintf(buf, "0x%llx", (unsigned long long)arg);
#else
  int len = sprintf(buf, "%p", arg);
#endif

  if (len >= 60) { abort(); }
  return *this << buf;
}


stringBuilder& stringBuilder::operator<< (
  stringBuilder::Hex const &h)
{
  char buf[32];        // should only need 19 for 64-bit word..
  int len = sprintf(buf, "0x%lX", h.value);
  if (len >= 20) {
    abort();
  }
  return *this << buf;

  // the length check above isn't perfect because we only find out there is
  // a problem *after* trashing the environment.  it is for this reason I
  // use 'assert' instead of 'xassert' -- the former calls abort(), while the
  // latter throws an exception in anticipation of recoverability
}


stringBuilder& stringBuilder::operator<< (Manipulator manip)
{
  return manip(*this);
}


string stringBuilder::str() const
{
  return OldSmbaseString::operator std::string ();
}


// slow but reliable
void stringBuilder::readdelim(istream &is, char const *delim)
{
  char c;
  is.get(c);
  while (!is.eof() &&
         (!delim || !strchr(delim, c))) {
    *this << c;
    is.get(c);
  }
}


// ---------------------- toString ---------------------
#define TOSTRING(type)      \
  string toString(type val) \
  {                         \
    return stringb(val);    \
  }

TOSTRING(int)
TOSTRING(unsigned)
TOSTRING(char)
TOSTRING(long)
TOSTRING(float)

#undef TOSTRING

// this one is more liberal than 'stringc << null' because it gets
// used by the PRINT_GENERIC macro in my astgen tool
string toString(char const *str)
{
  if (!str) {
    return string("(null)");
  }
  else {
    return string(str);
  }
}


// EOF
