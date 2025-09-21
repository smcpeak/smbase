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
  EXPECT_EQ(klb2.line(), 0xfffff);
  EXPECT_TRUE(klb2.lineIsSaturated());
  EXPECT_EQ(klb2.column(), 0xffffffffu);
  EXPECT_TRUE(klb2.columnIsSaturated());
  EXPECT_EQ(klb2.asString(), "1048575:4294967295");

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
  EXPECT_EQ(k.line(), 0xfffff);
  EXPECT_EQ(k.column(), 0xffffffffu);
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

  EXPECT_EQ(GDValueSourceLocation::fileNameToIndexC()->size(), 1);

  // Add some names.
  auto idx1 = GDValueSourceLocation::fileIndexOfName("foo.cpp");
  auto idx2 = GDValueSourceLocation::fileIndexOfName("bar.cpp");
  EXPECT_TRUE(idx1 == 1);
  EXPECT_TRUE(idx2 == 2);

  EXPECT_EQ(GDValueSourceLocation::fileNameToIndexC()->size(), 3);

  // Getting them again yields same index.
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName("foo.cpp"), idx1);
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfName("bar.cpp"), idx2);

  // Name lookups.
  EXPECT_EQ(GDValueSourceLocation::fileNameOptOfIndex(idx1).value(), "foo.cpp");
  EXPECT_EQ(GDValueSourceLocation::fileNameOptOfIndex(idx2).value(), "bar.cpp");

  // Optional mapping.
  std::optional<std::string> fooName = "foo.cpp";
  EXPECT_EQ(GDValueSourceLocation::fileIndexOfNameOpt(fooName).value(), idx1);
  EXPECT_NULLOPT(GDValueSourceLocation::fileIndexOfNameOpt(std::nullopt));

  // Construct with file index.
  GDValueSourceLocation loc1(idx1, 10, 20);
  EXPECT_TRUE(loc1.hasFileIndex());
  EXPECT_EQ(loc1.fileIndex(), idx1);
  EXPECT_EQ(loc1.fileNameOpt().value(), "foo.cpp");
  EXPECT_EQ(loc1.fileNameOrExplanationOpt().value(), "foo.cpp");
  EXPECT_EQ(loc1.asString(), "foo.cpp:10:20");

  // Construct with no file index.
  GDValueSourceLocation loc2(10, 20);
  EXPECT_FALSE(loc2.hasFileIndex());
  EXPECT_EQ(loc2.fileIndexOrZero(), 0u);
  EXPECT_NULLOPT(loc2.fileIndexOpt());
  EXPECT_NULLOPT(loc2.fileNameOpt());
  EXPECT_NULLOPT(loc2.fileNameOrExplanationOpt());
  EXPECT_EQ(loc2.asString(), "10:20");

  // Saturated file index.
  GDValueSourceLocation satLoc(
    GDValueSourceLocation::c_saturatedFileIndexValue, 5, 6);
  EXPECT_TRUE(satLoc.fileIndexIsSaturated());
  EXPECT_NULLOPT(satLoc.fileNameOpt());
  EXPECT_EQ(satLoc.fileNameOrExplanationOpt().value(), "(Saturated FileIndex)");

  // Unmapped file index (greater than any existing).
  GDValueSourceLocation unmapped(idx2 + 10, 1, 1);
  EXPECT_TRUE(unmapped.hasFileIndex());
  EXPECT_FALSE(unmapped.fileIndexIsSaturated());
  EXPECT_NULLOPT(unmapped.fileNameOpt());
  EXPECT_EQ(unmapped.fileNameOrExplanationOpt().value(),
            stringb("(FileIndex " << (idx2 + 10) << ")"));

  // Ordering comparisons with file indices.
  GDValueSourceLocation a(idx1, 1, 1);
  GDValueSourceLocation b(idx2, 1, 1);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(compare(a, b) < 0);
  EXPECT_STRICTLY_ORDERED(GDValueSourceLocation, a, b);

  EXPECT_EQ(GDValueSourceLocation::fileNameToIndexC()->size(), 3);
  GDValueSourceLocation::globalSelfCheck();
}


void test_fileIndexSaturation()
{
  GDValueSourceLocation::resetFileNameToIndex();
  GDValueSourceLocation::globalSelfCheck();

  for (int i=1; i <= 256; ++i) {
    EXN_CONTEXT(i);
    std::string name = stringb("name" << i);
    EXPECT_EQ(GDValueSourceLocation::fileIndexOfName(name), i);
  }

  EXPECT_EQ(GDValueSourceLocation::fileNameToIndexC()->size(), 257);

  GDValueSourceLocation loc254(254, 1,1);
  GDValueSourceLocation loc255(255, 1,1);
  GDValueSourceLocation loc256(256, 1,1);

  EXPECT_FALSE(loc254.fileIndexIsSaturated());
  EXPECT_TRUE(loc255.fileIndexIsSaturated());
  EXPECT_TRUE(loc256.fileIndexIsSaturated());

  EXPECT_EQ(loc254.fileNameOrExplanationOpt().value(),
            "name254");
  EXPECT_EQ(loc255.fileNameOrExplanationOpt().value(),
            "(Saturated FileIndex)");
  EXPECT_EQ(loc256.fileNameOrExplanationOpt().value(),
            "(Saturated FileIndex)");

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
