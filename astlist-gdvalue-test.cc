// astlist-gdvalue-test.cc
// Tests for `astlist-gdvalue.h`.
// See license.txt for copyright and terms of use.

#include "astlist-gdvalue.h"           // module under test

#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class Data {
public:
  int m_x;

public:      // funcs
  Data(int x)
    : m_x(x)
  {}

  operator GDValue() const
  {
    return GDValue(GDVTaggedMap(GDVSymbol("Data"), {
      GDV_SKV("x", m_x),
    }));
  }
};


void test1()
{
  ASTList<Data> lst;
  EXPECT_EQ(toGDValue(lst).asString(), "[]");

  lst.append(new Data(1));
  EXPECT_EQ(toGDValue(lst).asString(), "[Data{x:1}]");

  lst.append(new Data(22));
  EXPECT_EQ(toGDValue(lst).asString(), "[Data{x:1} Data{x:22}]");

  lst.append(new Data(3));
  EXPECT_EQ(toGDValue(lst).asString(), "[Data{x:1} Data{x:22} Data{x:3}]");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_astlist_gdvalue()
{
  test1();
}


// EOF
