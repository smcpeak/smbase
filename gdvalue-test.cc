// gdvalue-test.cc
// Tests for gdvalue.

#include "gdvalue.h"                   // module under test

// this dir
#include "gdvsymbol.h"                 // gdv::GDVSymbol
#include "gdvalue-reader-exception.h"  // GDValueReaderException

// libc++
#include <cassert>                     // assert
#include <cstdlib>                     // std::{atoi, exit}
#include <iostream>                    // std::cout

using std::cout;
using namespace gdv;


// Check that 'ser' deserializes to 'expect'.
static void checkParse(GDValue const &expect, std::string const &ser)
{
  try {
    GDValue actual(GDValue::readFromString(ser));

    if (actual != expect) {
      cout << "During checkParse, found mismatch:\n"
           << "---- expect ----\n"
           << expect.asLinesString()
           << "---- ser ----\n"
           << ser << "\n"
           << "---- actual ----\n"
           << actual.asLinesString();
      std::exit(2);
    }
  }
  catch (GDValueReaderException const &e) {
    cout << "During checkParse, caught exception:\n"
         << "---- expect ----\n"
         << expect.asLinesString()
         << "---- ser ----\n"
         << ser << "\n";
    throw e;
  }
}


// Serialize and deserialize 'value', a couple ways, expecting
// equivalence.
static void testSerializeRoundtrip(GDValue const &value)
{
  // Compact form.
  checkParse(value, value.asString());

  // Indented form.
  checkParse(value, value.asLinesString());
}


static void testNull()
{
  GDValue v;
  cout << "null: " << v << "\n";
  assert(v.asString() == "null");
  assert(v.size() == 0);
  assert(v.empty());
  assert(v.isNull());
  assert(v.getKind() == GDVK_NULL);

  GDValue v2;
  assert(v == v2);
  assert(v2.asString() == "null");
  assert(v2.getKind() == GDVK_NULL);

  v2.clear();
  assert(v == v2);
  assert(v2.getKind() == GDVK_NULL);

  v2 = GDValue(GDVK_NULL);
  assert(v == v2);

  v2 = GDValue();
  assert(v == v2);

  GDValue v3(GDVK_NULL);
  assert(v == v3);

  testSerializeRoundtrip(v);
}


static void testBool()
{
  GDValue dTrue(GDValue::BoolTag, true);
  cout << "true: " << dTrue << "\n";
  assert(dTrue.asString() == "true");
  assert(dTrue.size() == 1);
  assert(!dTrue.empty());
  assert(dTrue.getKind() == GDVK_BOOL);
  assert(dTrue.isBool());
  assert(dTrue.boolGet() == true);

  GDValue dFalse(GDValue::BoolTag, false);
  cout << "false: " << dFalse << "\n";
  assert(dFalse.asString() == "false");
  assert(dFalse.size() == 1);
  assert(!dFalse.empty());
  assert(dFalse.getKind() == GDVK_BOOL);
  assert(dFalse.boolGet() == false);

  assert(dTrue != dFalse);
  assert(dTrue > dFalse);

  GDValue dNull;

  assert(dTrue != dNull);
  assert(dFalse != dNull);

  assert(dTrue > dNull);
  assert(dFalse > dNull);

  testSerializeRoundtrip(dTrue);
  testSerializeRoundtrip(dFalse);
}


static void testInteger()
{
  GDValue d0(0);
  cout << "0: " << d0 << "\n";
  assert(d0.asString() == "0");
  assert(d0.size() == 1);
  assert(!d0.empty());
  assert(d0.getKind() == GDVK_INTEGER);
  assert(d0.isInteger());
  assert(d0.integerGet() == 0);

  GDValue d1(1);
  assert(d1.asString() == "1");
  assert(d1.size() == 1);
  assert(!d1.empty());
  assert(d1.getKind() == GDVK_INTEGER);
  assert(d1.integerGet() == 1);

  assert(d0 < d1);
  assert(GDValue() < d0);

  testSerializeRoundtrip(d0);
  testSerializeRoundtrip(d1);

  testSerializeRoundtrip(GDValue(1234567890));
  testSerializeRoundtrip(GDValue(-1234567890));
}


static void testSymbol()
{
  GDValue dSym1(GDVSymbol("sym1"));
  cout << "sym1: " << dSym1 << "\n";
  assert(dSym1.asString() == "sym1");
  assert(dSym1.size() == 1);
  assert(!dSym1.empty());
  assert(dSym1.getKind() == GDVK_SYMBOL);
  assert(dSym1.isSymbol());
  assert(dSym1.symbolGet() == GDVSymbol("sym1"));
  testSerializeRoundtrip(dSym1);

  GDValue dSym2(GDVSymbol("sym2"));
  assert(dSym2.asString() == "sym2");
  assert(dSym2.size() == 1);
  assert(!dSym2.empty());
  assert(dSym2.getKind() == GDVK_SYMBOL);
  assert(dSym2.symbolGet() == GDVSymbol("sym2"));
  testSerializeRoundtrip(dSym2);

  assert(dSym1 < dSym2);
  assert(GDValue() < dSym1);

  dSym2.clear();
  assert(dSym2.isNull());
  assert(dSym2.getKind() == GDVK_NULL);

  testSerializeRoundtrip(dSym2);
}


static void testString()
{
  // Get initial counts.
  unsigned initCtSetCopy = GDValue::s_ct_stringSetCopy;
  unsigned initCtSetMove = GDValue::s_ct_stringSetMove;
  unsigned initCtCtorCopy = GDValue::s_ct_stringCtorCopy;
  unsigned initCtCtorMove = GDValue::s_ct_stringCtorMove;

  // Check counts.
  #define CHECK_COUNTS(setCopy, setMove, ctorCopy, ctorMove)           \
    assert(GDValue::s_ct_stringSetCopy - initCtSetCopy == setCopy);    \
    assert(GDValue::s_ct_stringSetMove - initCtSetMove == setMove);    \
    assert(GDValue::s_ct_stringCtorCopy - initCtCtorCopy == ctorCopy); \
    assert(GDValue::s_ct_stringCtorMove - initCtCtorMove == ctorMove);

  GDValue dStr1(GDVString("str1"));
  CHECK_COUNTS(0, 1, 0, 1)
  cout << "str1: " << dStr1 << "\n";
  assert(dStr1.asString() == "\"str1\"");
  assert(dStr1.size() == 1);
  assert(!dStr1.empty());
  assert(dStr1.getKind() == GDVK_STRING);
  assert(dStr1.isString());
  assert(dStr1.stringGet() == GDVString("str1"));

  GDValue dStr2(GDVString("str2"));
  CHECK_COUNTS(0, 2, 0, 2)
  assert(dStr2.asString() == "\"str2\"");
  assert(dStr2.size() == 1);
  assert(!dStr2.empty());
  assert(dStr2.getKind() == GDVK_STRING);
  assert(dStr2.stringGet() == GDVString("str2"));

  assert(dStr1 < dStr2);
  assert(GDValue() < dStr1);

  dStr2.clear();
  assert(dStr2.isNull());
  assert(dStr2.getKind() == GDVK_NULL);

  GDVString str1("str1");
  dStr2.stringSet(str1);     // 'set' without 'ctor'
  CHECK_COUNTS(1, 2, 0, 2)

  assert(dStr1 == dStr2);

  {
    GDValue const &dv = dStr1;
    cout << "string begin/end const iteration: ";
    for (auto it = dv.stringBegin(); it != dv.stringEnd(); ++it) {
      cout << *it;
    }
    cout << "\n";

    cout << "string iterable const iteration: ";
    for (auto const &c : dStr1.stringIterable()) {
      cout << c;
    }
    cout << "\n";
  }

  {
    GDValue &dv = dStr1;
    cout << "string begin/end non-const iteration: ";
    for (auto it = dv.stringBegin(); it != dv.stringEnd(); ++it) {
      ++(*it);
      cout << *it;
    }
    cout << "\n";
    cout << "again: " << dStr1 << "\n";
    assert(dStr1.stringGet() == "tus2");

    cout << "string iterable non-const iteration: ";
    for (auto &c : dStr1.stringIterable()) {
      --c;
      cout << c;
    }
    cout << "\n";
    assert(dStr1.stringGet() == "str1");
  }

  #undef CHECK_COUNTS

  testSerializeRoundtrip(dStr1);
  testSerializeRoundtrip(GDVString(""));

  {
    std::ostringstream oss;
    for (int i=0; i < 256; ++i) {
      oss << (char)i;
    }
    testSerializeRoundtrip(GDVString(oss.str()));
  }
}


static void testVector()
{
  GDValue v1(GDVK_VECTOR);
  cout << "empty vec: " << v1 << "\n";
  assert(v1.asString() == "[]");
  assert(v1.size() == 0);
  assert(v1.empty());
  assert(v1.getKind() == GDVK_VECTOR);
  assert(v1.isVector());
  assert(v1.vectorGet() == GDVVector());
  testSerializeRoundtrip(v1);

  GDValue v2((GDVVector()));

  assert(v1 == v2);

  GDVVector vec1b3{GDValue(1), GDValue("b"), GDValue(3)};
  GDValue v3(vec1b3);
  cout << "three-element vec: " << v3 << "\n";
  assert(v3.asString() == "[1 \"b\" 3]");
  assert(v3.size() == 3);
  assert(!v3.empty());
  assert(v3.getKind() == GDVK_VECTOR);
  assert(v3.vectorGet() == vec1b3);
  assert(v1 < v3);
  testSerializeRoundtrip(v3);

  v1.vectorAppend(GDValue(-1));
  assert(v1.asString() == "[-1]");
  assert(v1 < v3);

  v3.vectorAppend(GDValue("four"));
  assert(v3.asString() == R"([1 "b" 3 "four"])");

  v1.vectorResize(3);
  assert(v1.asString() == "[-1 null null]");

  v3.vectorResize(3);
  assert(v3.asString() == R"([1 "b" 3])");

  v1.vectorSetValueAt(1, v3);
  assert(v1.asString() == R"([-1 [1 "b" 3] null])");

  v1.vectorSetValueAt(4, GDValue(5));
  cout << v1 << "\n";
  assert(v1.asString() == R"([-1 [1 "b" 3] null null 5])");
  testSerializeRoundtrip(v1);

  assert(v1.vectorGetValueAt(1) == v3);

  {
    GDVIndex i = 0;
    for (GDValue const &value : v1.vectorIterableC()) {
      assert(value == v1.vectorGetValueAt(i));
      ++i;
    }
  }

  for (GDValue &value : v1.vectorIterable()) {
    if (value.isInteger()) {
      value.integerSet(value.integerGet() + 1);
    }
  }
  assert(v1.asString() == R"([0 [1 "b" 3] null null 6])");

  v1.vectorClear();
  assert(v1 == v2);
  assert(v1.empty());
  testSerializeRoundtrip(v1);
}


static void testSet()
{
  GDValue v1((GDVSet()));
  cout << "empty set: " << v1 << "\n";
  assert(v1.asString() == "{{}}");
  assert(v1.size() == 0);
  assert(v1.empty());
  assert(v1.getKind() == GDVK_SET);
  assert(v1.isSet());
  assert(v1.setGet() == GDVSet());
  testSerializeRoundtrip(v1);

  GDValue v2(v1);
  assert(v1 == v2);

  v2.setInsert(GDValue(1));
  assert(v2.setContains(GDValue(1)));
  assert(v2.size() == 1);
  assert(v1 < v2);

  v2.setInsert(GDValue(2));
  assert(v2.asString() == "{{1 2}}");
  testSerializeRoundtrip(v2);

  v2.setRemove(GDValue(1));
  assert(v2.asString() == "{{2}}");

  v2.setClear();
  assert(v2.asString() == "{{}}");
  assert(v1 == v2);
  testSerializeRoundtrip(v2);

  v2 = GDValue(GDVSet{
         GDValue("x"),
         GDValue(10),
         GDValue(GDVVector{
           GDValue(2),
           GDValue(3),
           GDValue(4),
         }),
       });
  cout << v2 << "\n";
  assert(v2.asString() == R"({{10 "x" [2 3 4]}})");
  testSerializeRoundtrip(v2);
}


static void testMap()
{
  GDValue v1((GDVMap()));
  cout << "empty map: " << v1 << "\n";
  assert(v1.asString() == "{}");
  assert(v1.size() == 0);
  assert(v1.empty());
  assert(v1.getKind() == GDVK_MAP);
  assert(v1.isMap());
  assert(v1.mapGet() == GDVMap());
  testSerializeRoundtrip(v1);

  GDValue v2(v1);
  assert(v1 == v2);

  v2.mapSetValueAt(GDValue("one"), GDValue(1));
  assert(v2.size() == 1);
  assert(v2.mapGetValueAt(GDValue("one")) == GDValue(1));
  cout << v2 << "\n";
  assert(v2.asString() == R"({"one":1})");
  assert(v2.mapContains(GDValue("one")));
  assert(v2 > v1);
  testSerializeRoundtrip(v2);

  v2.mapSetValueAt(GDValue("one"), GDValue(2));
  assert(v2.asString() == R"({"one":2})");
  assert(v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.mapSetValueAt(GDValue("two"), GDValue(2));
  assert(v2.asString() == R"({"one":2 "two":2})");
  assert(v2.size() == 2);
  testSerializeRoundtrip(v2);

  v2.mapRemoveKey(GDValue("one"));
  assert(v2.asString() == R"({"two":2})");
  assert(!v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.mapClear();
  assert(v1 == v2);
  testSerializeRoundtrip(v2);

  v2 = GDValue(GDVMap{
         GDVMapEntry(GDValue("a"), GDValue(1)),
         GDVMapEntry(GDValue(2), GDValue(3)),
         GDVMapEntry(
           GDValue(GDVVector({        // Use a vector as a key.
             GDValue(10),
             GDValue(11)
           })),
           GDValue(GDVSymbol("ten_eleven"))
         )
       });
  cout << v2 << "\n";
  assert(v2.asString() == "{2:3 \"a\":1 [10 11]:ten_eleven}");
  testSerializeRoundtrip(v2);

  assert(v2.mapGetValueAt(
           GDValue(GDVVector({GDValue(10), GDValue(11)}))) ==
         GDValue(GDVSymbol("ten_eleven")));
}


// Print a little ruler to help judge the behavior.
static void printRuler(int width)
{
  if (width <= 0) {
    return;
  }

  for (int i=0; i < width; ++i) {
    if (i == 0 || i == width-1) {
      cout << '|';
    }
    else {
      cout << '-';
    }
  }
  cout << "\n";
}


// This is an ad-hoc collection of things to print, meant for
// interactive experimentation and verification.
static void testPrettyPrint(int width)
{
  cout << "pretty print target width: " << width << "\n";
  printRuler(width);

  GDValue v(GDVVector{1,2,3});
  v.writeLines(cout, GDValueWriteOptions()
                       .setTargetLineWidth(width));

  GDValue m2(GDVMap{
               {GDVSymbol("a"), v},
               {v, v},
             });
  m2.writeLines(cout, GDValueWriteOptions()
                        .setTargetLineWidth(width));

  v = GDVVector{
    1,
    "hello",
    GDVVector{2,3,4},
    GDVSet{
      "x",
      10,
      GDVVector{2,3,4},
    }
  };
  v.writeLines(cout, GDValueWriteOptions()
                      .setTargetLineWidth(width));

  GDValue m(GDVMap{
              { 8, 9},
              {10,11},
              {12,13},
              {14,15},
            });
  m.writeLines(cout, GDValueWriteOptions()
                      .setTargetLineWidth(width));

  GDValue s(GDVSet{"eins", "zwei", "drei"});

  v = GDValue(GDVMap{
        {GDVSymbol("v"),        v},
        {"four",                4},
        {"x",                   m},
        {m,                     m},
        {GDVSymbol("counting"), s},
      });
  v.writeLines(cout, GDValueWriteOptions()
                      .setTargetLineWidth(width));

  v = GDValue(GDVMap{ {1,2} });
  v = GDValue(GDVMap{ {v,s} });
  v.writeLines(cout, GDValueWriteOptions()
                      .setTargetLineWidth(width));

  printRuler(width);
}


static std::string linesStringFor(GDValue const &value, int targetWidth)
{
  return value.asLinesString(GDValueWriteOptions()
                               .setTargetLineWidth(targetWidth));
}


static void checkLinesStringFor(
  GDValue const &value,
  int targetWidth,
  std::string expect)
{
  std::string actual = linesStringFor(value, targetWidth);
  if (actual != expect) {
    cout << "expect:\n" << expect << "\n";
    cout << "actual:\n" << actual << "\n";
    cout.flush();
  }
  assert(actual == expect);
}


// Test printing specific structures against expectation.
//
// This test mechanism is very crude.  I should be able to replace it
// with something better once I have text deserialization.
static void testPrettyExpect()
{
  GDValue counting(GDVSet{
    GDValue("ein"),
    GDValue("zwei"),
    GDValue("drei")
  });
  GDValue oneTwo = GDValue(GDVMap{
    GDVMapEntry(GDValue(1), GDValue(2))
  });
  GDValue m(GDVMap{
    GDVMapEntry(oneTwo, counting)
  });

  checkLinesStringFor(m, 9, R"({
  {1:2}:
    {{
      "drei"
      "ein"
      "zwei"
    }}
}
)");

  checkLinesStringFor(m, 10, R"({
  {1:2}:{{
    "drei"
    "ein"
    "zwei"
  }}
}
)");

  checkLinesStringFor(m, 11, R"({
  {1:2}:{{
    "drei"
    "ein"
    "zwei"
  }}
}
)");

  checkLinesStringFor(m, 26, R"({
  {1:2}:{{
    "drei"
    "ein"
    "zwei"
  }}
}
)");

  checkLinesStringFor(m, 27, R"({
  {1:2}:
    {{"drei" "ein" "zwei"}}
}
)");
}


static void testSyntaxErrors()
{
  // TODO: Write tests that exercise syntax errors.
}


// Called from unit-tests.cc.
void test_gdvalue()
{
  if (char const *widthStr = std::getenv("GDVALUE_TEST_WIDTH")) {
    // With envvar set, treat it as the target width for the
    // pretty-print tests so I can interactively experiment.
    int width = std::atoi(widthStr);

    testPrettyPrint(width);
  }
  else {
    testNull();
    testBool();
    testInteger();
    testSymbol();
    testString();
    testVector();
    testSet();
    testMap();
    testSyntaxErrors();

    // Some interesting values for the particular data used.
    testPrettyPrint(0);
    testPrettyPrint(19);
    testPrettyPrint(20);
    testPrettyPrint(37);
    testPrettyPrint(38);

    testPrettyExpect();
  }

  // Ctor and dtor calls should be balanced.
  assert(GDValue::countConstructorCalls() == GDValue::s_ct_dtor);
}


// EOF
