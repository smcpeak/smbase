// sm-test-test.cc
// Tests for `sm-test` itself.

// This file is in the public domain.

#include "smbase/sm-test.h"            // module under test; and test harness to use

#include "smbase/exc.h"                // xmessage, smbase::XMessage
#include "smbase/gdv-ordered-map.h"    // gdv::GDVOrderedMap (for TEST_CASE_EXPRS)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void sampleTest_testCase()
{
  TEST_CASE("sampleTest_testCase");
  xmessage("some error from sampleTest_testCase");
}

void test_TEST_CASE()
{
  // Make sure the context is right.
  EXPECT_EXN_SUBSTR(sampleTest_testCase(),
    XMessage,
    "sampleTest_testCase: some error from sampleTest_testCase");
}


void sampleTest_testCaseExprs(int a, std::string b)
{
  TEST_CASE_EXPRS("sampleTest_testCaseExprs", a, b);
  xmessage("some error from sampleTest_testCaseExprs");
}

void test_TEST_CASE_EXPRS()
{
  EXPECT_EXN_SUBSTR(sampleTest_testCaseExprs(5, "bee"),
    XMessage,
    "sampleTest_testCaseExprs: [a:5 b:\"bee\"]: "
    "some error from sampleTest_testCaseExprs");
}


void sampleTest_expectEq()
{
  EXPECT_EQ(3, 4);
}

void test_EXPECT_EQ()
{
  EXPECT_EXN_SUBSTR(sampleTest_expectEq(),
    XMessage,
    "3: values are not equal:\n"
    "  actual: 3\n"
    "  expect: 4");

  EXPECT_EQ(5, 5);
}


void sampleTest_expectHasSubstring()
{
  EXPECT_HAS_SUBSTRING("actual", "expectSubstring");
}

void test_EXPECT_HAS_SUBSTRING()
{
  try {
    sampleTest_expectHasSubstring();
  }
  catch (XMessage &x) {
    DIAG(x.getMessage());

    // Ordinarily, I would use TEST_EXN_SUBSTR, but that is the thing I
    // am trying to test, so instead use a lower-level test.
    xassert(x.getMessage() ==
      "While checking \"actual\": actual value is \"actual\" but "
      "expected it to have substring \"expectSubstring\".");
  }

  EXPECT_HAS_SUBSTRING("actual", "ctua");
}


void sampleTest_expectMatchesRegex()
{
  EXPECT_MATCHES_REGEX("actual", "expectRegex");
}

void test_EXPECT_MATCHES_REGEX()
{
  EXPECT_EXN_SUBSTR(sampleTest_expectMatchesRegex(),
    XMessage,
    "While checking \"actual\": actual value is \"actual\" but "
    "expected it to match regex \"expectRegex\".");

  EXPECT_MATCHES_REGEX("actual", "c[tu]{2}a");
}


void test_EXPECT_EQ_GDV()
{
  EXPECT_EQ_GDV(1, 1);
  EXPECT_EQ_GDV(GDVSet{3}, GDVSet{3});

  EXPECT_EXN_SUBSTR(EXPECT_EQ_GDV(1, 2),
    XMessage,
    "1: values are not equal:\n  actual: 1\n  expect: 2");

  EXPECT_EXN_SUBSTR(EXPECT_EQ_GDV(GDVSet{3}, (GDVSet{4,5})),
    XMessage,
    "GDVSet{3}: values are not equal:\n  actual: {3}\n  expect: {4 5}");

  // Make sure the indentation applied to the GDValues meshes properly
  // with the framing message.
  EXPECT_EXN_SUBSTR(
    EXPECT_EQ_GDV(
      (GDVSequence{
        "a long string to ensure the line has to be wrapped here1",
        "a long string to ensure the line has to be wrapped here2",
        "a long string to ensure the line has to be wrapped here3"}),
      (GDVMap{
        { 4, "a long string to ensure the line has to be wrapped here4"},
        { 5, "a long string to ensure the line has to be wrapped here5"},
      })),
    XMessage,
    "not equal:\n"
    "  actual: [\n"
    "    \"a long string to ensure the line has to be wrapped here1\"\n"
    "    \"a long string to ensure the line has to be wrapped here2\"\n"
    "    \"a long string to ensure the line has to be wrapped here3\"\n  ]\n"
    "  expect: {\n"
    "    4: \"a long string to ensure the line has to be wrapped here4\"\n"
    "    5: \"a long string to ensure the line has to be wrapped here5\"\n"
    "  }");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_test()
{
  test_TEST_CASE();
  test_TEST_CASE_EXPRS();
  test_EXPECT_EQ();
  test_EXPECT_HAS_SUBSTRING();
  test_EXPECT_MATCHES_REGEX();
  test_EXPECT_EQ_GDV();
}


// EOF
