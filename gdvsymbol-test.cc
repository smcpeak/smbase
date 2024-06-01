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
  EXPECT_EQ(s1.getSymbolName(), "");

  GDVSymbol s2("hello");
  EXPECT_EQ(s2.getSymbolName(), "hello");

  s1.swap(s2);

  EXPECT_EQ(s2.getSymbolName(), "");
  EXPECT_EQ(s1.getSymbolName(), "hello");
}


// EOF
