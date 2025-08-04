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
  gdv::GDValue const &actualGDV,
  gdv::GDValue const &expectGDV)
{
  if (expectGDV != actualGDV) {
    smbase::xmessage(stringb(
      label << ": values are not equal:\n"
      "  actual: " << actualGDV.asIndentedStringLevel(1) << "\n"
      "  expect: " << expectGDV.asIndentedStringLevel(1)));
  }
}


void expectEqGDVSer_inner(
  bool origCompare,
  char const *label,
  gdv::GDValue const &actualGDV,
  gdv::GDValue const &expectGDV)
{
  if (origCompare) {
    if (actualGDV == expectGDV) {
      // Originals and GDValues are equal, as expected.
    }
    else {
      smbase::xmessage(stringb(
        label << ": although the original values compared as equal, " <<
        "the GDValues compared unequal:\n"
        "  actual: " << actualGDV.asIndentedStringLevel(1) << "\n"
        "  expect: " << expectGDV.asIndentedStringLevel(1)));
    }
  }
  else {
    if (actualGDV == expectGDV) {
      smbase::xmessage(stringb(
        label << ": the original values compared as unequal, " <<
        "but the GDValues were equal:\n"
        "  actual/expect: " << actualGDV.asIndentedStringLevel(1)));
    }
    else {
      smbase::xmessage(stringb(
        label << ": values are not equal:\n"
        "  actual: " << actualGDV.asIndentedStringLevel(1) << "\n"
        "  expect: " << expectGDV.asIndentedStringLevel(1)));
    }
  }
}


extern "C" int dummy_printf(char const * /*fmt*/, ...)
{
  return 0;
}


// EOF
