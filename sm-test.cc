// sm-test.cc
// Code for sm-test.h

#include "sm-test.h"                   // this module

#include "smbase/chained-cond.h"       // smbase::cc::z_le_le
#include "smbase/counting-ostream.h"   // nullOStream
#include "smbase/exc.h"                // smbase::xmessage
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/nonport.h"            // getMilliseconds
#include "smbase/overflow.h"           // multiplyWithOverflowCheck
#include "smbase/pp-file-line.h"       // smbase::PreprocFileLine
#include "smbase/set-util.h"           // smbase::setInsert
#include "smbase/sm-env.h"             // smbase::envAsIntOr
#include "smbase/string-util.h"        // doubleQuote, matchesRegex
#include "smbase/stringb.h"            // stringbc
#include "smbase/strutil.h"            // hasSubstring

#include <cstdlib>                     // std::getenv
#include <iostream>                    // std::cout
#include <limits>                      // std::numeric_limits
#include <set>                         // std::set

// I tried using <cmath> to get `std::powf`, but then Clang complained
// that only `::powf` was available.
#include <math.h>                      // powf

using namespace smbase;


int verbose = envAsIntOr(0, "VERBOSE");


char const * NULLABLE g_argv0 = nullptr;


std::ostream &getTout()
{
  return verbose? std::cout : nullOStream;
}


TimedTestCase::TimedTestCase(char const *name)
:
  m_start(getMilliseconds()),
  m_name(name)
{}


TimedTestCase::~TimedTestCase()
{
  long elapsed = getMilliseconds() - m_start;
  DIAG(m_name << ": " << elapsed << " msecs");
}


void expectEq(
  smbase::PreprocFileLine loc,
  char const *label,
  char const *actual,
  char const *expect)
{
  expectEq(loc, label, std::string_view(actual), std::string_view(expect));
}


void expectHasSubstring(
  smbase::PreprocFileLine loc,
  char const *label,
  string const &actual,
  char const *expectSubstring)
{
  if (!hasSubstring(actual, expectSubstring)) {
    xmessage(stringbc(
      loc << ": While checking " << label <<
      ": actual value is " << doubleQuote(actual) <<
      " but expected it to have substring " << doubleQuote(expectSubstring) <<
      "."));
  }
}


void expectMatchesRegex(
  smbase::PreprocFileLine loc,
  char const *label,
  string const &actual,
  char const *expectRegex)
{
  if (!matchesRegex(actual, expectRegex)) {
    xmessage(stringbc(
      loc << ": While checking " << label <<
      ": actual value is " << doubleQuote(actual) <<
      " but expected it to match regex " << doubleQuote(expectRegex) <<
      "."));
  }
}


void expectEqGDV(
  smbase::PreprocFileLine loc,
  char const *label,
  gdv::GDValue const &actualGDV,
  gdv::GDValue const &expectGDV)
{
  if (expectGDV != actualGDV) {
    smbase::xmessage(stringb(
      loc << ": " << label << ": values are not equal:\n"
      "  actual: " << actualGDV.asIndentedStringLevel(1) << "\n"
      "  expect: " << expectGDV.asIndentedStringLevel(1)));
  }
}


void expectEqGDVSer_inner(
  bool origCompare,
  smbase::PreprocFileLine loc,
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
        loc << ": " <<
        label << ": although the original values compared as equal, " <<
        "the GDValues compared unequal:\n"
        "  actual: " << actualGDV.asIndentedStringLevel(1) << "\n"
        "  expect: " << expectGDV.asIndentedStringLevel(1)));
    }
  }
  else {
    if (actualGDV == expectGDV) {
      smbase::xmessage(stringb(
        loc << ": " <<
        label << ": the original values compared as unequal, " <<
        "but the GDValues were equal:\n"
        "  actual/expect: " << actualGDV.asIndentedStringLevel(1)));
    }
    else {
      smbase::xmessage(stringb(
        loc << ": " <<
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


int envRandomizedTestIters(int defaultValue, char const *name, int power)
{
  int ret = envAsIntOr(defaultValue, name);

  int multiplier = envAsIntOr(1, "RANDOMIZED_TEST_MULTIPLIER");
  if (multiplier != 1) {
    if (power != 1) {
      double const r = ret * powf(multiplier, 1.0 / power);
      double const maxIntDouble =
        static_cast<double>(std::numeric_limits<int>::max());
      xassert(cc::z_le_le<double>(r, maxIntDouble));
      ret = static_cast<int>(r);
    }
    else {
      ret = multiplyWithOverflowCheck(ret, multiplier);
    }
  }

  if (ret != defaultValue) {
    static std::set<std::string> printedNames;
    if (setInsert(printedNames, std::string(name))) {
      std::cout << "Effective " << name << " is " << ret
                << " (normally " << defaultValue << ")\n";
    }
  }

  return ret;
}


EnvRandomizedTestIters::operator int() const
{
  if (m_value < 0) {
    m_value = envRandomizedTestIters(m_defaultValue, m_name, m_power);
    xassert(m_value >= 0);
  }
  return m_value;
}


// EOF
