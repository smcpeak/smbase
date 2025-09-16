// gdvalue-klb-test.cc
// Tests for `gdvalue-klb` module.

#include "smbase/gdvalue-klb.h"        // module under test

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
  GDValueKindLineByte klb0(GDVK_SYMBOL);
  EXPECT_EQ(klb0.kind(), GDVK_SYMBOL);
  EXPECT_FALSE(klb0.hasLocation());
  EXPECT_TRUE(klb0 == klb0);
  EXPECT_EQ(compare(klb0, klb0), 0);
  EXPECT_EQ(klb0.asString(), "(GDVK_SYMBOL 0 0)");
  EXPECT_EQ(stringb(klb0), "(GDVK_SYMBOL 0 0)");

  GDValueKindLineByte klb1(GDVK_INTEGER, 2, 3);
  EXPECT_EQ(klb1.kind(), GDVK_INTEGER);
  EXPECT_TRUE(klb1.hasLocation());
  EXPECT_EQ(klb1.line(), 2);
  EXPECT_FALSE(klb1.lineIsSaturated());
  EXPECT_EQ(klb1.byteOffset(), 3);
  EXPECT_FALSE(klb1.byteOffsetIsSaturated());
  EXPECT_EQ(klb1.asString(), "(GDVK_INTEGER 2 3)");

  EXPECT_TRUE(klb0 != klb1);
  EXPECT_EQ(compare(klb0, klb1), -1);

  GDValueKindLineByte klb2(GDVK_SEQUENCE,
    GDValueKindLineByte::c_saturatedLineValue + 10,
    GDValueKindLineByte::c_saturatedByteOffsetValue);
  EXPECT_EQ(klb2.kind(), GDVK_SEQUENCE);
  EXPECT_TRUE(klb2.hasLocation());
  EXPECT_EQ(klb2.line(), 0xffffff);
  EXPECT_TRUE(klb2.lineIsSaturated());
  EXPECT_EQ(klb2.byteOffset(), 0xffffffffu);
  EXPECT_TRUE(klb2.byteOffsetIsSaturated());
  EXPECT_EQ(klb2.asString(), "(GDVK_SEQUENCE 16777215 4294967295)");

  EXPECT_STRICTLY_ORDERED(GDValueKindLineByte, klb0, klb1, klb2);
}


void test_order()
{
  GDValueKindLineByte klbs[] = {
    GDValueKindLineByte(GDVK_SYMBOL, 0, 0),
    GDValueKindLineByte(GDVK_SYMBOL, 1, 0),
    GDValueKindLineByte(GDVK_SYMBOL, 1, 10),
    GDValueKindLineByte(GDVK_SYMBOL, 10, 0),
    GDValueKindLineByte(GDVK_SYMBOL, 10, 10),
    GDValueKindLineByte(GDVK_INTEGER, 0, 0),
    GDValueKindLineByte(GDVK_INTEGER, 1, 0),
    GDValueKindLineByte(GDVK_INTEGER, 1, 10),
    GDValueKindLineByte(GDVK_INTEGER, 10, 0),
    GDValueKindLineByte(GDVK_INTEGER, 10, 10),
  };

  checkStrictlyOrdered("klbs", vecArrayToCRefs(klbs));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_klb()
{
  test_basics();
  test_order();
}


// EOF
