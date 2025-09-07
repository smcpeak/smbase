// gdvalue-subst-transform-test.cc
// Tests for `gdvalue-subst-transform` module.

#include "smbase/gdvalue-subst-transform.h"      // module under test

#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

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

  GDValue v2 = substitutionTransformGDValue(v,
    std::map<GDValue, GDValue>{
      { 3, 333 },
      { fromGDVN("tup(2)"), fromGDVN("t(u(p(3)))") },
    },
    false /*recursive*/,
    std::map<GDVSymbol, GDVSymbol>{
      { "seq"_sym, "SEQ"_sym },
    }
  );

  GDValue v2expect = fromGDVN(R"([
    // Symbols
    null
    true
    false
    foo

    // Other scalars
    333
    3.5
    "some string"

    // Containers
    [1 2 "three"]
    ("four" five 6)
    [7:8 nine:ten]
    {11 twelve}
    {13:14 "fifteen":16}

    // Tagged containers
    SEQ[1]
    t(u(p(3)))      // Note: No recursive substitution of "3".
    omap[333:4]
    set{5}
    map{6:7}
  ])");

  EXPECT_EQ(v2, v2expect);

  // Do the same thing with recursive substitution.
  v2 = substitutionTransformGDValue(v,
    std::map<GDValue, GDValue>{
      { 3, 333 },
      { fromGDVN("tup(2)"), fromGDVN("t(u(p(3)))") },
    },
    true /*recursive*/,
    std::map<GDVSymbol, GDVSymbol>{
      { "seq"_sym, "SEQ"_sym },
    }
  );

  v2expect = fromGDVN(R"([
    // Symbols
    null
    true
    false
    foo

    // Other scalars
    333
    3.5
    "some string"

    // Containers
    [1 2 "three"]
    ("four" five 6)
    [7:8 nine:ten]
    {11 twelve}
    {13:14 "fifteen":16}

    // Tagged containers
    SEQ[1]
    t(u(p(333)))      // Recursive substitution of "3".
    omap[333:4]
    set{5}
    map{6:7}
  ])");

  EXPECT_EQ(v2, v2expect);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_subst_transform()
{
  test_basics();
}


// EOF
