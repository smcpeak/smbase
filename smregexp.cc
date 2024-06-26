// smregexp.cc
// code for smregexp.h

#include "smregexp.h"     // this module
#include "str.h"          // string
#include "exc.h"          // smbase::xmessage
#include "array.h"        // Array

#include <stddef.h>       // size_t

using namespace smbase;


// The entire module does not work on Windows.
#ifndef __WIN32__


// for now, I implement everything using the libc POSIX regex
// facilities
//
// linux (etc.) has proper declarations in regex.h, but FreeBSD (and
// other BSDs?) has regex.h contents that do not compile under C++,
// and apparently gnuregex.h is the substitute that does
#ifndef __FreeBSD__
  #include <regex.h>
#else
  #include <gnuregex.h>
#endif


bool smregexpModuleWorks()
{
  return true;
}


// get an error string
static string regexpErrorString(regex_t const *pat, int code)
{
  // find out how long the error string is; this size
  // includes the final NUL byte
  int size = regerror(code, pat, NULL, 0);

  // get the string
  Array<char> buf(size);
  regerror(code, pat, buf.ptr(), size);
  buf[size] = 0;     // paranoia

  return string(buf.ptrC());
}

// throw an exception
static void regexpError(regex_t const *pat, int code) NORETURN;
static void regexpError(regex_t const *pat, int code)
{
  xmessage(regexpErrorString(pat, code));
}


// -------------------- Regexp --------------------------
// interpretation of 'impl' field
#define PAT ((regex_t*&)impl)

Regexp::Regexp(rostring exp, CFlags flags)
{
  PAT = new regex_t;

  int f = REG_EXTENDED;    // "extended" language

  // if the values I chose line up perfectly with the values used by
  // libc, then I don't have to interpret them (hopefully the
  // optimizer will discover that the 'if' test is constant
  // (gcc-2.95.3's optimizer does); I can't do it with the
  // preprocessor because it can't see the enumerator values)
  if (REG_ICASE==ICASE && REG_NOSUB==NOSUB) {
    f |= (int)flags;
  }
  else {
    // interpret my flags
    if (flags & ICASE) f |= REG_ICASE;
    if (flags & NOSUB) f |= REG_NOSUB;
  }

  int code = regcomp(PAT, toCStr(exp), f);
  if (code) {
    // deallocate the pattern buffer before throwing the exception
    string msg = regexpErrorString(PAT, code);
    delete PAT;
    xmessage(msg);
  }
}

Regexp::~Regexp()
{
  regfree(PAT);
  delete PAT;
}


void Regexp::err(int code)
{
  regexpError(PAT, code);
}


bool Regexp::match(rostring str, EFlags flags)
{
  int f = 0;

  // same thing as above
  if (REG_NOTBOL==NOTBOL && REG_NOTEOL==NOTEOL) {
    f = (int)flags;
  }
  else {
    if (flags & NOTBOL) f |= REG_NOTBOL;
    if (flags & NOTEOL) f |= REG_NOTEOL;
  }

  int code = regexec(PAT, toCStr(str), 0, NULL, f);
  if (code == 0) {
    return true;
  }
  else if (code == REG_NOMATCH) {
    return false;
  }
  else {
    err(code);
  }
}


#undef PAT


// --------------- convenience functions ---------------
bool regexpMatch(rostring str, rostring exp)
{
  Regexp pat(exp, Regexp::NOSUB);
  return pat.match(str);
}


#else // windows

bool smregexpModuleWorks()
{
  return false;
}


// Stubs.
Regexp::Regexp(rostring exp, CFlags flags)
{}

Regexp::~Regexp()
{}

bool Regexp::match(rostring str, EFlags flags)
{
  return false;
}

bool regexpMatch(rostring str, rostring exp)
{
  return false;
}


#endif // windows


// EOF
