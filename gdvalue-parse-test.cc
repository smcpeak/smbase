// gdvalue-parse-test.cc
// Code for `gdvalue-parse`.

#include "smbase/gdvalue-parse.h"      // module under test
#include "smbase/gdvalue-unique-ptr.h" // module under test
#include "smbase/gdvalue-vector.h"     // module under test

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <limits>                      // std::numeric_limits

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class Data final {
public:      // data
  int m_x;
  int m_y;

public:      // funcs
  Data(int x, int y) : m_x(x), m_y(y) {}

  operator GDValue() const
  {
    GDValue m(GDVK_TAGGED_MAP, "Data"_sym);

    m.mapSetSym("x", m_x);
    m.mapSetSym("y", m_y);

    return m;
  }

  explicit Data(GDValue const &m)
    : m_x(gdvTo<int>(mapGetSym_parse(m, "x"))),
      m_y(gdvTo<int>(mapGetSym_parse(m, "y")))
  {
    checkTaggedMapTag(m, "Data");
  }
};


void test_int()
{
  EXPECT_EQ(gdvTo<int>(GDValue(3)), 3);

  if (sizeof(int) < sizeof(GDVSmallInteger)) {
    // Too big.
    GDVSmallInteger maxGSI = std::numeric_limits<GDVSmallInteger>::max();
    EXPECT_EXN(gdvTo<int>(GDValue(maxGSI)), XFormat);
  }

  // Not an integer.
  EXPECT_EXN(gdvTo<int>(GDValue()), XFormat);
}


void test_string()
{
  EXPECT_EQ(gdvTo<std::string>(GDValue("abc")), "abc");

  EXPECT_EXN(gdvTo<std::string>(GDValue(GDVSymbol("abc"))), XFormat);
}


void test_unique_ptr()
{
  std::unique_ptr<Data> d1(new Data(3,4));
  GDValue v(toGDValue(d1));
  EXPECT_EQ(v.asString(), "Data{x:3 y:4}");

  std::unique_ptr<Data> d2(gdvTo<std::unique_ptr<Data>>(v));
  EXPECT_EQ(toGDValue(d2), v);
}


void test_vector()
{
  std::vector<Data> vec1{{1,2}, {3,4}};
  GDValue v(toGDValue(vec1));
  EXPECT_EQ(v.asString(), "[Data{x:1 y:2} Data{x:3 y:4}]");

  std::vector<Data> vec2(gdvTo<std::vector<Data>>(v));
  EXPECT_EQ(toGDValue(vec2), v);
}


void test_vector_of_unique()
{
  std::vector<std::unique_ptr<Data>> vec1;
  vec1.push_back(std::make_unique<Data>(1,2));
  vec1.push_back(std::make_unique<Data>(3,4));
  GDValue v(toGDValue(vec1));
  EXPECT_EQ(v.asString(), "[Data{x:1 y:2} Data{x:3 y:4}]");

  std::vector<std::unique_ptr<Data>> vec2(
    gdvTo<std::vector<std::unique_ptr<Data>>>(v));
  EXPECT_EQ(toGDValue(vec2), v);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_gdvalue_parse()
{
  test_int();
  test_string();
  test_unique_ptr();
  test_vector();
  test_vector_of_unique();
}


// EOF
