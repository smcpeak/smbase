// sm-regex-test.cc
// Tests for sm-regex.

#include "sm-regex.h"                  // module under test

#include "exc.h"                       // EXN_CONTEXT
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "string-util.h"               // doubleQuote

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void matchVector(char const *str, char const *exp, bool expect)
{
  EXN_CONTEXT("str=" << doubleQuote(str));
  EXN_CONTEXT("exp=" << doubleQuote(exp));

  EXPECT_EQ(Regex(exp).search(str), expect);
}


void testMatchVectors()
{
  matchVector("abc", "a", true);
  matchVector("abc", "b", true);
  matchVector("abc", "c", true);
  matchVector("abc", "d", false);

  matchVector("abc", "^a", true);
  matchVector("abc", "^b", false);
  matchVector("abc", "b$", false);
  matchVector("abc", "c$", true);
  matchVector("abc", "^d", false);
}


void testInvalidRegex()
{
  try {
    Regex r("(");
    xfailure("should have failed");
  }
  catch (XRegexSyntaxError &x) {
    VPVAL(x.what());
    EXPECT_HAS_SUBSTRING(x.what(), "syntax error");
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called by unit-tests.cc.
void test_sm_regex()
{
  testMatchVectors();
  testInvalidRegex();

  // The tests here are not very thorough in part because there are
  // additional regex tests in `string-util-test`.
}


// EOF
