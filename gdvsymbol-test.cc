// gdvsymbol-test.cc
// Tests for `gdvsymbol` module.

// This file is in the public domain.

#include "gdvsymbol.h"                 // module under test

#include "sm-test.h"                   // EXPECT_EQ

#include <string_view>                 // std::string_view

using namespace gdv;


// Called from unit-tests.cc.
void test_gdvsymbol()
{
  GDVSymbol s1;
  EXPECT_EQ(s1.getSymbolName(), "null");
  EXPECT_EQ(s1.size(), 4);
  EXPECT_EQ(s1.getSymbolIndex(), 0);
  EXPECT_EQ(stringb(s1), "null");
  EXPECT_EQ(s1.asString(), "null");

  GDVSymbol s2("hello");
  EXPECT_EQ(s2.getSymbolName(), "hello");
  EXPECT_EQ(s2.size(), 5);
  EXPECT_EQ(stringb(s2), "hello");
  EXPECT_EQ(s2.asString(), "hello");
  xassert(s1 > s2);
  xassert(GDVSymbol::compareIndices(s1.getSymbolIndex(),
                                    s2.getSymbolIndex()) > 0);

  s1.swap(s2);

  EXPECT_EQ(s2.getSymbolName(), "null");
  EXPECT_EQ(s1.getSymbolName(), "hello");
  xassert(s1 < s2);

  GDVSymbol s3(GDVSymbol::DirectIndex, s1.getSymbolIndex());
  EXPECT_EQ(s3.getSymbolName(), "hello");
  EXPECT_EQ(s1, s3);
  EXPECT_EQ(compare(s1, s3), 0);

  xassert(!GDVSymbol::validSymbolName(""));
  xassert(GDVSymbol::validSymbolName("_"));
  xassert(GDVSymbol::validSymbolName("_9"));
  xassert(!GDVSymbol::validSymbolName("9"));
  xassert(GDVSymbol::validSymbolName("a9"));
  xassert(!GDVSymbol::validSymbolName("a!"));
  xassert(!GDVSymbol::validSymbolName("!"));
}


// EOF
