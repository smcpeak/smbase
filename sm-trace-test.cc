// sm-trace-test.cc
// Tests for `sm-trace` module.

// This file is in the public domain.

#include "smbase/sm-trace.h"           // module under test

#include "smbase/gdvalue-map.h"        // gdv::toGDValue(std::map)
#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/gdvalue.h"            // GDVN_OMAP_EXPRS [h]
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // verbose, DIAG

#include <map>                         // std::map
#include <vector>                      // std::vector

#include <assert.h>                    // assert.h

using namespace gdv;

using std::cout;


// Allow exercising tracing in this module.
INIT_TRACE("sm-trace");


OPEN_ANONYMOUS_NAMESPACE


static void expectLevel(
  int expectLevel,
  std::string const &scope,
  std::string const &spec)
{
  std::string errorRE;
  std::string errorMsg;
  int actualLevel = innerGetTraceLevel(
    scope,
    spec,
    errorRE,
    errorMsg);

  if (actualLevel != expectLevel) {
    cout << "expectLevel: " << expectLevel << "\n"
         << "actualLevel: " << actualLevel << "\n"
         << "scope: " << scope << "\n"
         << "spec: " << spec << "\n";
    cout.flush();
  }

  assert(actualLevel == expectLevel);
  assert(errorRE.empty());
  assert(errorMsg.empty());
}


static void expectError(
  std::string const &spec,
  std::string const &expectErrorRE)
{
  std::string actualErrorRE;
  std::string errorMsg;
  int actualLevel = innerGetTraceLevel(
    "someScope",
    spec,
    actualErrorRE,
    errorMsg);

  assert(actualLevel == -1);
  assert(actualErrorRE == expectErrorRE);

  // There's not a lot of value here in checking the code, so I just
  // print the details.
  DIAG("Got expected error:\n"
    << "  spec: " << spec << "\n"
    << "  errorMsg: " << errorMsg);
}


class SomeClass {
  DECLARE_CLASS_TRACE_VARS(SomeClass);

public:
  static void foo();
};

DEFINE_CLASS_TRACE_VARS(SomeClass);

void SomeClass::foo()
{
  TRACE1("hello from SomeClass");
}


void test_basics()
{
  // Enabled.
  expectLevel(1, "someMod", "someMod");

  // Not enabled.
  expectLevel(0, "someMod", "otherMod");

  // Enabled with a different level than 1.
  expectLevel(2, "someMod", "someMod=2");

  // Enabled along with something else.
  expectLevel(1, "someMod", "otherMod,someMod");

  // Enabled using a substring.
  expectLevel(1, "someMod", "otherMod,some");

  // Enabled along with an empty spec.
  expectLevel(1, "someMod", "someMod,");

  // Disabled along with an empty spec.
  expectLevel(0, "someMod", "otherMod,");

  // Enabled with a broad regex.
  expectLevel(1, "someMod", ".");

  // Enabled with a narrow regex.
  expectLevel(1, "someMod", "[sS]ome[mM]o?d");

  // ?
  expectLevel(1, "SomeClass", "Some");

  // Some erroneous regexes.
  expectError("(", "(");

  // Curiously, for this example, Clang-16 libc++ yields an error
  // message of just "regex_error", whereas it's supposed to be a string
  // that describes std::error_badrepeat.
  expectError("*", "*");

  // Check that we recognize the part that is erroneous.
  expectError("x,(", "(");

  // Code that has actual tracing flags.  The output is seen iff the
  // caller actually sets TRACE.
  SomeClass sc;
  sc.foo();

  // Test SCOPED tracing.
  INIT_TRACE("trace_unit_tests");
  {
    TRACE1_SCOPED("start of scoped section");
    TRACE1("inside scoped section");
  }
  TRACE1("after scoped section");
}


void test_TRACEn_EXPRS()
{
  std::map<int, std::string> m1{
    { 1, "one string" },
    { 2, "two string" },
    { 3, "three string" },
  };

  std::vector<std::vector<std::string>> v1{
    { "some", "strings", "for", "the", "first", "vector" },
    { "some", "strings", "for", "the", "second", "vector" },
  };

  if (verbose) {
    TRACE0_GDVN_EXPRS("lvl0", m1, v1);
  }

  TRACE1_GDVN_EXPRS("lvl1", m1, v1);
  TRACE2_GDVN_EXPRS("lvl2", m1, v1);
  TRACE3_GDVN_EXPRS("lvl3", m1, v1);
  TRACE4_GDVN_EXPRS("lvl4", m1, v1);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_trace()
{
  test_basics();
  test_TRACEn_EXPRS();
}


// EOF
