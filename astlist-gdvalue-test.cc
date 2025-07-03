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
  explicit Data(int x)
    : m_x(x)
  {}

  operator GDValue() const
  {
    return GDValue(GDVTaggedMap(GDVSymbol("Data"), {
      GDV_SKV("x", m_x),
    }));
  }

  explicit Data(GDValue const &v)
    : m_x(gdvTo<int>(mapGetSym_parse(v, "x")))
  {
    checkTaggedMapTag(v, "Data");
  }
};


CLOSE_ANONYMOUS_NAMESPACE


// Annoyingly, I can't specialize this from inside the anonymous
// namespace, so have to temporarily close it.
template <>
struct gdv::GDVToNew<Data> {
  static Data *f(GDValue const &v)
  {
    return new Data(v);
  }
};


OPEN_ANONYMOUS_NAMESPACE


// Convert `orig` to GDV, then convert back and check for equality.
// Also check that the serialized form is `expectGDVN`.
void testOne(ASTList<Data> const &orig, char const *expectGDVN)
{
  GDValue v(toGDValue(orig));

  std::string actualGDVN = v.asString();
  EXPECT_EQ(actualGDVN, expectGDVN);

  ASTList<Data> after(gdv::gdvTo<ASTList<Data>>(v));

  EXPECT_EQ(after.count(), orig.count());

  ASTListIter iter1(orig);
  ASTListIter iter2(after);
  while (!iter1.isDone()) {
    xassert(!iter2.isDone());

    EXPECT_EQ(iter2.data()->m_x, iter1.data()->m_x);

    iter1.adv();
    iter2.adv();
  }
}


void testToAndFromGDValue()
{
  ASTList<Data> lst;
  testOne(lst, "[]");

  lst.append(new Data(1));
  testOne(lst, "[Data{x:1}]");

  lst.append(new Data(22));
  testOne(lst, "[Data{x:1} Data{x:22}]");

  lst.append(new Data(3));
  testOne(lst, "[Data{x:1} Data{x:22} Data{x:3}]");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_astlist_gdvalue()
{
  testToAndFromGDValue();
}


// EOF
