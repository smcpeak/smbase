// sm-test-test.cc
// Tests for `sm-test` itself.

// This file is in the public domain.

#include "smbase/sm-test.h"            // module under test; and test harness to use

#include "smbase/exc.h"                // smbase::{xmessage, XMessage}
#include "smbase/chained-cond.h"       // smbase::cc::le_le
#include "smbase/gdv-ordered-map.h"    // gdv::GDVOrderedMap (for TEST_CASE_EXPRS)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, EMEMB
#include "smbase/string-util.h"        // hasSubstring

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// ---------------- Tests sensitive to source location -----------------
// The tests in this first section are sensitive to their placement in
// the source file, hence are first to minimize disruption when making
// changes.

/* Some blank lines to facilitate adjusment if needed:









*/


// This is line 40.
void test_EXPECT_EQ_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_EQ(3, 4),
    XMessage,
    "sm-test-test.cc:43: 3: values are not equal");
}


void test_EXPECT_HAS_SUBSTRING_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_HAS_SUBSTRING("abc", "def"),
    XMessage,
    "sm-test-test.cc:51: While checking \"abc\": "
    "actual value is \"abc\" "
    "but expected it to have substring \"def\".");
}


void test_EXPECT_MATCHES_REGEX_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_MATCHES_REGEX("ghi", "jkl"),
    XMessage,
    "sm-test-test.cc:61: While checking \"ghi\": "
    "actual value is \"ghi\" "
    "but expected it to match regex \"jkl\".");
}


void test_EXPECT_EQ_GDV_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_EQ_GDV("mno", GDVSequence{"pqr"}),
    XMessage,
    "sm-test-test.cc:71: \"mno\": values are not equal:\n"
    "  actual: \"mno\"\n"
    "  expect: [\"pqr\"]");
}


void test_EXPECT_EQ_GDVSER_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_EQ_GDVSER("mno", GDVSequence{"pqr"}),
    XMessage,
    "sm-test-test.cc:81: \"mno\": values are not equal:\n"
    "  actual: \"mno\"\n"
    "  expect: [\"pqr\"]");
}


void test_EXPECT_EXN_SUBSTR_loc()
{
  EXPECT_EXN_SUBSTR(EXPECT_EXN_SUBSTR((void)1, XMessage, "blah"),
    XAssert,
    "sm-test-test.cc:91: assertion failed: Expected exception, but none was thrown.");

  EXPECT_EXN_SUBSTR(EXPECT_EXN_SUBSTR(xformat("whatever"), XUnimp, "blah"),
    XAssert,
    "sm-test-test.cc:95: assertion failed: Expected exception of type `XUnimp`");

  EXPECT_EXN_SUBSTR(EXPECT_EXN_SUBSTR(xmessage("gorf"), XMessage, "blah"),
    XAssert,
    "sm-test-test.cc:99: assertion failed: Expected exception to have \"blah\" as a substring");
}


// -------------- Tests not sensitive to source location ---------------
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


void test_TEST_FUNC_EXPRS()
{
  TEST_FUNC_EXPRS(5, "bee");

  EXPECT_EXN_SUBSTR(sampleTest_testCase(),
    XMessage,
    "test_TEST_FUNC_EXPRS: [`5`:5 `\"bee\"`:\"bee\"]: "
    "sampleTest_testCase: some error from sampleTest_testCase");
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


void test_EXPECT_TRUE()
{
  EXPECT_EXN_SUBSTR(EXPECT_TRUE(false),
    XMessage, "actual: 0");

  EXPECT_TRUE(true);
}


void test_EXPECT_FALSE()
{
  EXPECT_EXN_SUBSTR(EXPECT_FALSE(true),
    XMessage, "actual: 1");

  EXPECT_FALSE(false);
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
    xassert(hasSubstring(x.getMessage(),
      "While checking \"actual\": actual value is \"actual\" but "
      "expected it to have substring \"expectSubstring\"."));
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


// A class that exports to GDV in a way that is incompatible with its
// `operator==`, just to exercise the cases of `EXPECT_EQ_GDVSER`.
class WrongGDV {
public:      // data
  int m_n;

public:      // methods
  explicit WrongGDV(int n)
    : m_n(n)
  {}

  bool operator==(WrongGDV const &obj) const
  {
    // If both are in [30,40], claim they are equal, even though they
    // will have different GDVs.
    if (cc::le_le(30, m_n, 40) &&
        cc::le_le(30, obj.m_n, 40)) {
      return true;
    }

    return EMEMB(m_n);
  }

  operator GDValue() const
  {
    if (cc::le_le(10, m_n, 20)) {
      // Map multiple distinct `WrongGDV` to one GDV.
      return GDValue("[10,20]");
    }
    else {
      return GDValue(m_n);
    }
  }
};


struct A {
  bool operator==(A const &a) const
    { return true; }
};

struct B : A {
  // Possible ambiguity between superclass and subclass operators?
  bool operator==(B const &b) const
    { return true; }

  operator GDValue() const
    { return GDValue(); }
};


void test_EXPECT_EQ_GDVSER()
{
  EXPECT_EQ_GDVSER(1, 1);

  EXPECT_EQ_GDVSER(WrongGDV(2), WrongGDV(2));

  EXPECT_EXN_SUBSTR(
    EXPECT_EQ_GDVSER(WrongGDV(2), WrongGDV(3)),
    XMessage,
    "values are not equal");

  // Objects are equal but GDV is not.
  EXPECT_EXN_SUBSTR(
    EXPECT_EQ_GDVSER(WrongGDV(30), WrongGDV(35)),
    XMessage,
    "original values compared as equal, the GDValues compared unequal");

  // Objects are unequal but GDV is equal.
  EXPECT_EXN_SUBSTR(
    EXPECT_EQ_GDVSER(WrongGDV(10), WrongGDV(15)),
    XMessage,
    "the original values compared as unequal, but the GDValues were equal");

  // This was an unsuccessful attempt to replicate a problem from
  // elsewhere, but still useful to check.
  B b1, b2;
  EXPECT_EQ_GDVSER(b1, b2);
}


// Test `EnvRandomizedTestIters`.
EnvRandomizedTestIters const fileScopeIters{200, "FILE_SCOPE_ITERS"};

// Test it with a power other than 1.
EnvRandomizedTestIters const outerLoopIters{50, "OUTER_LOOP_ITERS", 2};
EnvRandomizedTestIters const innerLoopIters{50, "INNER_LOOP_ITERS", 2};


// This test is primarily validated manually by looking at the output in
// verbose mode.
void test_envRandomizedTestIters()
{
  TEST_CASE("test_envRandomizedTestIters");

  int fileScope = fileScopeIters;
  VPVAL(fileScope);

  int outer = outerLoopIters;
  int inner = innerLoopIters;
  VPVAL(outer);
  VPVAL(inner);

  // The idea is this should be about 50*50 (the default product) times
  // the multiplier, whereas with power=1, it would have been multiplied
  // twice.
  VPVAL(outer * inner);

  int iters = envRandomizedTestIters(100, "SM_TEST_TEST_ITERS");
  VPVAL(iters);

  // A second call should not cause a printout.
  envRandomizedTestIters(100, "SM_TEST_TEST_ITERS");
}


void test_op_eq()
{
  // Just check that it works like `operator==`.
  xassert(op_eq(1, 1) == true);
  xassert(op_eq(1, 2) == false);
}


void test_no_exn()
{
  EXPECT_EXN_SUBSTR(
    EXPECT_EXN((void)0, XAssert),
    XAssert, "Expected exception, but none was thrown.");

  EXPECT_EXN_SUBSTR(
    EXPECT_EXN_SUBSTR((void)0, XAssert, "irrelevant"),
    XAssert, "Expected exception, but none was thrown.");
}


void test_wrong_exn()
{
  EXPECT_EXN_SUBSTR(
    EXPECT_EXN(xmessage("blah"), XAssert),
    XAssert, "Expected exception of type `XAssert`, but instead "
             "got exception of type `XMessage`, with message: "
             "\"blah\".");

  EXPECT_EXN_SUBSTR(
    EXPECT_EXN_SUBSTR(xmessage("blah"), XAssert, "irrelevant"),
    XAssert, "Expected exception of type `XAssert`, but instead "
             "got exception of type `XMessage`, with message: "
             "\"blah\".");
}


void test_TEST_FUNC()
{
  // Verified manually.
  TEST_FUNC();
}


void test_TIMED_TEST_FUNC()
{
  // Verified manually.
  TIMED_TEST_FUNC();
}


void test_DIAG2()
{
  // Verified manually.
  DIAG("diagnostic level 1");
  DIAG2("diagnostic level 2");
  DIAGN(3, "diagnostic level 3");
  DIAGN(4, "diagnostic level 4");
}


CLOSE_ANONYMOUS_NAMESPACE


// Defined in sm-test-test2.cc.
void test_sm_test2();


// Called from unit-tests.cc.
void test_sm_test()
{
  test_EXPECT_EQ_loc();
  test_EXPECT_HAS_SUBSTRING_loc();
  test_EXPECT_MATCHES_REGEX_loc();
  test_EXPECT_EQ_GDV_loc();
  test_EXPECT_EQ_GDVSER_loc();
  test_EXPECT_EXN_SUBSTR_loc();

  test_TEST_CASE();
  test_TEST_CASE_EXPRS();
  test_TEST_FUNC_EXPRS();
  test_EXPECT_EQ();
  test_EXPECT_TRUE();
  test_EXPECT_FALSE();
  test_EXPECT_HAS_SUBSTRING();
  test_EXPECT_MATCHES_REGEX();
  test_EXPECT_EQ_GDV();
  test_EXPECT_EQ_GDVSER();
  test_envRandomizedTestIters();
  test_op_eq();
  test_no_exn();
  test_wrong_exn();
  test_TEST_FUNC();
  test_TIMED_TEST_FUNC();
  test_DIAG2();

  test_sm_test2();
}


// EOF
