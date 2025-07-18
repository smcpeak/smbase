// gdvalue-parser-test.cc
// Code for `gdvaluer-parse`.

#include "smbase/gdvalue-list-fwd.h"             // gdv::toGDValue(std::list)
#include "smbase/gdvalue-map-fwd.h"              // gdv::toGDValue(std::map)
#include "smbase/gdvalue-set-fwd.h"              // gdv::toGDValue(std::set)
#include "smbase/gdvalue-unique-ptr-fwd.h"       // gdv::toGDValue(std::unique_ptr)
#include "smbase/gdvalue-vector-fwd.h"           // gdv::toGDValue(std::vector)

#include "smbase/gdvalue-list.h"                 // module under test
#include "smbase/gdvalue-map.h"                  // module under test
#include "smbase/gdvalue-parser-ops.h"           // module under test
#include "smbase/gdvalue-set.h"                  // module under test
#include "smbase/gdvalue-unique-ptr.h"           // module under test
#include "smbase/gdvalue-vector.h"               // module under test

#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/ordered-map-ops.h"              // smbase::OrderedMap ctor, etc.
#include "smbase/sm-macros.h"                    // {OPEN,CLOSE}_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

#include <limits>                                // std::numeric_limits
#include <list>                                  // std::list
#include <optional>                              // std::optional

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

  explicit Data(GDValueParser const &p)
    : m_x(gdvpTo<int>(p.mapGetValueAtSym("x"))),
      m_y(gdvpTo<int>(p.mapGetValueAtSym("y")))
  {
    p.checkTaggedMapTag("Data");
  }
};


// Convert `GDValue srcValue` to `destType` using `GDValueParser`.
#define GDVP_TO(destType, srcValue) \
  gdvpTo<destType>(GDValueParser(srcValue))


// Expect an `XGDValueError` with a certain substring in the message.
#define EXPECT_ERROR_SUBSTR(expr, substr) \
  EXPECT_EXN_SUBSTR(expr, XGDValueError, substr)


void test_bool()
{
  EXPECT_EQ(GDVP_TO(bool, GDValue(true)), true);
  EXPECT_EQ(GDVP_TO(bool, GDValue(false)), false);
  EXPECT_ERROR_SUBSTR(GDVP_TO(bool, GDValue()),
    "expected symbol `true` or `false`, not null");
}


void test_int()
{
  EXPECT_EQ(GDVP_TO(int, GDValue(3)), 3);

  if (sizeof(int) < sizeof(GDVSmallInteger)) {
    // Too big.
    GDVSmallInteger maxGSI = std::numeric_limits<GDVSmallInteger>::max();
    EXPECT_ERROR_SUBSTR(GDVP_TO(int, GDValue(maxGSI)),
      "number too large");
  }

  // Not an integer.
  EXPECT_ERROR_SUBSTR(GDVP_TO(int, GDValue()),
    "expected small integer, not symbol");
}


void test_string()
{
  EXPECT_EQ(GDVP_TO(std::string, GDValue("abc")), "abc");

  EXPECT_ERROR_SUBSTR(GDVP_TO(std::string, GDValue(GDVSymbol("abc"))),
    "expected string, not symbol");

  EXPECT_EQ(GDValueParser(GDValue("xyz")).stringGet(), "xyz");

  EXPECT_ERROR_SUBSTR(GDValueParser("xyz"_sym).stringGet(),
    "expected string, not symbol");
}


void test_unique_ptr()
{
  std::unique_ptr<Data> d1(new Data(3,4));
  GDValue v(toGDValue(d1));
  EXPECT_EQ(v.asString(), "Data{x:3 y:4}");

  std::unique_ptr<Data> d2(GDVP_TO(std::unique_ptr<Data>, v));
  EXPECT_EQ(toGDValue(d2), v);

  // Test some GDValueParser error cases now that we have a container to
  // work with.
  GDValueParser p(v);

  // Non-existent key.
  EXPECT_ERROR_SUBSTR(p.mapGetValueAtSym("z"),
    "key z, but it does not");

  // Wrong container type.
  EXPECT_ERROR_SUBSTR(p.tupleGetValueAt(0),
    "tuple, not tagged map");

  // Wrong scalar kind at a key; demonstrates showing the path.
  EXPECT_ERROR_SUBSTR(p.mapGetValueAtSym("x").symbolGet(),
    "<top>.x: expected symbol, not small integer");
}


void test_vector()
{
  std::vector<Data> vec1{{1,2}, {3,4}};
  GDValue v(toGDValue(vec1));
  EXPECT_EQ(v.asString(), "[Data{x:1 y:2} Data{x:3 y:4}]");

  std::vector<Data> vec2(GDVP_TO(std::vector<Data>, v));
  EXPECT_EQ(toGDValue(vec2), v);

  // Test some parser error cases.
  GDValueParser p(v);

  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(2),
    "index 2, but it only has 2 elements");

  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(1).sequenceGetValueAt(0),
    "<top>[1]: expected sequence, not tagged map");

  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(1).mapGetValueAtSym("x").symbolGet(),
    "<top>[1].x: expected symbol, not small integer");
}


void test_vector_of_unique()
{
  std::vector<std::unique_ptr<Data>> vec1;
  vec1.push_back(std::make_unique<Data>(1,2));
  vec1.push_back(std::make_unique<Data>(3,4));
  GDValue v(toGDValue(vec1));
  EXPECT_EQ(v.asString(), "[Data{x:1 y:2} Data{x:3 y:4}]");

  std::vector<std::unique_ptr<Data>> vec2(
    GDVP_TO(std::vector<std::unique_ptr<Data>>, v));
  EXPECT_EQ(toGDValue(vec2), v);

  GDValueParser p(v);
  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(1).mapGetValueAtSym("x").symbolGet(),
    "<top>[1].x: expected symbol, not small integer");
}


void test_map()
{
  std::map<int, int> m1{{1,2}, {3,4}};
  GDValue v(toGDValue(m1));
  EXPECT_EQ(v.asString(), "{1:2 3:4}");

  // Work around the problem of passing a template specialization to a
  // macro and getting unwanted argument splitting at the comma.
  typedef std::map<int, int> map_int_int;

  std::map<int, int> m2(GDVP_TO(map_int_int, v));
  EXPECT_EQ(toGDValue(m2), v);
}


void test_set()
{
  std::set<int> s1{2,3,5,7};
  GDValue v(toGDValue(s1));
  EXPECT_EQ(v.asString(), "{2 3 5 7}");

  std::set<int> m2(GDVP_TO(std::set<int>, v));
  EXPECT_EQ(toGDValue(m2), v);
}


void test_map_of_vector_of_unique()
{
  typedef std::map<std::string, std::vector<std::unique_ptr<Data>>> DataVecMap;

  std::vector<std::unique_ptr<Data>> fooVec;
  fooVec.emplace_back(std::make_unique<Data>(1, 2));
  fooVec.emplace_back(std::make_unique<Data>(3, 4));

  std::vector<std::unique_ptr<Data>> barVec;
  barVec.emplace_back(std::make_unique<Data>(5, 6));

  DataVecMap m1;
  m1.insert(std::make_pair(
    std::string("foo"),
    std::move(fooVec)
  ));
  m1.insert(std::make_pair(
    std::string("bar"),
    std::move(barVec)
  ));

  GDValue v(toGDValue(m1));
  EXPECT_EQ(v.asString(),
    "{\"bar\":[Data{x:5 y:6}] "
     "\"foo\":[Data{x:1 y:2} Data{x:3 y:4}]}");

  DataVecMap m2(GDVP_TO(DataVecMap, v));
  EXPECT_EQ(toGDValue(m2), v);
}


void test_mapGetValueAtSymOpt()
{
  // Trying to get a value from a non-map.
  {
    GDValue v;
    EXPECT_ERROR_SUBSTR(GDValueParser(v).mapGetValueAtSymOpt("foo"),
      "expected map, not symbol");
  }

  {
    // Trying to get a value from an unmapped key.
    GDValue v(GDVK_MAP);
    xassert(GDValueParser(v).mapGetValueAtSymOpt("foo") == std::nullopt);

    // And a mapped key.
    v.mapSetSym("foo", GDValue(3));
    EXPECT_EQ(GDValueParser(v).mapGetValueAtSymOpt("foo").value().getValue(), GDValue(3));
  }
}


void test_gdvpOptTo()
{
  EXPECT_EQ(gdvpOptTo<int>(GDValueParser(GDValue(3))), 3);
  EXPECT_EQ(gdvpOptTo<int>(std::nullopt), 0);
}


class Data2 {
public:      // data
  // Uses a symbol as a key.
  std::string m_s1;

  // Uses a string as a key.
  std::list<int> m_intList;

public:      // funcs
  explicit Data2(GDValueParser const &p)
    : GDVP_READ_OPT_MEMBER_SYM(m_s1),
      GDVP_READ_OPT_MEMBER_STR(m_intList)
  {}

  operator GDValue() const
  {
    GDValue m(GDVK_MAP);

    GDV_WRITE_MEMBER(m_s1);
    GDV_WRITE_MEMBER_SK(m_intList);

    // Exercise the non-Opt parser too.
    xassert(GDValueParser(m).mapGetValueAtStr("intList").getValue() == toGDValue(m_intList));

    return m;
  }
};


void testWithData2()
{
  GDValue serialized(GDVMap{
    { "s1"_sym, "s1value" },
    { "intList", GDVSequence{1,2,3} },
  });

  Data2 d{GDValueParser(serialized)};
  EXPECT_EQ(toGDValue(d), serialized);
}


// Exercise some more cases of paths in GDValueParser.
void test_parserPaths()
{
  GDVInteger bigInt =
    Integer::fromDigits("1234567890123456789012345678901234567890");

  GDValue v(GDVMap{
    { 1, 2 },
    { "three"_sym, "four" },
    { bigInt, -17 },
    { "seq"_sym,
      GDVSequence{
        "one",
        "two"_sym,
        3,
        GDVTuple{
          4,
          "five",
          GDVSet{
            6,
            "seven",
            GDVOrderedMap{
              { 8, "nine" },
            },
          },
        },
      },
    },
    { GDVSequence{1,2,3}, 4 },
    { "omap"_sym,
      GDVOrderedMap{
        { 3, "three" },
        { 2, "two" },
        { 1, "one" },
        { "zero"_sym, 0 },
      },
    },
    { "tmap"_sym,
      GDVTaggedMap{"tmaptag"_sym, {
        { "a", "b" },
      }},
    },
    { "tomap"_sym,
      GDVTaggedOrderedMap{"tomaptag"_sym, {
        { "c", "d" },
      }},
    },
  });

  GDValueParser p(v);

  // It's a little silly to complain about the internals of a key, since
  // the client typically knows the key's entire structure beforehand,
  // but this might happen if we are enumerating all keys.
  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt(1).checkIsSymbol(),
    "path <top>@1: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt(GDVSequence{1,2,3}).sequenceGetValueAt(0).checkIsSymbol(),
    "path <top>@[1 2 3][0]: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt(1).checkIsSymbol(),
    "path <top>.1: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt("three"_sym).checkIsInteger(),
    "path <top>@three: expected integer, not symbol");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt("three"_sym).checkIsInteger(),
    "path <top>.three: expected integer, not string");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt(bigInt).checkIsMap(),
    "path <top>.1234567890123456789012345678901234567890: expected map, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(1).checkIsInteger(),
    "path <top>.seq[1]: expected integer, not symbol");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).checkIsInteger(),
    "path <top>.seq[3]: expected integer, not tuple");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(0)
     .checkIsSymbol(),
    "path <top>.seq[3][0]: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(0)
     .checkIsSymbol(),
    "path <top>.seq[3][0]: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .checkIsSymbol(),
    "path <top>.seq[3][2]: expected symbol, not set");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(6).checkIsSymbol(),
    "path <top>.seq[3][2]@6: expected symbol, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(GDVOrderedMap{{8,"nine"}}).checkIsSymbol(),
    "path <top>.seq[3][2]@[8:\"nine\"]: expected symbol, not ordered map");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(GDVOrderedMap{{8,"nine"}}).orderedMapGetValueAt(8)
     .checkIsSymbol(),
    "path <top>.seq[3][2]@[8:\"nine\"].8: expected symbol, not string");

  EXPECT_EQ(p.mapGetValueAt(1).integerGet(), 2);
  EXPECT_ERROR_SUBSTR(p.mapGetValueAtSym("three").integerGet(),
    "<top>.three: expected integer, not string");
  EXPECT_EQ(p.mapGetValueAt(1).integerIsNegative(), false);
  EXPECT_EQ(p.mapGetValueAt(bigInt).integerIsNegative(), true);
  EXPECT_EQ(p.mapGetKeyAt(bigInt).integerIsNegative(), false);
  EXPECT_EQ(p.mapGetKeyAt(bigInt).largeIntegerGet(), bigInt);

  xassert(p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGet()[0] == 4);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(2).tupleGet()[0],
    "<top>.seq[2]: expected tuple, not small integer");

  xassert(p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
           .tupleGetValueAt(2).isSet());
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(3),
    "<top>.seq[3]: expected tuple to have element at index 3, but it only has 3 elements");

  EXPECT_EQ(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
     .tupleGetValueAt(2).setGetValue(6).smallIntegerGet(),
    6);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
     .tupleGetValueAt(2).setGetValue(66).smallIntegerGet(),
    "<top>.seq[3][2]: expected set to have element 66, but it does not");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt("nonexist"_sym),
    "<top>: expected map to have key nonexist, but it does not");

  xassert(p.mapContainsSym("seq"));
  xassert(!p.mapContainsSym("nonexist"));

  EXPECT_EQ(p.mapGetValueAtSym("omap").orderedMapGet().valueAtKey(1), "one");
  EXPECT_EQ(p.mapGetValueAtSym("omap").orderedMapGet().valueAtIndex(0), "three");
  p.mapGetValueAtSym("omap").checkIsPOMap();
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").checkIsPOMap(),
    "<top>.seq: expected (possibly ordered) map, not sequence");
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetKeyAt(1).getValue(),
    GDValue(1));
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("omap").orderedMapGetKeyAt(4).getValue(),
    "<top>.omap: expected ordered map to have key 4, but it does not");
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetValueAt(1).getValue(),
    GDValue("one"));
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("omap").orderedMapGetValueAt(4).getValue(),
    "<top>.omap: expected ordered map to have key 4, but it does not");
  xassert(!p.mapGetValueAtSym("omap").orderedMapContainsSym("x"));
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetValueAtSym("zero").getValue(),
    GDValue(0));

  EXPECT_EQ(
    p.mapGetValueAtSym("tmap").taggedContainerGetTag(),
    "tmaptag"_sym);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkContainerTag("z"),
    "<top>.tmap: expected container to have tag z, but it instead has tag tmaptag");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkTaggedOrderedMapTag("tomaptag"),
    "<top>.tmap: expected tagged ordered map, not tagged map");
  p.mapGetValueAtSym("tomap").checkTaggedOrderedMapTag("tomaptag");

  // Do a test using a temporary object and parser to exercise the case
  // where we catch the exception after both have been destroyed.  This
  // would have been a problem with the original `XGDValueError` design,
  // which carried a copy of the `GDValueParser` object.
  EXPECT_ERROR_SUBSTR(
    GDValueParser(GDVMap{{1,2}}).mapGetValueAt(1).checkIsTaggedOrderedMap(),
    "<top>.1: expected tagged ordered map, not small integer");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkTaggedOrderedMapTag("tomaptag"),
    "<top>.tmap: expected tagged ordered map, not tagged map");

  // Exercise some simple queries.
  EXPECT_EQ(p.getKindName(), "GDVK_MAP");
  xassert(p.getSuperKind() == GDVK_MAP);
  xassert(p.mapGetKeyAt(1).getSuperKind() == GDVK_INTEGER);
  xassert(!p.isSymbol());
  xassert(!p.isTaggedSequence());
  xassert(!p.isTaggedTuple());
  xassert(!p.isTaggedSet());
  xassert(!p.isTaggedOrderedMap());
  xassert(!p.isTaggedPOMap());
  xassert(p.isPOMap());
  xassert(!p.isOrderedContainer());
  xassert(p.isUnorderedContainer());
  xassert(!p.isNull());
  xassert(!p.isBool());
  xassert(GDValueParser(GDValue()).isNull());
  xassert(GDValueParser(GDValue(true)).isBool());
  xassert(!p.containerIsEmpty());
  xassert(p.mapGetKeyAt(GDVSequence{1,2,3}).sequenceGet() ==
          (GDVSequence{1,2,3}));

  // Test copying the parser.
  {
    GDValueParser p2(p.mapGetValueAtSym("seq"));
    xassert(p2.isSequence());
    EXPECT_EQ(p2.sequenceGetValueAt(0).getValue(), GDValue("one"));

    // Copy using lvalue reference.
    GDValueParser p3(p2);
    xassert(p3.isSequence());
    EXPECT_EQ(p3.sequenceGetValueAt(0).getValue(), GDValue("one"));
  }
}



void test_copy_XGDValueError()
{
  XGDValueError e1("p", "m");
  XGDValueError e2(e1);
  EXPECT_EQ(e2.getConflict(), "At GDV path p: m");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_gdvalue_parser()
{
  // Activate extensive self-checking.
  GDValueParser::s_selfCheckCtors = true;

  test_bool();
  test_int();
  test_string();
  test_unique_ptr();
  test_vector();
  test_vector_of_unique();
  test_map();
  test_set();
  test_map_of_vector_of_unique();
  test_mapGetValueAtSymOpt();
  test_gdvpOptTo();
  testWithData2();
  test_parserPaths();
  test_copy_XGDValueError();
}


// EOF
