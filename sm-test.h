// sm-test.h            see license.txt for copyright and terms of use
// Various utilities for use in unit tests, especially those invoked by
// `unit-test.cc`.

#ifndef SMBASE_SM_TEST_H
#define SMBASE_SM_TEST_H

#include "smbase/dev-warning.h"        // g_abortUponDevWarning
#include "smbase/dummy-printf.h"       // dummy_printf
#include "smbase/exc.h"                // smbase::XBase
#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-iostream.h"        // cout
#include "smbase/sm-is-equal.h"        // smbase::is_equal
#include "smbase/sm-macros.h"          // SM_PRINTF_ANNOTATION, NULLABLE
#include "smbase/sm-pp-util.h"         // SM_PP_MAP
#include "smbase/str.h"                // string
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert, xfailure

#include <cstring>                     // std::strstr
#include <iomanip>                     // std::hex, std::dec
#include <iosfwd>                      // std::ostream

#include <stdio.h>                     // printf


// This is set, in a global initializer, to true if the "VERBOSE"
// environment variable is set.  Tests can use it to control whether
// they print extra diagnostics.
//
// This has type `int` so it can be used from C modules too.
extern int verbose;


// The value of `argv[0]`.  This is set by `main` in unit-tests.cc, and
// null until that function runs.
extern char const * NULLABLE g_argv0;


// ---------------------------- Entry point ----------------------------
// reports uncaught exceptions
//
// 12/30/02: I used to print "uncaught exception: " before
// printing the exception, but this is meaningless to the
// user and the message usually has enough info anyway
#define USUAL_MAIN                              \
void entry();                                   \
int main()                                      \
{                                               \
  try {                                         \
    entry();                                    \
    return 0;                                   \
  }                                             \
  catch (smbase::XBase &x) {                    \
    cout << x << endl;                          \
    return 4;                                   \
  }                                             \
}

// same as above, but with command-line args
#define ARGS_MAIN                               \
void entry(int argc, char *argv[]);             \
int main(int argc, char *argv[])                \
{                                               \
  try {                                         \
    entry(argc, argv);                          \
    return 0;                                   \
  }                                             \
  catch (smbase::XBase &x) {                    \
    cout << x << endl;                          \
    return 4;                                   \
  }                                             \
}


// Like USUAL_MAIN but meant for use in unit tests.
#define USUAL_TEST_MAIN                         \
  int main()                                    \
  {                                             \
    g_abortUponDevWarning = true;               \
    try {                                       \
      entry();                                  \
      return 0;                                 \
    }                                           \
    catch (smbase::XBase &x) {                  \
      cout << x << endl;                        \
      return 4;                                 \
    }                                           \
  }

// Like ARGS_MAIN but for use in unit tests.
#define ARGS_TEST_MAIN                          \
  int main(int argc, char *argv[])              \
  {                                             \
    g_abortUponDevWarning = true;               \
    try {                                       \
      entry(argc, argv);                        \
      return 0;                                 \
    }                                           \
    catch (smbase::XBase &x) {                  \
      cout << x << endl;                        \
      return 4;                                 \
    }                                           \
  }


// ------------------------- Diagnostic output -------------------------
// "Test output", which goes nowhere unless `verbose` is true.
#define tout getTout()
std::ostream &getTout();


// "Test printf", which goes nowhere unless `verbose` is true.
#define tprintf (verbose? printf : dummy_printf)


// convenient for printing the value of a variable or expression
#define PVAL(val) cout << #val << " = " << (val) << endl

// Same, but also print the file and line.
#define DEBUG_PVAL(val) cout << __FILE__ << ":" << __LINE__ << \
  ": " #val << " = " << (val) << endl

// As hexadecimal.
#define PVAL_HEX(val) \
  cout << #val << " = " << "0x" << std::hex << (val) << std::dec << endl

// Print a value if `verbose`.
#define VPVAL(stuff)                                        \
  if (verbose) {                                            \
    PVAL(stuff);                                            \
  }                                                         \
  else {                                                    \
    /* Evaluate it to ensure no crash, but do not print. */ \
    (void)(stuff);                                          \
  }

// PVAL with a specified output stream.
#define PVALTO(os, val) (os) << #val ": " << (val) << std::endl /* user ; */

// Conditionally write a line of diagnostic output.
#define DIAG(stuff)                  \
  if (verbose) {                     \
    std::cout << stuff << std::endl; \
  }


// 2024-06-01: There was a class called `TimedSection` here but I
// removed it because it did not belong in this file and was not being
// used.


// ----------------------------- EXPECT_EQ -----------------------------
// Throw an exception if `actual` does not equal `expect`.  This uses
// `is_equal` to deal with the possibility that exactly one of the types
// is a signed integral type.  According to that function, a negative
// number is not equal to any non-negative number.
template <typename TA, typename TE>
void expectEq(char const *label, TA const &actual, TE const &expect)
{
  if (!smbase::is_equal(expect, actual)) {
    smbase::xmessage(stringb(
      label << ": values are not equal:\n"
      "  actual: " << actual << "\n"
      "  expect: " << expect));
  }
}

#define EXPECT_EQ(actual, expect) \
  expectEq(#actual, actual, expect) /* user ; */


/* Variant for use when `actual` and `expect` are numbers.  This just
   applies unary `+` to them before checking.  That causes them to be
   promoted to at least `int` if they are integral, which ensures that
   they will be printed as numbers even if one or both have a type based
   on `char` (such as `uint8_t`).
*/
#define EXPECT_EQ_NUMBERS(actual, expect) \
  expectEq(#actual, +(actual), +(expect)) /* user ; */


// Overload for the `char*` case to ensure we compare string contents
// rather than addresses.  Both arguments must be non-null.
void expectEq(char const *label, char const *actual, char const *expect);


// ----------------------- EXPECT_HAS_SUBSTRING ------------------------
// Check that 'hasSubstring(actual, expectSubstring)'.
void expectHasSubstring(
  char const *label,
  string const &actual,
  char const *expectSubstring);

#define EXPECT_HAS_SUBSTRING(actual, expectSubstring) \
  expectHasSubstring(#actual, actual, expectSubstring) /* user ; */


// ----------------------- EXPECT_MATCHES_REGEX ------------------------
// Check that 'matchesRegex(actual, expectRegex)'.
void expectMatchesRegex(
  char const *label,
  string const &actual,
  char const *expectRegex);

#define EXPECT_MATCHES_REGEX(actual, expectRegex) \
  expectMatchesRegex(#actual, actual, expectRegex) /* user ; */


// --------------------------- EXPECT_EQ_GDV ---------------------------
// Check that `actual` equals `expect`.
void expectEqGDV(
  char const *label,
  gdv::GDValue const &actual,
  gdv::GDValue const &expect);

// Convert both `actual` and `expect` to `GDValue` before comparing.
// The main advantage is we can always serialize `GDValue` in the error
// message, whereas the original types might not be easily serializable.
//
// Using this macro often requires including additional headers to get
// the right `toGDValue`.  And it is intentional that `toGDValue` is not
// qualified because we want to allow argument-dependent lookup.
#define EXPECT_EQ_GDV(actual, expect) \
  expectEqGDV(#actual, toGDValue(actual), toGDValue(expect)) /* user ; */


// ------------------------- EXPECT_EQ_GDVSER --------------------------
// If `origCompare`, check that `actualGDV==expectGDV` and return.  If
// the latter do not match, throw XMessage.  If `origCompare==false`,
// throw XMessage due to the originals being different.
void expectEqGDVSer_inner(
  bool origCompare,
  char const *label,
  gdv::GDValue const &actualGDV,
  gdv::GDValue const &expectGDV);

// Check that `actualOrig==expectOrig`, but using `actualGDV` and
// `expectGDV` for the error message rather than trying to serialize the
// originals.
template <typename TA, typename TE>
void expectEqGDVSer(
  char const *label,
  TA const &actualOrig,
  TE const &expectOrig,
  gdv::GDValue const &actualGDV,
  gdv::GDValue const &expectGDV)
{
  expectEqGDVSer_inner(
    smbase::is_equal(expectOrig, actualOrig),
    label, actualGDV, expectGDV);
}

// Check that `actual==expect`, but use GDV for serialization for the
// error message.  We serialize unconditionally both so we can check
// that GDV equality agrees and so that the requirement to have a
// suitable `toGDValue` in scope is imposed at the call site, not here
// where the function template is defined.
#define EXPECT_EQ_GDVSER(actual, expect)                            \
  expectEqGDVSer(#actual,                                           \
                 actual, expect,                                    \
                 toGDValue(actual), toGDValue(expect)) /* user ; */


// ------------------------- EXPECT_TRUE/FALSE -------------------------
// ChatGPT likes to generate tests that use these.
#define EXPECT_TRUE(expr) EXPECT_EQ((expr), true)
#define EXPECT_FALSE(expr) EXPECT_EQ((expr), false)


// ---------------------------- EXPECT_EXN -----------------------------
// Common core of `EXPECT_EXN` and `EXPECT_EXN_SUBSTR`
#define EXPECT_EXN_COMMON_CORE(expr, ExnType, additionalCheck)    \
  {                                                               \
    bool evalFinished = false;                                    \
    try {                                                         \
      expr;                                                       \
      evalFinished = true;                                        \
    }                                                             \
    catch (ExnType &e) {                                          \
      char const *w = e.what();                                   \
      additionalCheck                                             \
      if (verbose) {                                              \
        cout << "As expected: " << w << "\n";                     \
      }                                                           \
    }                                                             \
    catch (std::exception &e) {                                   \
      xfailure_stringbc(                                          \
        "Expected exception of type `" #ExnType "`, but instead " \
        "got exception of type `" << getExceptionTypeName(e) <<   \
        "`, with message: " << doubleQuote(e.what()) << ".");     \
    }                                                             \
    if (evalFinished) {                                           \
      x_assert_fail("Expected exception, but none was thrown.",   \
        __FILE__, __LINE__);                                      \
    }                                                             \
  }


// Check that evaluating `expr` throws an exception of type `ExnType`.
#define EXPECT_EXN(expr, ExnType)                    \
  EXPECT_EXN_COMMON_CORE(expr, ExnType, /*nothing*/)


// Check that evaluating `expr` throws an exception of type `ExnType`
// whose `what` string contains `substring`.
#define EXPECT_EXN_SUBSTR(expr, ExnType, substring)                \
  EXPECT_EXN_COMMON_CORE(expr, ExnType,                            \
    if (!std::strstr(w, (substring))) {                            \
      xfailure_stringbc(                                           \
        "Expected exception to have " << doubleQuote(substring) << \
        " as a substring of its what string " << doubleQuote(w) << \
        " but it did not.");                                       \
    })


// ------------------------ Comparison testing -------------------------
// Test `a==b`, but also verify that the result is the same if the
// order is swapped, and that it is consistent with `a!=b`.
template <typename T>
bool op_eq(T const &a, T const &b)
{
  bool ret = (a == b);

  xassert(ret == (b == a));

  xassert(ret != (a != b));
  xassert(ret != (b != a));

  return ret;
}


// ----------------------------- TEST_CASE -----------------------------
/*
  Print `stuff` in verbose mode, and push it onto the exception context
  stack.
*/
#define TEST_CASE(stuff) \
  DIAG(stuff);           \
  EXN_CONTEXT(stuff) /* user ; */


/*
  Print/context each of several argument expressions.

  Use it like:

    TEST_CASE_EXPRS("someFunction", expr1, expr2, expr3)

  which will print (in verbose mode) and add as context a string that
  includes the names and values of all expressions.

  To use this macro, you have to #include "gdvalue.h" and possibly other
  headers that know how to convert various types to `GDValue`.
*/
#define TEST_CASE_EXPRS(label, ...) \
  TEST_CASE(label ": " << GDVN_OMAP_EXPRS(__VA_ARGS__)) /* user ; */


// -------------------- Randomized testing support ---------------------
// If `name` is set as an environment variable, return its value as
// interpreted by `atoi`, otherwise return `defaultValue`.
//
// But, independently, if "RANDOMIZED_TEST_MULTIPLIER" is set, then
// multiply the previously specified value by the result of `atoi`
// applied to the latter before returning.
//
// Finally, if `power` is not 1, then the multiplier is adjusted by
// taking its `power`th root, which is appropriate when the result will
// be used as the number of iterations in a nested loop with nesting
// equal to `power`, and we want the multiplier to have the overall
// effect of linearly increasing the time spent testing.
//
// The idea is to provide a single envvar that can be used to linearly
// scale up all randomized testing, which is useful after making a major
// change.
//
// If the return value is not `defaultValue`, print to stdout the value
// we will use, thus providing feedback on the effectiveness of the
// envvar setting.  But only print this once (per process) so as not to
// spam the output if it is queried multiple time.
int envRandomizedTestIters(
  int defaultValue, char const *name, int power = 1);


/* Allow a call to the above function to be prepared in advance (such as
   at file scope) but its activation delayed until actually needed in
   some function.  In particular, it will not print anything if it is at
   file scope in one test but we only run some other test.

   Use it like:

     EnvRandomizedTestIters const numIters{2000, "RANDOM_TEST_ITERS"};

   and then use `numIters` when needed.
*/
struct EnvRandomizedTestIters {
  // Arguments to the function.
  int m_defaultValue;
  char const *m_name;
  int m_power = 1;

  // Memoized value.
  mutable int m_value = -1;

  // Equivalent to
  // `envRandomizedTestIters(m_defaultValue, m_name, power)`.
  operator int() const;
};


#endif // SMBASE_SM_TEST_H
