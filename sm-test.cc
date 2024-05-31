// sm-test.cc
// Code for sm-test.h

#include "sm-test.h"                   // this module

#include "stringb.h"                   // stringbc
#include "strutil.h"                   // hasSubstring
#include "string-utils.h"              // doubleQuote, matchesRegex
#include "xassert.h"                   // xfailure

#include <cstdlib>                     // std::getenv


int verbose = !!std::getenv("VERBOSE");


void expectEq(char const *label, string const &actual, char const *expect)
{
  expectEq(label, actual, string(expect));
}


void expectEq(char const *label, std::string_view actual, char const *expect)
{
  expectEq(label, std::string(actual), expect);
}


void expectHasSubstring(
  char const *label,
  string const &actual,
  char const *expectSubstring)
{
  if (!hasSubstring(actual, expectSubstring)) {
    xfailure(stringbc(
      "While checking " << label <<
      ": actual value is " << doubleQuote(actual) <<
      " but expected it to have substring " << doubleQuote(expectSubstring) <<
      "."));
  }
}


void expectMatchesRegex(
  char const *label,
  string const &actual,
  char const *expectRegex)
{
  if (!matchesRegex(actual, expectRegex)) {
    xfailure(stringbc(
      "While checking " << label <<
      ": actual value is " << doubleQuote(actual) <<
      " but expected it to match regex " << doubleQuote(expectRegex) <<
      "."));
  }
}


extern "C" int dummy_printf(char const * /*fmt*/, ...)
{
  return 0;
}


// EOF
