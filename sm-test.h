// sm-test.h            see license.txt for copyright and terms of use
// A few test-harness macros.

#ifndef SMBASE_SM_TEST_H
#define SMBASE_SM_TEST_H

#include "dev-warning.h"   // g_abortUponDevWarning
#include "dummy-printf.h"  // dummy_printf (provided for clients)
#include "exc.h"           // smbase::XBase
#include "nonport.h"       // getMilliseconds
#include "sm-iostream.h"   // cout
#include "sm-macros.h"     // SM_PRINTF_ANNOTATION
#include "str.h"           // stringb, string
#include "xassert.h"       // xassert

#include <iomanip>         // std::hex, std::dec
#include <string_view>     // std::string_view [n]

#include <stdio.h>         // printf


// This is set, in a global initializer, to true if the "VERBOSE"
// environment variable is set.  Tests can use it to control whether
// they print extra diagnostics.
//
// This has type `int` so it can be used from C modules too.
extern int verbose;


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

// Conditionally write a line of diagnostic output.
#define DIAG(stuff)                  \
  if (verbose) {                     \
    std::cout << stuff << std::endl; \
  }


// easy way to time a section of code
class TimedSection {
  char const *name;
  long start;

public:
  TimedSection(char const *n) : name(n) {
    start = getMilliseconds();
  }
  ~TimedSection() {
    cout << name << ": " << (getMilliseconds() - start) << " msecs\n";
  }
};


// Throw an exception if `actual != expect`.
template <class T>
void expectEq(char const *label, T const &actual, T const &expect)
{
  if (expect != actual) {
    smbase::xmessage(stringb(
      "While checking " << label << ":\n"
      "  actual: " << actual << "\n"
      "  expect: " << expect));
  }
}

#define EXPECT_EQ(actual, expect) \
  expectEq(#actual, actual, expect) /* user ; */


/* Variant for use when `actual` and `expect` are numbers.  This just
   applies unary `+` to them before checking.  That causes them to be
   promoted to at least `int` if they are integral, which has two
   benefits:

   * If either is a `char` type (which `uint8_t` is), then it will
     ensure they are printed as numbers rather than characters.

   * It is more likely they will have the same type after promotion,
     reducing the need for additional casts to make the `expectEq`
     template happy.
*/
#define EXPECT_EQ_NUMBERS(actual, expect) \
  expectEq(#actual, +(actual), +(expect)) /* user ; */


// Overloads for some common cases that would otherwise either not work
// due to template argument deduction failing due to a mismatch in
// argument types, or else ambiguity in the conversions.
void expectEq(char const *label, char const *actual, char const *expect);
void expectEq(char const *label, string const &actual, char const *expect);
void expectEq(char const *label, std::string_view actual, char const *expect);


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
