// gdvalue-writer-test.cc
// Tests for `gdvalue-writer` module.

// `gdvalue-test` has most of the testing for this module, but this file
// has a few more specific tests.

#include "smbase/gdvalue-writer.h"     // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <sstream>                     // std::ostringstream

using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


void test_sourceLocs()
{
  std::ostringstream oss;
  GDValueWriteOptions opts;
  opts.m_writeSourceLocations = true;
  GDValueWriter writer(oss, opts);

  GDValueSourceLocation loc1(2,3);
  GDValue v1("symbol"_sym, loc1);
  writer.write(v1);
  EXPECT_EQ(oss.str(), "/*2:3*/symbol");
}


void test_dump()
{
  GDValueSourceLocation loc1(2,3);
  GDValueSourceLocation loc2(4,5);
  GDValue seq(GDVK_SEQUENCE, loc1);
  seq.sequenceAppend(GDValue("str", loc2));

  {
    std::ostringstream oss;
    seq.dumpTo(oss);
    EXPECT_EQ(oss.str(), "/*2:3*/[/*4:5*/\"str\"]\n");
  }

  EXPECT_EQ(seq.dumpToString(), "/*2:3*/[/*4:5*/\"str\"]\n");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_writer()
{
  test_sourceLocs();
  test_dump();
}


// EOF
