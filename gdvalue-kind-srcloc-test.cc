// gdvalue-kind-srcloc-test.cc
// Tests for `gdvalue-kind-srcloc` module.

#include "smbase/gdvalue-kind-srcloc.h"          // module under test

#include "smbase/compare-util.h"                 // smbase::compare [h]
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_{EQ,TRUE,FALSE}
#include "smbase/sm-test-order.h"                // EXPECT_STRICTLY_ORDERED, checkStrictlyOrdered
#include "smbase/stringb.h"                      // stringb
#include "smbase/vector-util.h"                  // vecArrayToCRefs

#include <string>                                // std::string


using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDValueKindSourceLocation klb0(GDVK_SYMBOL);
  EXPECT_EQ(klb0.getKind(), GDVK_SYMBOL);
  EXPECT_FALSE(klb0.hasSourceLocation());
  EXPECT_TRUE(klb0 == klb0);
  EXPECT_EQ(compare(klb0, klb0), 0);
  EXPECT_EQ(klb0.asString(), "GDVK_SYMBOL at noloc");
  EXPECT_EQ(stringb(klb0), "GDVK_SYMBOL at noloc");

  GDValueKindSourceLocation klb1(GDVK_INTEGER, {2, 3});
  EXPECT_EQ(klb1.getKind(), GDVK_INTEGER);
  EXPECT_TRUE(klb1.hasSourceLocation());
  EXPECT_EQ(klb1.sourceLocation().line(), 2);
  EXPECT_FALSE(klb1.sourceLocation().lineIsSaturated());
  EXPECT_EQ(klb1.sourceLocation().column(), 3);
  EXPECT_FALSE(klb1.sourceLocation().columnIsSaturated());
  EXPECT_EQ(klb1.asString(), "GDVK_INTEGER at 2:3");

  EXPECT_TRUE(klb0 != klb1);
  EXPECT_EQ(compare(klb0, klb1), -1);

  GDValueKindSourceLocation klb2(GDVK_SEQUENCE,
    { GDValueKindSourceLocation::c_saturatedLineValue + 10,
      GDValueKindSourceLocation::c_saturatedColumnValue });
  EXPECT_EQ(klb2.getKind(), GDVK_SEQUENCE);
  EXPECT_TRUE(klb2.hasSourceLocation());
  EXPECT_EQ(klb2.sourceLocation().line(), 0xffffff);
  EXPECT_TRUE(klb2.sourceLocation().lineIsSaturated());
  EXPECT_EQ(klb2.sourceLocation().column(), 0xffffffffu);
  EXPECT_TRUE(klb2.sourceLocation().columnIsSaturated());
  EXPECT_EQ(klb2.asString(), "GDVK_SEQUENCE at 16777215:4294967295");

  EXPECT_STRICTLY_ORDERED(GDValueKindSourceLocation, klb0, klb1, klb2);

  GDValueKindSourceLocation k(klb1);
  EXPECT_EQ(k, klb1);
  EXPECT_EQ(k.getKind(), GDVK_INTEGER);
  EXPECT_EQ(k.sourceLocation().line(), 2);
  EXPECT_EQ(k.sourceLocation().column(), 3);

  // Self-assign, obfuscated to avoid warning.
  GDValueKindSourceLocation const &cr = k;
  k = cr;
  EXPECT_EQ(k, klb1);

  k = klb2;
  EXPECT_EQ(k, klb2);
  EXPECT_EQ(k.getKind(), GDVK_SEQUENCE);
  EXPECT_EQ(k.sourceLocation().line(), 0xffffff);
  EXPECT_EQ(k.sourceLocation().column(), 0xffffffffu);
}


void test_order()
{
  GDValueKindSourceLocation klbs[] = {
    GDValueKindSourceLocation(GDVK_SYMBOL),
    GDValueKindSourceLocation(GDVK_SYMBOL, { 1, 1 }),
    GDValueKindSourceLocation(GDVK_SYMBOL, { 1, 10 }),
    GDValueKindSourceLocation(GDVK_SYMBOL, { 10, 1 }),
    GDValueKindSourceLocation(GDVK_SYMBOL, { 10, 10 }),
    GDValueKindSourceLocation(GDVK_INTEGER),
    GDValueKindSourceLocation(GDVK_INTEGER, { 1, 1 }),
    GDValueKindSourceLocation(GDVK_INTEGER, { 1, 10 }),
    GDValueKindSourceLocation(GDVK_INTEGER, { 10, 1 }),
    GDValueKindSourceLocation(GDVK_INTEGER, { 10, 10 }),
  };

  checkStrictlyOrdered("klbs", vecArrayToCRefs(klbs));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_kind_srcloc()
{
  test_basics();
  test_order();
}


// EOF
