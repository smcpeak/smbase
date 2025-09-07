// gdvalue-transform-test.cc
// Tests for `gdvalue-transform` module.

#include "smbase/gdvalue-transform.h"  // module under test

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDValue v = fromGDVN(R"([
    // Symbols
    null
    true
    false
    foo

    // Other scalars
    3
    3.5
    "some string"

    // Containers
    [1 2 "three"]
    ("four" five 6)
    [7:8 nine:ten]
    {11 twelve}
    {13:14 "fifteen":16}

    // Tagged containers
    seq[1]
    tup(2)
    omap[3:4]
    set{5}
    map{6:7}
  ])");

  GDValue v2 = deepCopyGDValue(v);
  EXPECT_EQ(v2, v);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_transform()
{
  test_basics();
}


// EOF
