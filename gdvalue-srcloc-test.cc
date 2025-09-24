// gdvalue-srcloc-test.cc
// Tests for `gdvalue-srcloc` module.

#include "smbase/gdvalue-srcloc.h"     // module under test

#include "smbase/compare-util.h"       // smbase::compare [h]
#include "smbase/ordered-map.h"        // smbase::OrderedMap [h]
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
  GDValueSourceLocation::resetFileNameToIndex();

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

  GDValueSourceLocation::globalSelfCheck();

  GDValueSourceLocation klb2(
    GDValueSourceLocation::c_saturatedFileAndLineValue + 10,
    GDValueSourceLocation::c_saturatedColumnValue);
  EXPECT_EQ(klb2.line(), 0xfffffff);
  EXPECT_TRUE(klb2.lineIsSaturated());
  EXPECT_TRUE(klb2.fileIndexIsSaturated());
  EXPECT_EQ(klb2.column(), 0xffffffffu);
  EXPECT_TRUE(klb2.columnIsSaturated());
  EXPECT_EQ(klb2.asString(), "268435455:4294967295");

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
  EXPECT_EQ(k.line(), 0xfffffff);
  EXPECT_EQ(k.column(), 0xffffffffu);

  GDValueSourceLocation::globalSelfCheck();

  // Undo the file+line saturation.
  GDValueSourceLocation::resetFileNameToIndex();
  GDValueSourceLocation::globalSelfCheck();
}


void test_order()
{
  GDValueSourceLocation locs[] = {
    GDValueSourceLocation(1, 1),
    GDValueSourceLocation(1, 10),
    GDValueSourceLocation(10, 1),
    GDValueSourceLocation(10, 10),
  };

  checkStrictlyOrdered("locs", vecArrayToCRefs(locs));
}


// TODO: Move to `sm-test`?
#define EXPECT_NULLOPT(valueOpt) xassert(!( (valueOpt).has_value() ))


void test_fileNames()
{
  // Reset to a clean mapping.
  GDValueSourceLocation::resetFileNameToIndex();
  GDValueSourceLocation::globalSelfCheck();

  EXPECT_EQ(GDValueSourceLocation::numFileIndices(), 1);

  // Add some names.
  auto idx1 = GDValueSourceLocation::fileIndexOfName("foo.cpp");
  auto idx2 = GDValueSourceLocation::fileIndexOfName("bar.cpp");
  EXPECT_TRUE(idx1 == 1);
  EXPECT_TRUE(idx2 == 2);

  EXPECT_EQ(GDValueSourceLocation::numFileIndices(), 3);

  // Getting them again yields same index.
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName("foo.cpp"), idx1);
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName("bar.cpp"), idx2);

  // Name lookups.
  EXPECT_EQ(GDValueSourceLocation::fileNameOfIndex(idx1), "foo.cpp");
  EXPECT_EQ(GDValueSourceLocation::fileNameOfIndex(idx2), "bar.cpp");

  // Optional mapping.
  std::string fooName = "foo.cpp";
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName(fooName), idx1);
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName(""), 0);

  // Construct with file index.
  GDValueSourceLocation loc1(idx1, 10, 20);
  EXPECT_TRUE(loc1.hasFileIndex());
  EXPECT_EQ(loc1.fileIndex(), idx1);
  EXPECT_EQ(loc1.fileName(), "foo.cpp");
  EXPECT_EQ(loc1.asString(), "foo.cpp:10:20");

  // Construct with no file index.
  GDValueSourceLocation loc2(10, 20);
  EXPECT_FALSE(loc2.hasFileIndex());
  EXPECT_EQ(loc2.fileIndex(), 0);
  EXPECT_EQ(loc2.fileName(), "");
  EXPECT_EQ(loc2.asString(), "10:20");

  // Saturated file index.
  GDValueSourceLocation satLoc(
    GDValueSourceLocation::c_saturatedFileAndLineValue, 6);
  EXPECT_TRUE(satLoc.fileIndexIsSaturated());
  EXPECT_EQ(loc2.fileName(), "");

  // Ordering comparisons with file indices.
  GDValueSourceLocation a(idx1, 1, 1);
  GDValueSourceLocation b(idx2, 1, 1);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(compare(a, b) < 0);
  EXPECT_STRICTLY_ORDERED(GDValueSourceLocation, a, b);

  // "foo.cpp", "bar.cpp", ""
  EXPECT_EQ(GDValueSourceLocation::numFileIndices(), 3);

  GDValueSourceLocation::globalSelfCheck();
}


void test_fileIndexSaturation()
{
  GDValueSourceLocation::resetFileNameToIndex();
  GDValueSourceLocation::globalSelfCheck();

  // The first 256 should exactly fill the available space.  Then the
  // 257th should saturate.
  for (int i=1; i <= 257; ++i) {
    EXN_CONTEXT(i);
    std::string name = stringb("name" << i);
    EXPECT_EQ(GDValueSourceLocation::fileIndexOfName(name), i);

    GDValueSourceLocation loc(i, 0xfffff, 1);
  }

  EXPECT_EQ(GDValueSourceLocation::numFileIndices(), 258);

  GDValueSourceLocation loc255(255, 1,1);
  GDValueSourceLocation loc256(256, 1,1);
  GDValueSourceLocation loc257(257, 1,1);

  EXPECT_FALSE(loc255.fileIndexIsSaturated());
  EXPECT_FALSE(loc256.fileIndexIsSaturated());
  EXPECT_TRUE(loc257.fileIndexIsSaturated());

  EXPECT_EQ(loc255.fileName(), "name255");
  EXPECT_EQ(loc256.fileName(), "name256");

  // Asking about a saturated location yields info about the last
  // unsaturated location.
  EXPECT_EQ(loc257.fileName(), "name256");

  GDValueSourceLocation::globalSelfCheck();
  GDValueSourceLocation::resetFileNameToIndex();
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_srcloc()
{
  test_basics();
  test_order();
  test_fileNames();
  test_fileIndexSaturation();
}


// EOF
