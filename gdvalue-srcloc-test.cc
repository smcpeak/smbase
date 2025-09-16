// gdvalue-srcloc-test.cc
// Tests for `gdvalue-srcloc` module.

#include "smbase/gdvalue-srcloc.h"     // module under test

#include "smbase/compare-util.h"       // smbase::compare [h]
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_{EQ,TRUE,FALSE}
#include "smbase/sm-test-order.h"      // EXPECT_STRICTLY_ORDERED, checkStrictlyOrdered
#include "smbase/stringb.h"            // stringb
#include "smbase/vector-util.h"        // vecArrayToCRefs

#include <string>                      // std::string


using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDValueSourceLocation klb0(1, 1);
  EXPECT_EQ(klb0.line(), 1);
  EXPECT_FALSE(klb0.lineIsSaturated());
  EXPECT_EQ(klb0.column(), 1);
  EXPECT_FALSE(klb0.columnIsSaturated());
  EXPECT_EQ(klb0.asString(), "1:1");
  EXPECT_EQ(stringb(klb0), "1:1");

  EXPECT_TRUE(klb0 == klb0);
  EXPECT_EQ(compare(klb0, klb0), 0);

  GDValueSourceLocation klb1(2, 3);
  EXPECT_EQ(klb1.line(), 2);
  EXPECT_FALSE(klb1.lineIsSaturated());
  EXPECT_EQ(klb1.column(), 3);
  EXPECT_FALSE(klb1.columnIsSaturated());
  EXPECT_EQ(klb1.asString(), "2:3");

  EXPECT_TRUE(klb0 != klb1);
  EXPECT_EQ(compare(klb0, klb1), -1);

  GDValueSourceLocation klb2(
    GDValueSourceLocation::c_saturatedLineValue + 10,
    GDValueSourceLocation::c_saturatedColumnValue);
  EXPECT_EQ(klb2.line(), 0xffffff);
  EXPECT_TRUE(klb2.lineIsSaturated());
  EXPECT_EQ(klb2.column(), 0xffffffffu);
  EXPECT_TRUE(klb2.columnIsSaturated());
  EXPECT_EQ(klb2.asString(), "16777215:4294967295");

  EXPECT_STRICTLY_ORDERED(GDValueSourceLocation, klb0, klb1, klb2);

  GDValueSourceLocation k(klb1);
  EXPECT_EQ(k, klb1);
  EXPECT_EQ(k.line(), 2);
  EXPECT_EQ(k.column(), 3);

  // Self-assign, obfuscated to avoid warning.
  GDValueSourceLocation const &cr = k;
  k = cr;
  EXPECT_EQ(k, klb1);

  k = klb2;
  EXPECT_EQ(k, klb2);
  EXPECT_EQ(k.line(), 0xffffff);
  EXPECT_EQ(k.column(), 0xffffffffu);
}


void test_order()
{
  GDValueSourceLocation klbs[] = {
    GDValueSourceLocation(1, 1),
    GDValueSourceLocation(1, 10),
    GDValueSourceLocation(10, 1),
    GDValueSourceLocation(10, 10),
  };

  checkStrictlyOrdered("klbs", vecArrayToCRefs(klbs));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_srcloc()
{
  test_basics();
  test_order();
}


// EOF
