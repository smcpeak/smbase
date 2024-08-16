// sm-regex-test.cc
// Tests for sm-regex.

#include "sm-regex.h"                  // module under test

// Must come before sm-test.h.
#include "smbase/vector-util.h"        // operator<<(vector)

#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/string-util.h"        // doubleQuote

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void matchVector(char const *str, char const *exp, bool expect)
{
  EXN_CONTEXT("str=" << doubleQuote(str));
  EXN_CONTEXT("exp=" << doubleQuote(exp));

  EXPECT_EQ(Regex(exp).searchB(str), expect);
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


void testSearchMR()
{
  Regex r("a(b)c(d+)e");

  MatchResults mr = r.searchMR("xabcddey");
  EXPECT_EQ(mr.empty(), false);
  EXPECT_EQ(mr.succeeded(), true);
  EXPECT_EQ((bool)mr, true);
  EXPECT_EQ(mr.size(), 3);
  EXPECT_EQ(mr.str(0), "abcdde");
  EXPECT_EQ(mr.str(1), "b");
  EXPECT_EQ(mr.str(2), "dd");
  EXPECT_EQ(mr.asVector(),
            (std::vector<std::string>{ "abcdde", "b", "dd" }));

  mr = r.searchMR("xabcey");
  EXPECT_EQ(mr.empty(), true);
  EXPECT_EQ(mr.succeeded(), false);
  EXPECT_EQ((bool)mr, false);
  EXPECT_EQ(mr.size(), 0);
  EXPECT_EQ(mr.asVector(),
            (std::vector<std::string>{}));
}


void testMatchResultsIterator()
{
  Regex re("a(b)c(d+)e");

  MatchResultsIterator end;

  {
    MatchResultsIterator it("xabcddey abcddde abcdddde", re);
    xassert(it != end);
    xassert(!(it == end));
    EXPECT_EQ((*it).asVector(),
              (std::vector<std::string>{"abcdde", "b", "dd"}));

    ++it;
    xassert(it != end);
    EXPECT_EQ((*it).asVector(),
              (std::vector<std::string>{"abcddde", "b", "ddd"}));

    ++it;
    xassert(it != end);
    EXPECT_EQ((*it).asVector(),
              (std::vector<std::string>{"abcdddde", "b", "dddd"}));

    ++it;
    xassert(it == end);
    xassert(!(it != end));
  }

  {
    MatchResultsIterator it("abc", re);
    xassert(it == end);
    xassert(!(it != end));
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called by unit-tests.cc.
void test_sm_regex()
{
  testMatchVectors();
  testInvalidRegex();
  testSearchMR();
  testMatchResultsIterator();

  // The tests here are not very thorough in part because there are
  // additional regex tests in `string-util-test`.
}


// EOF
