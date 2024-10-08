// sm-trace.cc
// Code for `sm-trace` module.

// This file is in the public domain.

#include "sm-trace.h"                  // this module

#include "smbase/sm-regex.h"           // smbase::{Regex, MatchResults, ...}

// libc++
#include <cstdlib>                     // std::{getenv, atoi}
#include <string>                      // std::string

using namespace smbase;

using std::cout;
using std::cerr;
using std::string;


int getTraceLevel(char const *scope)
{
  char const *spec = std::getenv("TRACE");
  if (!spec) {
    return 0;
  }

  string errorRE;
  string errorMsg;
  int ret = innerGetTraceLevel(scope, spec,
                               errorRE /*OUT*/, errorMsg /*OUT*/);

  if (ret < 0) {
    // Treat this as a fatal error, at least for now.
    cerr << "envvar TRACE contains invalid regex:\n"
            "  " << errorRE << "\n"
            "msg: " << errorMsg << "\n";
    std::exit(2);
  }

  return ret;
}


// Core of 'getTraceLevel'.  On error, this returns -1 and sets
// 'errorRE' and 'errorMsg'.
int innerGetTraceLevel(
  std::string const &scope,
  std::string const &spec,
  std::string /*OUT*/ &errorRE,
  std::string /*OUT*/ &errorMsg)
{
  // Allow tracing this function by modifying the hardcoded
  // 'traceLevel'.
  char const * const traceScope = "getTraceLevel";
  int const traceLevel = 0;

  // The element (itself a regex) is a sequence of characters that are
  // not commas or equals, optionally followed by a numeric level.
  Regex elementRE("([^,=]+)(?:=(\\d+))?");

  // Level we will return.  This is increased when we find an element
  // that matches the scope.
  int retLevel = 0;

  // Extract elements.
  for (MatchResults m : MatchResultsIterable(spec, elementRE)) {
    string elt = m.str(1);
    string levelStr = m.str(2);
    TRACE1_EXPR(elt);
    TRACE1_EXPR(levelStr);

    int level = (levelStr.empty()? 1 : std::atoi(levelStr.c_str()));

    try {
      Regex eltUserRE(elt);

      if (level > retLevel &&
          eltUserRE.searchB(scope)) {
        retLevel = level;
      }
    }
    catch (XRegexSyntaxError &e) {
      errorRE = elt;
      errorMsg = e.what();
      return -1;
    }
  }

  return retLevel;
}


int g_traceIndentationLevel = 0;


ScopedTraceIndentationLevel::ScopedTraceIndentationLevel(bool enabled)
  : m_enabled(enabled)
{
  if (m_enabled) {
    ++g_traceIndentationLevel;
  }
}

ScopedTraceIndentationLevel::~ScopedTraceIndentationLevel()
{
  if (m_enabled) {
    --g_traceIndentationLevel;
  }
}


std::ostream *g_traceOutputStream = nullptr;


std::ostream &beginTraceOutput(char const *traceScope)
{
  return beginTraceOutput(traceScope, ": ");
}


std::ostream &beginTraceOutput(char const *traceScope, char const *suffix)
{
  std::ostream *os =
    g_traceOutputStream? g_traceOutputStream : &std::clog;

  *os << "### ";
  for (int i=0; i < g_traceIndentationLevel; ++i) {
    *os << "  ";
  }
  *os << traceScope << suffix;
  return *os;
}


// EOF
