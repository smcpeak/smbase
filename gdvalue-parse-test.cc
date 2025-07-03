// gdvalue-parse-test.cc
// Code for `gdvalue-parse`.

#include "smbase/gdvalue-parse.h"      // module under test

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <limits>                      // std::numeric_limits

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_gdvTo_int()
{
  EXPECT_EQ(gdvTo<int>(GDValue(3)), 3);

  if (sizeof(int) < sizeof(GDVSmallInteger)) {
    // Too big.
    GDVSmallInteger maxGSI = std::numeric_limits<GDVSmallInteger>::max();
    EXPECT_EXN(gdvTo<int>(GDValue(maxGSI)), XFormat);
  }

  // Not an integer.
  EXPECT_EXN(gdvTo<int>(GDValue()), XFormat);
}


void test_gdvTo_string()
{
  EXPECT_EQ(gdvTo<std::string>(GDValue("abc")), "abc");

  EXPECT_EXN(gdvTo<std::string>(GDValue(GDVSymbol("abc"))), XFormat);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_gdvalue_parse()
{
  test_gdvTo_int();
  test_gdvTo_string();
}


// EOF
