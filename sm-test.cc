// sm-test.cc
// Code for sm-test.h

#include "sm-test.h"                   // this module

#include "smbase/counting-ostream.h"   // nullOStream
#include "smbase/exc.h"                // smbase::xmessage
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/string-util.h"        // doubleQuote, matchesRegex
#include "smbase/stringb.h"            // stringbc
#include "smbase/strutil.h"            // hasSubstring

#include <cstdlib>                     // std::getenv
#include <iostream>                    // std::cout

using namespace smbase;


int verbose = !!std::getenv("VERBOSE");


char const * NULLABLE g_argv0 = nullptr;


std::ostream &getTout()
{
  return verbose? std::cout : nullOStream;
}


void expectEq(char const *label, char const *actual, char const *expect)
{
  expectEq(label, std::string_view(actual), std::string_view(expect));
}


void expectHasSubstring(
  char const *label,
  string const &actual,
  char const *expectSubstring)
{
  if (!hasSubstring(actual, expectSubstring)) {
    xmessage(stringbc(
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
    xmessage(stringbc(
      "While checking " << label <<
      ": actual value is " << doubleQuote(actual) <<
      " but expected it to match regex " << doubleQuote(expectRegex) <<
      "."));
  }
}


void expectEqGDV(
  char const *label,
  gdv::GDValue const &actual,
  gdv::GDValue const &expect)
{
  if (expect != actual) {
    gdv::GDValueWriteOptions opts;
    opts.m_indentLevel = 1;
    smbase::xmessage(stringb(
      label << ": values are not equal:\n"
      "  actual: " << actual.asIndentedString(opts) << "\n"
      "  expect: " << expect.asIndentedString(opts)));
  }
}


extern "C" int dummy_printf(char const * /*fmt*/, ...)
{
  return 0;
}


// EOF
