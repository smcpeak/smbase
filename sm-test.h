// sm-test.h            see license.txt for copyright and terms of use
// Various utilities for use in unit tests, especially those invoked by
// `unit-test.cc`.

#ifndef SMBASE_SM_TEST_H
#define SMBASE_SM_TEST_H

#include "dev-warning.h"   // g_abortUponDevWarning
#include "dummy-printf.h"  // dummy_printf
#include "exc.h"           // smbase::XBase
#include "sm-is-equal.h"   // smbase::is_equal
#include "sm-iostream.h"   // cout
#include "sm-macros.h"     // SM_PRINTF_ANNOTATION, NULLABLE
#include "str.h"           // stringb, string
#include "xassert.h"       // xassert

#include <iomanip>         // std::hex, std::dec
#include <iosfwd>          // std::ostream

#include <stdio.h>         // printf


// This is set, in a global initializer, to true if the "VERBOSE"
// environment variable is set.  Tests can use it to control whether
// they print extra diagnostics.
//
// This has type `int` so it can be used from C modules too.
extern int verbose;


// The value of `argv[0]`.  This is set by `main` in unit-tests.cc, and
// null until that function runs.
extern char const * NULLABLE g_argv0;


// "Test output", which goes nowhere unless `verbose` is true.
#define tout getTout()
std::ostream &getTout();


// "Test printf", which goes nowhere unless `verbose` is true.
#define tprintf (verbose? printf : dummy_printf)


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
    (void)stuff;                                            \
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


// Check that 'hasSubstring(actual, expectSubstring)'.
void expectHasSubstring(
  char const *label,
  string const &actual,
  char const *expectSubstring);

#define EXPECT_HAS_SUBSTRING(actual, expectSubstring) \
  expectHasSubstring(#actual, actual, expectSubstring) /* user ; */


// Check that 'matchesRegex(actual, expectRegex)'.
void expectMatchesRegex(
  char const *label,
  string const &actual,
  char const *expectRegex);

#define EXPECT_MATCHES_REGEX(actual, expectRegex) \
  expectMatchesRegex(#actual, actual, expectRegex) /* user ; */


#endif // SMBASE_SM_TEST_H
