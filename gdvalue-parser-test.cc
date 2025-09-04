// gdvalue-parser-test.cc
// Code for `gdvaluer-parse`.

#include "smbase/gdvalue-either-fwd.h"           // gdv::toGDValue(smbase::Either)
#include "smbase/gdvalue-list-fwd.h"             // gdv::toGDValue(std::list)
#include "smbase/gdvalue-map-fwd.h"              // gdv::toGDValue(std::map)
#include "smbase/gdvalue-optional-fwd.h"         // gdv::toGDValue(std::optional)
#include "smbase/gdvalue-set-fwd.h"              // gdv::toGDValue(std::set)
#include "smbase/gdvalue-tuple-fwd.h"            // gdv::toGDValue(std::tuple)
#include "smbase/gdvalue-unique-ptr-fwd.h"       // gdv::toGDValue(std::unique_ptr)
#include "smbase/gdvalue-vector-fwd.h"           // gdv::toGDValue(std::vector)

#include "smbase/gdvalue-either.h"               // module under test
#include "smbase/gdvalue-list.h"                 // module under test
#include "smbase/gdvalue-map.h"                  // module under test
#include "smbase/gdvalue-optional.h"             // module under test
#include "smbase/gdvalue-parser-ops.h"           // module under test
#include "smbase/gdvalue-set.h"                  // module under test
#include "smbase/gdvalue-tuple.h"                // module under test
#include "smbase/gdvalue-unique-ptr.h"           // module under test
#include "smbase/gdvalue-vector.h"               // module under test

#include "smbase/gdv-binary64-float.h"           // gdv::GDVBinary64Float
#include "smbase/gdv-ordered-map.h"              // gdv::GDVOrderedMap
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/gdvn-test-roundtrip.h"          // gdvnTestRoundtrip
#include "smbase/sm-macros.h"                    // {OPEN,CLOSE}_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

#include <cmath>                                 // std::pow
#include <limits>                                // std::numeric_limits
#include <list>                                  // std::list
#include <optional>                              // std::optional
#include <tuple>                                 // std::tuple
#include <vector>                                // std::vector

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

    m.mapSetValueAtSym("x", m_x);
    m.mapSetValueAtSym("y", m_y);

    return m;
  }

  explicit Data(GDValueParser const &p)
    : GDVP_READ_MEMBER_SYM(m_x),
      GDVP_READ_MEMBER_SYM(m_y)
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
    "Expected symbol `true` or `false`, not null.");

  gdvnTestRoundtripEq(true, "true");
  gdvnTestRoundtripEq(false, "false");

  EXPECT_ERROR_SUBSTR(GDValueParser(fromGDVN("foo")).boolGet(),
    "Expected symbol `true` or `false`, not foo.");
  EXPECT_ERROR_SUBSTR(GDValueParser(fromGDVN("`a b c`")).boolGet(),
    "Expected symbol `true` or `false`, not `a b c`.");

  {
    GDValue t(true);
    GDValueParser p(t);
    p.checkIsSymbol();
    EXPECT_TRUE(p.isBool());
    EXPECT_TRUE(p.boolGet());
    EXPECT_EQ(p.symbolGet(), "true"_sym);
    EXPECT_EQ(p.symbolGetName(), "true");
  }

  {
    GDValue f(false);
    GDValueParser p(f);
    p.checkIsSymbol();
    EXPECT_TRUE(p.isBool());
    EXPECT_FALSE(p.boolGet());
    EXPECT_EQ(p.symbolGet(), "false"_sym);
    EXPECT_EQ(p.symbolGetName(), "false");
  }
}


void test_int()
{
  EXPECT_EQ(GDVP_TO(int, GDValue(3)), 3);

  if (sizeof(int) < sizeof(GDVSmallInteger)) {
    // Too big.
    GDVSmallInteger maxGSI = std::numeric_limits<GDVSmallInteger>::max();
    EXPECT_ERROR_SUBSTR(GDVP_TO(int, GDValue(maxGSI)),
      "cannot be represented");
  }

  // Not an integer.
  EXPECT_ERROR_SUBSTR(GDVP_TO(int, GDValue()),
    "Expected integer, not symbol.");

  gdvnTestRoundtripEq(0, "0");
  gdvnTestRoundtripEq(123, "123");
  gdvnTestRoundtripEq(-456, "-456");
}


void test_string()
{
  EXPECT_EQ(GDVP_TO(std::string, GDValue("abc")), "abc");

  EXPECT_ERROR_SUBSTR(GDVP_TO(std::string, GDValue(GDVSymbol("abc"))),
    "Expected string, not symbol.");

  EXPECT_EQ(GDValueParser(GDValue("xyz")).stringGet(), "xyz");

  EXPECT_ERROR_SUBSTR(GDValueParser("xyz"_sym).stringGet(),
    "Expected string, not symbol.");

  gdvnTestRoundtripEq(std::string("abc"), "\"abc\"");
  gdvnTestRoundtripEq(std::string(""), "\"\"");
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
    "key z, but it does not.");

  // Wrong container type.
  EXPECT_ERROR_SUBSTR(p.tupleGetValueAt(0),
    "tuple, not tagged map.");

  // Wrong scalar kind at a key; demonstrates showing the path.
  EXPECT_ERROR_SUBSTR(p.mapGetValueAtSym("x").symbolGet(),
    "<top>.x: Expected symbol, not small integer.");

  gdvnTestRoundtrip(std::make_unique<Data>(5,6), "Data{x:5 y:6}");
}


// De/serialization should handle a null unique pointer.
void test_null_unique_ptr()
{
  std::unique_ptr<Data> d1;
  GDValue const v(toGDValue(d1));
  EXPECT_EQ(v, GDValue());

  std::unique_ptr<Data> d2(GDVP_TO(std::unique_ptr<Data>, v));
  EXPECT_EQ(toGDValue(d2), v);

  // Both are nullptr.
  xassert(d1 == d2);

  gdvnTestRoundtrip(d1, "null");
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
    "index 2, but it only has 2 elements.");

  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(1).sequenceGetValueAt(0),
    "<top>[1]: Expected sequence, not tagged map.");

  EXPECT_ERROR_SUBSTR(p.sequenceGetValueAt(1).mapGetValueAtSym("x").symbolGet(),
    "<top>[1].x: Expected symbol, not small integer.");

  gdvnTestRoundtrip(vec1, "[Data{x:1 y:2} Data{x:3 y:4}]");
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
    "<top>[1].x: Expected symbol, not small integer.");

  gdvnTestRoundtrip(vec1, "[Data{x:1 y:2} Data{x:3 y:4}]");
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

  gdvnTestRoundtrip(m1, "{1:2 3:4}");
}


void test_set()
{
  std::set<int> s1{2,3,5,7};
  GDValue v(toGDValue(s1));
  EXPECT_EQ(v.asString(), "{2 3 5 7}");

  std::set<int> s2(GDVP_TO(std::set<int>, v));
  EXPECT_EQ(toGDValue(s2), v);

  gdvnTestRoundtrip(s1, "{2 3 5 7}");
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

  gdvnTestRoundtrip(m1,
    "{\"bar\":[Data{x:5 y:6}] "
     "\"foo\":[Data{x:1 y:2} Data{x:3 y:4}]}");
}


void test_mapGetValueAtSymOpt()
{
  // Trying to get a value from a non-map.
  {
    GDValue v;
    EXPECT_ERROR_SUBSTR(GDValueParser(v).mapGetValueAtSymOpt("foo"),
      "Expected (possibly ordered) map, not symbol.");
  }

  {
    // Trying to get a value from an unmapped key.
    GDValue v(GDVK_MAP);
    xassert(GDValueParser(v).mapGetValueAtSymOpt("foo") == std::nullopt);

    // And a mapped key.
    v.mapSetValueAtSym("foo", GDValue(3));
    EXPECT_EQ(GDValueParser(v).mapGetValueAtSymOpt("foo").value().getValue(), GDValue(3));
  }
}


void test_gdvpOptTo()
{
  EXPECT_EQ(gdvpOptTo<int>(GDValueParser(GDValue(3))), 3);
  EXPECT_EQ(gdvpOptTo<int>(std::nullopt), 0);
}


class Data2 final {
public:      // data
  // Uses a symbol as a key.
  std::string m_s1;

  // Uses a string as a key.
  std::list<int> m_intList;

  // String as key, not optional.
  std::string m_s2;

public:      // funcs
  explicit Data2(GDValueParser const &p)
    : GDVP_READ_OPT_MEMBER_SYM(m_s1),
      GDVP_READ_OPT_MEMBER_STR(m_intList),
      GDVP_READ_MEMBER_STR(m_s2)
  {}

  operator GDValue() const
  {
    GDValue m(GDVK_MAP);

    GDV_WRITE_MEMBER_SYM(m_s1);
    GDV_WRITE_MEMBER_STR(m_intList);
    GDV_WRITE_MEMBER_STR(m_s2);

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
    { "s2", "s2value" },
  });

  Data2 d{GDValueParser(serialized)};
  EXPECT_EQ(toGDValue(d), serialized);

  gdvnTestRoundtrip(d, "{"
    "s1:\"s1value\" "
    "\"intList\":[1 2 3] "
    "\"s2\":\"s2value\""
  "}");
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
    "path <top>@1: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt(GDVSequence{1,2,3}).sequenceGetValueAt(0).checkIsSymbol(),
    "path <top>@[1 2 3][0]: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt(1).checkIsSymbol(),
    "path <top>.1: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt("three"_sym).checkIsInteger(),
    "path <top>@three: Expected integer, not symbol.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt("three"_sym).checkIsInteger(),
    "path <top>.three: Expected integer, not string.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAt(bigInt).checkIsMap(),
    "path <top>.1234567890123456789012345678901234567890: Expected map, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(1).checkIsInteger(),
    "path <top>.seq[1]: Expected integer, not symbol.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).checkIsInteger(),
    "path <top>.seq[3]: Expected integer, not tuple.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(0)
     .checkIsSymbol(),
    "path <top>.seq[3][0]: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(0)
     .checkIsSymbol(),
    "path <top>.seq[3][0]: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .checkIsSymbol(),
    "path <top>.seq[3][2]: Expected symbol, not set.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(6).checkIsSymbol(),
    "path <top>.seq[3][2]@6: Expected symbol, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(GDVOrderedMap{{8,"nine"}}).checkIsSymbol(),
    "path <top>.seq[3][2]@[8:\"nine\"]: Expected symbol, not ordered map.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(2)
     .setGetValue(GDVOrderedMap{{8,"nine"}}).orderedMapGetValueAt(8)
     .checkIsSymbol(),
    "path <top>.seq[3][2]@[8:\"nine\"].8: Expected symbol, not string.");

  EXPECT_EQ(p.mapGetValueAt(1).integerGet(), 2);
  EXPECT_ERROR_SUBSTR(p.mapGetValueAtSym("three").integerGet(),
    "<top>.three: Expected integer, not string.");
  EXPECT_EQ(p.mapGetValueAt(1).integerIsNegative(), false);
  EXPECT_EQ(p.mapGetValueAt(bigInt).integerIsNegative(), true);
  EXPECT_EQ(p.mapGetKeyAt(bigInt).integerIsNegative(), false);
  EXPECT_EQ(p.mapGetKeyAt(bigInt).largeIntegerGet(), bigInt);

  xassert(p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGet()[0] == 4);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(2).tupleGet()[0],
    "<top>.seq[2]: Expected tuple, not small integer.");

  xassert(p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
           .tupleGetValueAt(2).isSet());
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3).tupleGetValueAt(3),
    "<top>.seq[3]: Expected tuple to have element at index 3, but it only has 3 elements.");

  EXPECT_EQ(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
     .tupleGetValueAt(2).setGetValue(6).smallIntegerGet(),
    6);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").sequenceGetValueAt(3)
     .tupleGetValueAt(2).setGetValue(66).smallIntegerGet(),
    "<top>.seq[3][2]: Expected set to have element 66, but it does not.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetKeyAt("nonexist"_sym),
    "<top>: Expected map to have key nonexist, but it does not.");

  xassert(p.mapContainsSym("seq"));
  xassert(!p.mapContainsSym("nonexist"));

  EXPECT_EQ(p.mapGetValueAtSym("omap").orderedMapGet().valueAtKey(1), "one");
  EXPECT_EQ(p.mapGetValueAtSym("omap").orderedMapGet().valueAtIndex(0), "three");
  p.mapGetValueAtSym("omap").checkIsPOMap();
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("seq").checkIsPOMap(),
    "<top>.seq: Expected (possibly ordered) map, not sequence.");
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetKeyAt(1).getValue(),
    GDValue(1));
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("omap").orderedMapGetKeyAt(4).getValue(),
    "<top>.omap: Expected ordered map to have key 4, but it does not.");
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetValueAt(1).getValue(),
    GDValue("one"));
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("omap").orderedMapGetValueAt(4).getValue(),
    "<top>.omap: Expected ordered map to have key 4, but it does not.");
  xassert(!p.mapGetValueAtSym("omap").orderedMapContainsSym("x"));
  EXPECT_EQ(
    p.mapGetValueAtSym("omap").orderedMapGetValueAtSym("zero").getValue(),
    GDValue(0));

  EXPECT_EQ(
    p.mapGetValueAtSym("tmap").taggedContainerGetTag(),
    "tmaptag"_sym);
  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkContainerTag("z"),
    "<top>.tmap: Expected container to have tag z, but it instead has tag tmaptag.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkTaggedOrderedMapTag("tomaptag"),
    "<top>.tmap: Expected tagged ordered map, not tagged map.");
  p.mapGetValueAtSym("tomap").checkTaggedOrderedMapTag("tomaptag");

  // Do a test using a temporary object and parser to exercise the case
  // where we catch the exception after both have been destroyed.  This
  // would have been a problem with the original `XGDValueError` design,
  // which carried a copy of the `GDValueParser` object.
  EXPECT_ERROR_SUBSTR(
    GDValueParser(GDVMap{{1,2}}).mapGetValueAt(1).checkIsTaggedOrderedMap(),
    "<top>.1: Expected tagged ordered map, not small integer.");

  EXPECT_ERROR_SUBSTR(
    p.mapGetValueAtSym("tmap").checkTaggedOrderedMapTag("tomaptag"),
    "<top>.tmap: Expected tagged ordered map, not tagged map.");

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


// Replicate a scenario where we move an object being parsed.
void test_move_parsedObject()
{
  // No clearing.
  {
    GDValue dest;

    {
      GDValue src(GDVMap{{1,2}});
      GDValueParser p(src);

      p.checkIsMap();

      // Having examined `p`, we decide to move the value, despite the
      // parser object still existing.  For now at least, I want to regard
      // this as valid.
      dest = std::move(src);
    }
  }

  // Clear the pointers.
  {
    GDValue dest;

    {
      GDValue src(GDVMap{{1,2}});
      GDValueParser p(src);

      p.checkIsMap();

      // Proactive safety measure: clear the parser.  This leaves the
      // parser in an invalid but somewhat safer state than when we just
      // move the source object out from under it.
      p.clearParserPointers();

      dest = std::move(src);
    }
  }
}


void test_optional()
{
  gdvnTestRoundtripEq(std::optional<int>(),  "null");
  gdvnTestRoundtripEq(std::optional<int>(3), "3");

  // Interestingly, `nullopt_t` does not allow `operator==`.
  gdvnTestRoundtrip  (std::nullopt,          "null");
}


// Make sure an ordered map can be treated like a map.
void test_orderedMapAsMap()
{
  GDValue v{GDVOrderedMap{
    { 1, 2 },
    { "x"_sym, "exs" },
  }};
  GDValueParser p(v);

  p.checkIsPOMap();
  EXPECT_ERROR_SUBSTR(p.checkIsMap(),
    "Expected map, not ordered map.");

  xassert(p.mapContains(1));
  xassert(!p.mapContains(2));

  EXPECT_EQ(p.mapGetKeyAt(1).getValue().smallIntegerGet(), 1);
  EXPECT_EQ(p.mapGetValueAt(1).getValue().smallIntegerGet(), 2);

  xassert(p.mapContainsSym("x"));
  EXPECT_EQ(p.mapGetValueAtSym("x").getValue().stringGet(), "exs");
}


class ErrorHandler : public HandleXGDValueError {
public:      // data
  // The error details.
  std::string m_path;
  std::string m_message;

public:      // methods
  virtual void handle(GDValueParser const &p, XGDValueError &x)
  {
    // Should only get here once.
    xassert(m_path.empty());

    m_path = x.m_path;
    m_message = x.m_message;
  }
};


void test_errorHandler()
{
  ErrorHandler errorHandler;

  GDValue value = fromGDVN("{ 1: Data{x:1 y:2}  2: Data{x:1 /*no y*/} }");
  GDValueParser parser(value);
  parser.m_errorHandler = &errorHandler;

  // This should not throw, instead it should drop element 2.
  std::map<int, Data> m = gdvpTo<std::map<int, Data>>(parser);
  EXPECT_EQ(errorHandler.m_path, "<top>.2");
  EXPECT_EQ(errorHandler.m_message,
    "Expected map to have key y, but it does not.");
  EXPECT_EQ_GDV(m, fromGDVN("{ 1: Data{x:1 y:2} }"));
}


void test_integerGetAs()
{
  TEST_CASE(__func__);

  {
    GDValue n(123);
    GDValueParser p(n);
    EXPECT_EQ(p.integerGetAs<int>(), 123);
    EXPECT_EQ(p.integerGetAs<std::int8_t>(), 123);
  }

  {
    GDValue n(123456);
    GDValueParser p(n);
    EXPECT_EQ(p.integerGetAs<int>(), 123456);
    EXPECT_EXN_SUBSTR(p.integerGetAs<std::int8_t>(),
      XGDValueError, "Source value 123456 of type");
  }

  {
    GDValue n(GDVInteger::fromDigits("1234567890123456789"));
    GDValueParser p(n);
    EXPECT_EXN_SUBSTR(p.integerGetAs<int>(),
      XGDValueError, "Source value 1234567890123456789 of type");
  }

  {
    GDValue n(GDVInteger::fromDigits("12345678901234567890123456789"));
    GDValueParser p(n);
    EXPECT_EXN_SUBSTR(p.integerGetAs<int>(),
      XGDValueError,
      "Attempted to convert the GDVInteger value "
      "12345678901234567890123456789 to a signed 32-bit integer type, "
      "but it does not fit.");
  }
}


void test_checkContainerSize()
{
  GDValue t(GDVTuple{1, 2});
  GDValueParser p(t);

  p.checkContainerSize(2);

  EXPECT_EXN_SUBSTR(p.checkContainerSize(1),
    XGDValueError,
    "Expected container to have 1 elements, "
    "but it instead has 2 elements.");
}


// Also test `checkTupleSize`.
void test_checkTaggedTupleSize()
{
  {
    GDValue t(GDVK_TAGGED_TUPLE, "mytag"_sym);
    t.tupleSet(GDVTuple{1, 2, 3});
    GDValueParser p(t);

    p.checkTupleSize(3);
    p.checkTaggedTupleSize("mytag", 3);

    EXPECT_EXN_SUBSTR(p.checkTaggedTupleSize("othertag", 3),
      XGDValueError,
      "Expected container to have tag othertag, "
      "but it instead has tag mytag.");

    EXPECT_EXN_SUBSTR(p.checkTaggedTupleSize("mytag", 2),
      XGDValueError,
      "Expected container to have 2 elements, "
      "but it instead has 3 elements.");
  }

  {
    GDValue t(GDVTuple{1, 2, 3});
    GDValueParser p(t);

    EXPECT_EXN_SUBSTR(
      p.checkTaggedTupleSize("mytag", 3),
      XGDValueError,
      "Expected tagged tuple, not tuple.");

    p.checkTupleSize(3);
    EXPECT_EXN_SUBSTR(
      p.checkTupleSize(2),
      XGDValueError,
      "Expected container to have 2 elements, "
      "but it instead has 3 elements.");
  }
}


// Test `GDValue` <-> `std::tuple`.
void test_tuple()
{
  using Tuple = std::tuple<int, std::string, Data>;

  {
    // `std::tuple` -> `GDValue`
    Tuple tup(3, "foo", Data(4,5));
    GDValue v(toGDValue(tup));
    EXPECT_EQ(v.asString(), "(3 \"foo\" Data{x:4 y:5})");

    // `GDValue` -> `std::tuple`
    {
      Tuple tup2(GDVP_TO(Tuple, v));
      EXPECT_EQ(toGDValue(tup2), v);
    }

    // Again with the convenience function.
    {
      Tuple tup2(gdvpToTuple<int, std::string, Data>(GDValueParser(v)));
      EXPECT_EQ(toGDValue(tup2), v);
    }

    // Both in one call.
    gdvnTestRoundtrip(tup, "(3 \"foo\" Data{x:4 y:5})");
  }

  // Try to parse an element with the wrong type.
  {
    GDValue v(fromGDVN("(3 4 Data{x:4 y:5})"));
    GDValueParser p(v);

    EXPECT_ERROR_SUBSTR(GDVP_TO(Tuple, v),
      "At GDV path <top>[1]: Expected string, not small integer.");
  }

  // Try to parse a tuple with too many elements.
  {
    GDValue v(fromGDVN("(3 \"foo\" Data{x:4 y:5} 6)"));
    GDValueParser p(v);

    EXPECT_ERROR_SUBSTR(GDVP_TO(Tuple, v),
      "At GDV path <top>: Expected container to have 3 elements, but it instead has 4 elements.");
  }

  // Try to parse a tuple with too few elements.
  {
    GDValue v(fromGDVN("(3)"));
    GDValueParser p(v);

    EXPECT_ERROR_SUBSTR(GDVP_TO(Tuple, v),
      "At GDV path <top>: Expected container to have 3 elements, but it instead has 1 elements.");
  }

  // We can parse tagged tuples too, discarding the tag.
  {
    GDValue v(fromGDVN("Tag(3 \"foo\" Data{x:4 y:5})"));
    GDValueParser p(v);
    Tuple tup(GDVP_TO(Tuple, v));

    EXPECT_EQ(toGDValue(tup).asString(),
      "(3 \"foo\" Data{x:4 y:5})");
  }
}


void test_Either()
{
  using E = Either<int, std::string>;

  {
    E e(3);
    GDValue v(toGDValue(e));
    EXPECT_EQ(v.asString(), "left(3)");

    E e2(gdvpTo<E>(GDValueParser(v)));
    EXPECT_EQ(e2.left(), 3);

    gdvnTestRoundtrip(e, "left(3)");
  }

  gdvnTestRoundtrip(E("hello"), "right(\"hello\")");

  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("4")),
    "At GDV path <top>: Expected tuple, not small integer.");
  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("left[3]")),
    "At GDV path <top>: Expected tuple, not tagged sequence.");
  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("foo(3)")),
    "At GDV path <top>: Expected tuple tag `left` or `right`, not foo.");
  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("(3)")),
    "At GDV path <top>: Expected tagged container, not tuple.");
  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("right(3 4)")),
    "At GDV path <top>: Expected container to have 1 elements, but it instead has 2 elements.");
  EXPECT_ERROR_SUBSTR(GDVP_TO(E, fromGDVN("right(3)")),
    "At GDV path <top>[0]: Expected string, not small integer.");
}


void test_binary64Float()
{
  gdvnTestRoundtrip(GDVBinary64Float(0.0), "0.0");
  gdvnTestRoundtrip(GDVBinary64Float(4.5), "4.5");

  gdvnTestRoundtrip(GDVBinary64Float(-0.0), "-0.0");

  // Moderately large and small values that have exact representations
  // in base 2.
  double large = std::pow(2.0, 100.0);
  double small = std::pow(2.0, -100.0);

  gdvnTestRoundtrip(GDVBinary64Float(large), "1.2676506002282294e+30");
  gdvnTestRoundtrip(GDVBinary64Float(small), "7.8886090522101181e-31");

  gdvnTestRoundtrip(
    std::tuple{GDVBinary64Float(4.5), 6, std::string("seven")},
    "(4.5 6 \"seven\")");
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
  test_null_unique_ptr();
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
  test_move_parsedObject();
  test_optional();
  test_orderedMapAsMap();
  test_errorHandler();
  test_integerGetAs();
  test_checkContainerSize();
  test_checkTaggedTupleSize();
  test_tuple();
  test_Either();
  test_binary64Float();
}


// EOF
