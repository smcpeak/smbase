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

  GDVSymbol s2("hello");
  EXPECT_EQ(s2.getSymbolName(), "hello");
  EXPECT_EQ(s2.size(), 5);

  s1.swap(s2);

  EXPECT_EQ(s2.getSymbolName(), "null");
  EXPECT_EQ(s1.getSymbolName(), "hello");

  xassert(!GDVSymbol::validSymbolName(""));
  xassert(GDVSymbol::validSymbolName("_"));
  xassert(GDVSymbol::validSymbolName("_9"));
  xassert(!GDVSymbol::validSymbolName("9"));
  xassert(GDVSymbol::validSymbolName("a9"));
  xassert(!GDVSymbol::validSymbolName("a!"));
  xassert(!GDVSymbol::validSymbolName("!"));
}


// EOF
