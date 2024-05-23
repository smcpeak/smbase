// gdvalue-test.cc
// Tests for gdvalue.

#include "gdvalue.h"                   // module under test

// this dir
#include "gdvsymbol.h"                 // gdv::GDVSymbol
#include "gdvalue-reader-exception.h"  // GDValueReaderException
#include "sm-test.h"                   // EXPECT_EQ, EXPECT_MATCHES_REGEX
#include "strutil.h"                   // hasSubstring
#include "string-utils.h"              // doubleQuote

// libc++
#include <cassert>                     // assert
#include <cstdint>                     // INT64_C
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


static void testSequence()
{
  GDValue v1(GDVK_SEQUENCE);
  cout << "empty seq: " << v1 << "\n";
  assert(v1.asString() == "[]");
  assert(v1.size() == 0);
  assert(v1.empty());
  assert(v1.getKind() == GDVK_SEQUENCE);
  assert(v1.isSequence());
  assert(v1.sequenceGet() == GDVSequence());
  testSerializeRoundtrip(v1);

  GDValue v2((GDVSequence()));

  assert(v1 == v2);

  GDVSequence seq1b3{GDValue(1), GDValue("b"), GDValue(3)};
  GDValue v3(seq1b3);
  cout << "three-element seq: " << v3 << "\n";
  assert(v3.asString() == "[1 \"b\" 3]");
  assert(v3.size() == 3);
  assert(!v3.empty());
  assert(v3.getKind() == GDVK_SEQUENCE);
  assert(v3.sequenceGet() == seq1b3);
  assert(v1 < v3);
  testSerializeRoundtrip(v3);

  v1.sequenceAppend(GDValue(-1));
  assert(v1.asString() == "[-1]");
  assert(v1 < v3);

  v3.sequenceAppend(GDValue("four"));
  assert(v3.asString() == R"([1 "b" 3 "four"])");

  v1.sequenceResize(3);
  assert(v1.asString() == "[-1 null null]");

  v3.sequenceResize(3);
  assert(v3.asString() == R"([1 "b" 3])");

  v1.sequenceSetValueAt(1, v3);
  assert(v1.asString() == R"([-1 [1 "b" 3] null])");

  v1.sequenceSetValueAt(4, GDValue(5));
  cout << v1 << "\n";
  assert(v1.asString() == R"([-1 [1 "b" 3] null null 5])");
  testSerializeRoundtrip(v1);

  assert(v1.sequenceGetValueAt(1) == v3);

  {
    GDVIndex i = 0;
    for (GDValue const &value : v1.sequenceIterableC()) {
      assert(value == v1.sequenceGetValueAt(i));
      ++i;
    }
  }

  for (GDValue &value : v1.sequenceIterable()) {
    if (value.isInteger()) {
      value.integerSet(value.integerGet() + 1);
    }
  }
  assert(v1.asString() == R"([0 [1 "b" 3] null null 6])");

  v1.sequenceClear();
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
         GDValue(GDVSequence{
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
           GDValue(GDVSequence({        // Use a sequence as a key.
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
           GDValue(GDVSequence({GDValue(10), GDValue(11)}))) ==
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

  GDValue v(GDVSequence{1,2,3});
  v.writeLines(cout, GDValueWriteOptions()
                       .setTargetLineWidth(width));

  GDValue m2(GDVMap{
               {GDVSymbol("a"), v},
               {v, v},
             });
  m2.writeLines(cout, GDValueWriteOptions()
                        .setTargetLineWidth(width));

  v = GDVSequence{
    1,
    "hello",
    GDVSequence{2,3,4},
    GDVSet{
      "x",
      10,
      GDVSequence{2,3,4},
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


static void testOneErrorSubstrOrRegex(
  char const *input,
  int expectLine,
  int expectColumn,
  char const * NULLABLE expectErrorSubstring,
  char const * NULLABLE expectErrorRegex)
{
  try {
    try {
      GDValue::readFromString(input);
      xfailure("should have failed");
    }
    catch (GDValueReaderException &e) {
      EXPECT_EQ(e.m_location.m_lc.m_line, expectLine);
      EXPECT_EQ(e.m_location.m_lc.m_column, expectColumn);
      if (expectErrorSubstring) {
        EXPECT_HAS_SUBSTRING(e.m_syntaxError, expectErrorSubstring);
      }
      else {
        EXPECT_MATCHES_REGEX(e.m_syntaxError, expectErrorRegex);
      }
    }
  }
  catch (xBase &x) {
    x.prependContext(stringb("input=" << doubleQuote(input)));
    throw;
  }
}


static void testOneErrorSubstr(
  char const *input,
  int expectLine,
  int expectColumn,
  char const *expectErrorSubstring)
{
  testOneErrorSubstrOrRegex(
    input,
    expectLine,
    expectColumn,
    expectErrorSubstring,
    nullptr /*regex*/);
}


static void testOneErrorRegex(
  char const *input,
  int expectLine,
  int expectColumn,
  char const *expectErrorRegex)
{
  testOneErrorSubstrOrRegex(
    input,
    expectLine,
    expectColumn,
    nullptr /*substr*/,
    expectErrorRegex);
}


static void testSyntaxErrors()
{
  // This test is meant to correspond to gdvalue-reader.cc, exercising
  // each of the error paths evident in each function.  Basically, I
  // search for "err" and then target each occurrence.

  // errUnexpectedChar
  testOneErrorSubstr("", 1, 1, "end of file");
  testOneErrorSubstr(";", 1, 1, "';'");
  testOneErrorSubstr("\001", 1, 1, "(0x01)");

  // processExpectChar: TODO: Look for call sites.

  // readCharNotEOF: TODO: Callers.

  // readExpectEOF
  testOneErrorSubstr("1 2", 1, 3, "only have one value");

  // skipWhitespaceAndComments
  testOneErrorRegex(" /", 1, 3, "end of file.*after '/'");
  testOneErrorRegex("/-", 1, 2, "'-'.*after '/'");

  // skipCStyleComment
  testOneErrorSubstr("/*/1", 1, 5,
    R"(inside "/*" comment, looking for corresponding "*/")");
  testOneErrorSubstr("/*/*", 1, 5,
    R"(inside "/*" comment, nested inside 1 other comments of the same kind, looking for corresponding "*/")");
  testOneErrorSubstr("/*/**/", 1, 7,
    R"(inside "/*" comment, which contains 1 child comments, looking for corresponding "*/")");
  testOneErrorSubstr("/*/**//**/", 1, 11,
    R"(inside "/*" comment, which contains 2 child comments, looking for corresponding "*/")");
  testOneErrorSubstr("/*/*/**//**/", 1, 13,
    R"(inside "/*" comment, nested inside 1 other comments of the same kind, which contains 2 child comments, looking for corresponding "*/")");

  // readNextMap: looking for '}'.
  testOneErrorRegex("{]", 1, 2,
    "']'.*looking for '}'");
  testOneErrorSubstr("{", 1, 2,
    "Unexpected end of file after '{'");
  testOneErrorRegex("{1:2", 1, 5,
    "end of file.*looking for '}'");
  testOneErrorRegex("{1:2]", 1, 5,
    "']'.*looking for '}'");

  // readNextMap: looking for ':'.
  testOneErrorRegex("{1", 1, 3,
    "end of file.*looking for ':'");
  testOneErrorRegex("{1}", 1, 3,
    "'}'.*looking for ':'");
  testOneErrorRegex("{1]", 1, 3,
    "']'.*looking for ':'");
  testOneErrorRegex("{1-", 1, 3,
    "'-' after a value");
  testOneErrorRegex("{1 2", 1, 4,
    "'2'.*looking for ':'");

  // readNextMap: looking for value after ':'.
  testOneErrorRegex("{1:", 1, 4,
    "end of file.*after ':'");
  testOneErrorRegex("{1 : ", 1, 6,
    "end of file.*after ':'");
  testOneErrorRegex("{1:]", 1, 4,
    "']'.*after ':'");
  testOneErrorRegex("{1:}", 1, 4,
    "'}'.*after ':'");
  testOneErrorRegex("{1::", 1, 4,
    "':'.*start of a value");
  testOneErrorRegex("{1: }", 1, 5,
    "'}'.*after ':'");

  // readNextMap: Duplicate map key.
  testOneErrorSubstr("{1:2 1:2}", 1, 6,
    "Duplicate map key: 1");
  testOneErrorSubstr("{1:2 3:4 1:2}", 1, 10,
    "Duplicate map key: 1");
  testOneErrorSubstr("{1:2 {4:4}:4 11:2 {4:4}:5}", 1, 19,
    "Duplicate map key: {4:4}");

  // readNextDQString: looking for closing '"'.
  testOneErrorRegex("\"", 1, 2,
    "end of file.*looking for closing '\"'");
  testOneErrorRegex("\"\\\"", 1, 4,
    "end of file.*looking for closing '\"'");
  testOneErrorRegex("\"\n", 2, 1,
    "end of file.*looking for closing '\"'");

  // readNextDQString: looking for character after backslash (1).
  testOneErrorRegex("\"\\", 1, 3,
    "end of file.*looking for character after '\\\\'");

  // TODO: More in readNextDQString after I fix it to match spec.

  // readNextDQString: looking for character after backslash (2).
  testOneErrorRegex("\"\\z", 1, 3,
    "'z'.*looking for the character after a '\\\\'");

  // readNextInteger: putbackAfterValue.
  testOneErrorRegex("1a", 1, 2,
    "'a'.*after a value");

  // For reference, the maximum uint64_t is 18446744073709551615.

  // readNextInteger: value too large.
  testOneErrorSubstr("12345678901234567890", 1, 20,
    "too large");

  // readNextSymbolOrSpecial: after value.
  testOneErrorSubstr("true[", 1, 5,
    "'[' after a value");

  // readNextValue: EOF after '{'.
  testOneErrorSubstr("{", 1, 2,
    "end of file after '{'");

  // readNextValue: Bad character at start of value.
  testOneErrorSubstr("(", 1, 1,
    "'(' while looking for the start of a value");

  // readExactlyOneValue: EOF or bad at start.
  testOneErrorSubstr("", 1, 1,
    "end of file while looking for the start of a value");
  testOneErrorSubstr("]", 1, 1,
    "']' while looking for the start of a value");
}


// Check that deserializing 'input' succeeds and yields 'expect'.
static void testOneDeserialize(
  char const *input,
  GDValue expect)
{
  try {
    GDValue actual = GDValue::readFromString(input);
    EXPECT_EQ(actual, expect);
  }
  catch (xBase &x) {
    x.prependContext(stringb("input=" << doubleQuote(input)));
    throw;
  }
}


static void testDeserialize()
{
  // Comma is whitespace.
  testOneDeserialize(",1", 1);

  // Check handling of whitespace and comments after the value.
  testOneDeserialize("1 ", 1);
  testOneDeserialize("1 \n\n", 1);
  testOneDeserialize("1 //", 1);
  testOneDeserialize("1 //\n//\n", 1);
  testOneDeserialize("1 /**/", 1);
  testOneDeserialize("1 /**/ ", 1);

  // Check that we don't somehow recognize comments inside strings.
  testOneDeserialize("\"/*\"", GDVString("/*"));

  // Confirm we can deserialize a value near the top of the range for a
  // signed 64-bit integer.
  testOneDeserialize("1234567890123456789", INT64_C(1234567890123456789));

  // TODO: Lots here.
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
    testSequence();
    testSet();
    testMap();
    testSyntaxErrors();
    testDeserialize();

    // Some interesting values for the particular data used.
    testPrettyPrint(0);
    testPrettyPrint(19);
    testPrettyPrint(20);
    testPrettyPrint(37);
    testPrettyPrint(38);

    testPrettyExpect();
  }

  // Ctor and dtor calls should be balanced.
  EXPECT_EQ(GDValue::countConstructorCalls(),
            GDValue::s_ct_dtor);
  EXPECT_EQ(GDVSymbol::s_numSymbolCtorCalls,
            GDVSymbol::s_numSymbolDtorCalls);
}


// EOF
