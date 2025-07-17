// gdvalue-json-test.cc
// Test de/serializing as JSON.

#include "smbase/gdvalue-json.h"       // module under test

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/ordered-map-ops.h"    // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ


using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


// Convert `v` to JSON and back.
void testCycle(GDValue const &v, char const *expectJSON)
{
  std::string actualJSON = gdvToJSON(v);
  EXPECT_EQ(actualJSON, expectJSON);

  GDValue v2(jsonToGDV(actualJSON));
  EXPECT_EQ(v2, v);
}


void testSymbol()
{
  testCycle(GDValue(), "null");
  testCycle(GDValue(true), "true");
  testCycle(GDValue(false), "false");

  testCycle(GDValue("something"_sym),
    R"({"_type":"symbol", "value":"something"})");
  testCycle(GDValue(""_sym),
    R"({"_type":"symbol", "value":""})");
}


void testInteger()
{
  testCycle(GDValue(0), "0");
  testCycle(GDValue(1), "1");
  testCycle(GDValue(3), "3");
  testCycle(GDValue(10), "10");
  testCycle(GDValue(-1), "-1");

  testCycle(GDValue(MOST_POSITIVE_JSON_INT),   "999999999");
  testCycle(GDValue(MOST_NEGATIVE_JSON_INT), "-1000000000");

  testCycle(GDValue(MOST_POSITIVE_JSON_INT+1),
    R"({"_type":"integer", "value":"1000000000"})");
  testCycle(GDValue(MOST_NEGATIVE_JSON_INT-1),
    R"({"_type":"integer", "value":"-1000000001"})");
}


void testString()
{
  testCycle(GDValue(""), R"("")");
  testCycle(GDValue("some string"), R"("some string")");

  // TODO: What about my syntactic extensions to string notation?
}


void testSequence()
{
  testCycle(
    GDValue(GDVSequence{
    }),
    "[]");

  testCycle(
    GDValue(GDVSequence{
      1, "two", 3
    }),
    R"([1, "two", 3])");

  testCycle(
    GDValue(GDVSequence{
      GDValue(true), "sym"_sym, "string"
    }),
    R"([true, {"_type":"symbol", "value":"sym"}, "string"])");
}


void testTuple()
{
  testCycle(
    GDValue(GDVTuple{}),
    R"({"_type":"tuple", "elements":[]})");

  testCycle(
    GDValue(GDVTuple{1}),
    R"({"_type":"tuple", "elements":[1]})");

  testCycle(
    GDValue(GDVTuple{1, "abc", GDVMap{}}),
    R"({"_type":"tuple", "elements":[1, "abc", {}]})");
}


void testSet()
{
  testCycle(
    GDValue(GDVSet{}),
    R"({"_type":"set", "elements":[]})");

  testCycle(
    GDValue(GDVSet{3,2,1}),
    R"({"_type":"set", "elements":[1, 2, 3]})");

  testCycle(
    GDValue(GDVSet{GDVMap{}}),
    R"({"_type":"set", "elements":[{}]})");
}


void testMap()
{
  testCycle(
    GDValue(GDVMap{}),
    "{}");

  testCycle(
    GDValue(GDVMap{
      { "a", 1 },
      { "b", 2 },
      { "c", 3 },
    }),
    R"({"a":1, "b":2, "c":3})");

  testCycle(
    GDValue(GDVMap{
      { "m", GDVMap{} },
    }),
    R"({"m":{}})");

  testCycle(
    GDValue(GDVMap{
      { 11, 1 },
      { 22, 2 },
      { 33, 3 },
    }),
    R"({"_type":"map", "elements":[[11, 1], [22, 2], [33, 3]]})");

  testCycle(
    GDValue(GDVMap{
      { "eleven", 1 },
      { 22, 2 },
      { 33, 3 },
    }),

    // Within the GDValue framework, strings sort after integers, so the
    // string key ends up last here.
    R"({"_type":"map", "elements":[[22, 2], [33, 3], ["eleven", 1]]})");

  testCycle(
    GDValue(GDVMap{
      { GDValue(true), 1 },
      { GDValue(GDVSet{}), 2 },
      { GDValue(GDVTuple{}), 3 },
      { GDValue(GDVMap{}), 4 },
      { GDValue(GDVSequence{}), 5 },
    }),
    R"({)"
      R"("_type":"map", )"
      R"("elements":[)"
        // The serialized order is symbol -> sequence -> tuple ->
        // set -> map, consistent with the GDValueKind enumeration.
        R"([true, 1], )"
        R"([[], 5], )"
        R"([{"_type":"tuple", "elements":[]}, 3], )"
        R"([{"_type":"set", "elements":[]}, 2], )"
        R"([{}, 4])"
      R"(])"
    R"(})"
  );

  testCycle(
    GDValue(GDVMap{
      { GDVMap{}, GDVMap{} },
    }),
    R"({"_type":"map", "elements":[[{}, {}]]})");
}


void testOrderedMap()
{
  testCycle(
    GDValue(GDVOrderedMap{}),
    R"({"_type":"ordered map", "elements":[]})");

  testCycle(
    GDValue(GDVOrderedMap{
      { "one", 1 },
      { 2, "two" },
    }),
    R"({"_type":"ordered map", "elements":[["one", 1], [2, "two"]]})");

  testCycle(
    GDValue(GDVOrderedMap{
      { GDVMap{}, GDVMap{} },
    }),
    R"({"_type":"ordered map", "elements":[[{}, {}]]})");
}


void testTaggedSequence()
{
  testCycle(
    GDValue(GDVTaggedSequence{"Foo"_sym, {
    }}),
    R"({"_type":"sequence", "elements":[], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedSequence{"Bar"_sym, {
      1, "two", 3
    }}),
    R"({"_type":"sequence", "elements":[1, "two", 3], "tag":"Bar"})");

  testCycle(
    GDValue(GDVTaggedSequence{"Foo"_sym, {
      GDValue(true), "sym"_sym, "string"
    }}),
    R"({"_type":"sequence", "elements":[)"
      R"(true, {"_type":"symbol", "value":"sym"}, "string")"
    R"(], "tag":"Foo"})");
}


void testTaggedTuple()
{
  testCycle(
    GDValue(GDVTaggedTuple{"Foo"_sym, {}}),
    R"({"_type":"tuple", "elements":[], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedTuple{"Foo"_sym, {1}}),
    R"({"_type":"tuple", "elements":[1], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedTuple{"Foo"_sym, {1, "abc", GDVMap{}}}),
    R"({"_type":"tuple", "elements":[1, "abc", {}], "tag":"Foo"})");
}


void testTaggedSet()
{
  testCycle(
    GDValue(GDVTaggedSet{"FooBar"_sym, {}}),
    R"({"_type":"set", "elements":[], "tag":"FooBar"})");

  testCycle(
    GDValue(GDVTaggedSet{"Foo"_sym, {3,2,1}}),
    R"({"_type":"set", "elements":[1, 2, 3], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedSet{"Foo"_sym, {GDVMap{}}}),
    R"({"_type":"set", "elements":[{}], "tag":"Foo"})");
}


void testTaggedMap()
{
  testCycle(
    GDValue(GDVTaggedMap{"Foo"_sym, {}}),
    R"({"_type":"map", "elements":[], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedMap{"Foo"_sym, {
      { "a", 1 },
      { "b", 2 },
      { "c", 3 },
    }}),
    R"({"_type":"map", "elements":[["a", 1], ["b", 2], ["c", 3]], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedMap{"Foo"_sym, {
      { "m", GDVMap{} },
    }}),
    R"({"_type":"map", "elements":[["m", {}]], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedMap{"Foo"_sym, {
      { 11, 1 },
      { 22, 2 },
      { 33, 3 },
    }}),
    R"({"_type":"map", "elements":[[11, 1], [22, 2], [33, 3]], "tag":"Foo"})");
}


void testTaggedOrderedMap()
{
  testCycle(
    GDValue(GDVTaggedOrderedMap{"Foo"_sym, {}}),
    R"({"_type":"ordered map", "elements":[], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedOrderedMap{"Foo"_sym, {
      { "one", 1 },
      { 2, "two" },
    }}),
    R"({"_type":"ordered map", "elements":[["one", 1], [2, "two"]], "tag":"Foo"})");

  testCycle(
    GDValue(GDVTaggedOrderedMap{"Foo"_sym, {
      { GDVMap{}, GDVMap{} },
    }}),
    R"({"_type":"ordered map", "elements":[[{}, {}]], "tag":"Foo"})");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_gdvalue_json()
{
  testSymbol();
  testString();
  testSequence();
  testTuple();
  testInteger();
  testSet();
  testMap();
  testOrderedMap();
  testTaggedSequence();
  testTaggedTuple();
  testTaggedSet();
  testTaggedMap();
  testTaggedOrderedMap();
}


// EOF
