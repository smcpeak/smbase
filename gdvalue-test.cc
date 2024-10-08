// gdvalue-test.cc
// Tests for gdvalue.

// This file is in the public domain.

#include "gdvalue.h"                   // module under test

// this dir
#include "smbase/counting-ostream.h"   // nullOStream
#include "smbase/gdvsymbol.h"          // gdv::GDVSymbol
#include "smbase/ordered-map-ops.h"    // smbase::OrderedMap ctor, etc.
#include "smbase/reader.h"             // smbase::ReaderException
#include "smbase/save-restore.h"       // SAVE_RESTORE
#include "smbase/sm-file-util.h"       // SMFileUtil
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_MATCHES_REGEX, VPVAL, DIAG, verbose, tout
#include "smbase/strutil.h"            // hasSubstring
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/syserr.h"             // smbase::XSysError
#include "smbase/utf8-writer.h"        // smbase::utf8EncodeVector
#include "smbase/xassert.h"            // xassert

// libc++
#include <cstdint>                     // INT64_C
#include <cstdlib>                     // std::{atoi, exit}
#include <iostream>                    // std::cout
#include <string>                      // std::string

using namespace smbase;
using namespace gdv;

using std::cout;


// Throughout this file, there are exception handlers that only run if
// a test fails, so ignore them for coverage purposes:
// gcov-exception-lines-ignore


OPEN_ANONYMOUS_NAMESPACE


// Check that 'ser' deserializes to 'expect'.
void checkParse(GDValue const &expect, std::string const &ser)
{
  try {
    GDValue actual(GDValue::readFromString(ser));

    if (actual != expect) {
      // gcov-begin-ignore
      cout << "During checkParse, found mismatch:\n"
           << "---- expect ----\n"
           << expect.asLinesString()
           << "---- ser ----\n"
           << ser << "\n"
           << "---- actual ----\n"
           << actual.asLinesString();
      std::exit(2);
      // gcov-end-ignore
    }

    actual.selfCheck();
  }
  catch (ReaderException const &e) {
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
void testSerializeRoundtrip(GDValue const &value)
{
  value.selfCheck();

  // Compact form.
  checkParse(value, value.asString());

  // Indented form.
  checkParse(value, value.asLinesString());
}


void testNull()
{
  GDValue v;
  DIAG("null: " << v);
  xassert(v.asString() == "null");
  xassert(v.isNull());
  xassert(v.getKind() == GDVK_SYMBOL);
  v.selfCheck();

  GDValue v2;
  xassert(v == v2);
  xassert(v2.asString() == "null");
  xassert(v.isNull());
  xassert(v2.getKind() == GDVK_SYMBOL);

  v2.reset();
  xassert(v == v2);
  xassert(v.isNull());
  xassert(v2.getKind() == GDVK_SYMBOL);

  v2 = GDValue(GDVK_SYMBOL);
  xassert(v == v2);

  v2 = GDValue();
  xassert(v == v2);

  GDValue v3(GDVK_SYMBOL);
  xassert(v == v3);

  testSerializeRoundtrip(v);
}


void testBool()
{
  GDValue dTrue(GDValue::BoolTag, true);
  VPVAL(dTrue);
  xassert(dTrue.asString() == "true");
  xassert(dTrue.getKind() == GDVK_SYMBOL);
  xassert(dTrue.isBool());
  xassert(dTrue.boolGet() == true);

  GDValue dFalse(GDValue::BoolTag, false);
  VPVAL(dFalse);
  xassert(dFalse.asString() == "false");
  xassert(dFalse.getKind() == GDVK_SYMBOL);
  xassert(dFalse.isBool());
  xassert(dFalse.boolGet() == false);

  xassert(dTrue != dFalse);
  xassert(dTrue > dFalse);

  GDValue dNull;

  xassert(dTrue != dNull);
  xassert(dFalse != dNull);

  xassert(dTrue > dNull);
  xassert(dFalse < dNull);

  testSerializeRoundtrip(dTrue);
  testSerializeRoundtrip(dFalse);

  GDValue b = GDValue::makeBool(true);
  xassert(b == dTrue);
  xassert(GDValue::makeBool(false) == dFalse);

  // I now allow semi-implicit conversion with `toGDValue`.
  {
    GDValue v = toGDValue(true);
    v = toGDValue(false);
    bool b = true;
    v = toGDValue(b);
    EXPECT_EQ(v.asString(), "true");
  }

  // And now I'm trying to allow implicit conversion of exactly `bool`.
  {
    GDValue v1(true);
    GDValue v2(false);
    bool b = true;
    GDValue v3(b);
    EXPECT_EQ(v3.asString(), "true");
  }

  // But not these.  Each of these cases is exercised by the
  // `check-gdvalue-errs` target in `Makefile`.
  {
    #if ERRNUM == 1
      void *p = nullptr;
      GDValue v = toGDValue(p);
    #endif

    #if ERRNUM == 2
      void *p = nullptr;
      GDValue v(p);
    #endif
  }
}


void testSymbol()
{
  GDValue dSym1(GDVSymbol("sym1"));
  VPVAL(dSym1);
  xassert(dSym1.asString() == "sym1");
  xassert(dSym1.getKind() == GDVK_SYMBOL);
  xassert(dSym1.isSymbol());
  xassert(dSym1.symbolGet() == GDVSymbol("sym1"));
  testSerializeRoundtrip(dSym1);

  GDValue dSym2(GDVSymbol("sym2"));
  xassert(dSym2.asString() == "sym2");
  xassert(dSym2.getKind() == GDVK_SYMBOL);
  xassert(dSym2.symbolGet() == GDVSymbol("sym2"));
  testSerializeRoundtrip(dSym2);

  xassert(dSym1 < dSym2);
  xassert(GDValue() < dSym1);

  dSym2.reset();
  xassert(dSym2.isNull());
  xassert(dSym2.getKind() == GDVK_SYMBOL);
  testSerializeRoundtrip(dSym2);

  GDValue empty(GDVSymbol(""));
  EXPECT_EQ(empty.asString(), "``");
  EXPECT_EQ(empty.symbolGetName(), "");
  EXPECT_EQ(empty.symbolGet().size(), 0);
  testSerializeRoundtrip(empty);

  std::string hasNulStr("has\0nul", 7);
  GDValue hasNul{GDVSymbol(hasNulStr)};
  EXPECT_EQ(hasNul.asString(), "`has\\u{0}nul`");
  EXPECT_EQ(hasNul.symbolGetName(), hasNulStr);
  EXPECT_EQ(hasNul.symbolGet().size(), 7);
  testSerializeRoundtrip(hasNul);

  GDValue hasBacktick(GDVSymbol("has`back`tick"));
  EXPECT_EQ(hasBacktick.asString(), "`has\\`back\\`tick`");
  EXPECT_EQ(hasBacktick.symbolGetName(), "has`back`tick");
  EXPECT_EQ(hasBacktick.symbolGet().size(), 13);
  testSerializeRoundtrip(hasBacktick);
}


void testInteger()
{
  GDValue d0(0);
  VPVAL(d0);
  xassert(d0.asString() == "0");
  xassert(d0.getSuperKind() == GDVK_INTEGER);
  xassert(d0.getKind() == GDVK_SMALL_INTEGER);
  xassert(d0.isInteger());
  xassert(d0.integerGet() == 0);
  xassert(!d0.isNull());
  xassert(!d0.isBool());
  d0.selfCheck();

  // Ctor from the value kind.
  xassert(d0 == GDValue(GDVK_INTEGER));
  xassert(d0 == GDValue(GDVK_SMALL_INTEGER));

  GDValue d1(1);
  xassert(d1.asString() == "1");
  xassert(d1.getSuperKind() == GDVK_INTEGER);
  xassert(d1.integerGet() == 1);
  d1.selfCheck();

  xassert(d0 < d1);
  xassert(GDValue() < d0);

  testSerializeRoundtrip(d0);
  testSerializeRoundtrip(d1);

  testSerializeRoundtrip(GDValue(1234567890));
  testSerializeRoundtrip(GDValue(-1234567890));

  GDValue big = GDVInteger::fromDigits("0x1234567890ABCDEF1234567890ABCDEF");
  big.selfCheck();
  xassert(big.getSuperKind() == GDVK_INTEGER);
  xassert(big.getKind() == GDVK_INTEGER);

  // Copy ctor where the integer is not small.
  GDValue big2(big);
  xassert(big == big2);

  // Largest small integer.
  GDValue n = GDVInteger::fromDigits("0x7FFFFFFFFFFFFFFF");
  n.selfCheck();
  xassert(n.isSmallInteger());

  // Smallest positive large integer.
  n = n.integerGet() + 1;
  n.selfCheck();
  xassert(!n.isSmallInteger());

  // Most negative small integer.
  n = -n.integerGet();
  n.selfCheck();
  xassert(n.isSmallInteger());

  // Smallest magnitude negative large integer.
  n = n.integerGet() - 1;
  n.selfCheck();
  xassert(!n.isSmallInteger());
}


void testString()
{
  // Get initial counts.
  unsigned initCtSetCopy = GDValue::s_ct_stringSetCopy;
  unsigned initCtSetMove = GDValue::s_ct_stringSetMove;
  unsigned initCtCtorCopy = GDValue::s_ct_stringCtorCopy;
  unsigned initCtCtorMove = GDValue::s_ct_stringCtorMove;

  // Check counts.
  #define CHECK_COUNTS(setCopy, setMove, ctorCopy, ctorMove)            \
    xassert(GDValue::s_ct_stringSetCopy - initCtSetCopy == setCopy);    \
    xassert(GDValue::s_ct_stringSetMove - initCtSetMove == setMove);    \
    xassert(GDValue::s_ct_stringCtorCopy - initCtCtorCopy == ctorCopy); \
    xassert(GDValue::s_ct_stringCtorMove - initCtCtorMove == ctorMove);

  GDValue dStr1(GDVString("str1"));
  CHECK_COUNTS(0, 1, 0, 1)
  VPVAL(dStr1);
  xassert(dStr1.asString() == "\"str1\"");
  xassert(dStr1.getKind() == GDVK_STRING);
  xassert(dStr1.isString());
  xassert(dStr1.stringGet() == GDVString("str1"));
  xassert(!dStr1.isNull());

  GDValue dStr2(GDVString("str2"));
  CHECK_COUNTS(0, 2, 0, 2)
  xassert(dStr2.asString() == "\"str2\"");
  xassert(dStr2.getKind() == GDVK_STRING);
  xassert(dStr2.stringGet() == GDVString("str2"));

  xassert(dStr1 < dStr2);
  xassert(GDValue() < dStr1);

  dStr2.reset();
  xassert(dStr2.isNull());
  xassert(dStr2.getKind() == GDVK_SYMBOL);

  GDVString str1("str1");
  dStr2.stringSet(str1);     // 'set' without 'ctor'
  CHECK_COUNTS(1, 2, 0, 2)

  xassert(dStr1 == dStr2);

  {
    GDValue emptyStr("");
    xassert(emptyStr.stringGet() == GDVString(""));
    xassert(emptyStr < dStr1);

    // Ctor that accepts a kind.
    xassert(emptyStr == GDValue(GDVK_STRING));
  }

  {
    GDValue const &dv = dStr1;
    DIAG("string begin/end const iteration:");
    for (auto it = dv.stringBegin(); it != dv.stringEnd(); ++it) {
      DIAG(*it);
    }

    DIAG("string iterable const iteration:");
    for (auto const &c : dStr1.stringIterable()) {
      DIAG(c);
    }
  }

  {
    GDValue &dv = dStr1;
    DIAG("string begin/end non-const iteration:");
    for (auto it = dv.stringBegin(); it != dv.stringEnd(); ++it) {
      ++(*it);
      DIAG(*it);
    }
    DIAG("again: " << dStr1);
    xassert(dStr1.stringGet() == "tus2");

    DIAG("string iterable non-const iteration:");
    for (auto &c : dStr1.stringIterable()) {
      --c;
      DIAG(c);
    }
    xassert(dStr1.stringGet() == "str1");
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


void testSequence()
{
  GDValue v1(GDVK_SEQUENCE);
  DIAG("empty seq: " << v1);
  xassert(v1.asString() == "[]");
  xassert(v1.containerSize() == 0);
  xassert(v1.containerIsEmpty());
  xassert(v1.getKind() == GDVK_SEQUENCE);
  xassert(v1.isSequence());
  xassert(v1.sequenceGet() == GDVSequence());
  testSerializeRoundtrip(v1);

  GDValue v2((GDVSequence()));

  xassert(v1 == v2);

  GDVSequence seq1b3{GDValue(1), GDValue("b"), GDValue(3)};
  GDValue v3(seq1b3);
  DIAG("three-element seq: " << v3);
  xassert(v3.asString() == "[1 \"b\" 3]");
  xassert(v3.containerSize() == 3);
  xassert(!v3.containerIsEmpty());
  xassert(v3.getKind() == GDVK_SEQUENCE);
  xassert(v3.sequenceGet() == seq1b3);
  xassert(v1 < v3);
  xassert(v3 > v1);
  testSerializeRoundtrip(v3);

  v1.sequenceAppend(GDValue(-1));
  xassert(v1.asString() == "[-1]");
  xassert(v1 < v3);

  v3.sequenceAppend(GDValue("four"));
  xassert(v3.asString() == R"([1 "b" 3 "four"])");

  v1.sequenceResize(3);
  xassert(v1.asString() == "[-1 null null]");

  v3.sequenceResize(3);
  xassert(v3.asString() == R"([1 "b" 3])");

  v1.sequenceSetValueAt(1, v3);
  xassert(v1.asString() == R"([-1 [1 "b" 3] null])");

  v1.sequenceSetValueAt(4, GDValue(5));
  VPVAL(v1);
  xassert(v1.asString() == R"([-1 [1 "b" 3] null null 5])");
  testSerializeRoundtrip(v1);

  xassert(v1.sequenceGetValueAt(1) == v3);

  {
    // Get a `const` reference so I can exercise the
    // `sequenceGetValueAt` that works with `const` objects.
    GDValue const &v1c = v1;

    GDVIndex i = 0;
    for (GDValue const &value : v1.sequenceIterableC()) {
      xassert(value == v1.sequenceGetValueAt(i));
      xassert(value == v1c.sequenceGetValueAt(i));
      ++i;
    }
  }

  for (GDValue &value : v1.sequenceIterable()) {
    if (value.isInteger()) {
      value.integerSet(value.integerGet() + 1);
    }
  }
  xassert(v1.asString() == R"([0 [1 "b" 3] null null 6])");

  // Use `sequenceSetValueAt(const&)` to expand the sequence.
  GDValue seven(7);
  v1.sequenceSetValueAt(5, seven);
  xassert(v1.asString() == R"([0 [1 "b" 3] null null 6 7])");

  v1.sequenceClear();
  xassert(v1 == v2);
  xassert(v1.containerIsEmpty());
  testSerializeRoundtrip(v1);
}


void testTuple()
{
  GDValue v1((GDVTuple()));
  EXPECT_EQ(v1.asString(), "()");
  testSerializeRoundtrip(v1);

  GDVTuple t{1,2,3};
  GDValue v2(t);
  EXPECT_EQ(v2.asString(), "(1 2 3)");
  testSerializeRoundtrip(v2);

  GDValue v3;
  v3.tupleSet(t);
  EXPECT_EQ(v3.asString(), "(1 2 3)");
  testSerializeRoundtrip(v3);

  v3.tupleSet(GDVTuple{4,5,6});
  EXPECT_EQ(v3.asString(), "(4 5 6)");
  testSerializeRoundtrip(v3);

  GDValue const &cv3 = v3;

  int sum = 0;
  for (GDValue const &v : cv3.tupleIterableC()) {
    sum += v.smallIntegerGet();
  }
  EXPECT_EQ(sum, 15);

  sum = 0;
  for (GDValue const &v : cv3.tupleIterable()) {
    sum += v.smallIntegerGet();
  }
  EXPECT_EQ(sum, 15);

  sum = 0;
  for (GDValue &v : v3.tupleIterable()) {
    sum += v.smallIntegerGet();
  }
  EXPECT_EQ(sum, 15);

  v3.tupleAppend(7);
  EXPECT_EQ(v3.asString(), "(4 5 6 7)");
  testSerializeRoundtrip(v3);

  v3.tupleResize(2);
  EXPECT_EQ(v3.asString(), "(4 5)");
  testSerializeRoundtrip(v3);

  v3.tupleSetValueAt(1, 1);
  EXPECT_EQ(v3.asString(), "(4 1)");
  testSerializeRoundtrip(v3);

  v3.tupleSetValueAt(3, v2);
  EXPECT_EQ(v3.asString(), "(4 1 null (1 2 3))");
  testSerializeRoundtrip(v3);

  EXPECT_EQ(cv3.tupleGetValueAt(0).asString(), "4");
  EXPECT_EQ(v3.tupleGetValueAt(2).asString(), "null");

  v3.tupleSetValueAt(4, 4);
  EXPECT_EQ(v3.asString(), "(4 1 null (1 2 3) 4)");
  testSerializeRoundtrip(v3);

  v3.tupleClear();
  EXPECT_EQ(v3.asString(), "()");
  testSerializeRoundtrip(v3);
}


void testSet()
{
  GDValue v1((GDVSet()));
  DIAG("empty set: " << v1);
  xassert(v1.asString() == "{}");
  xassert(v1.containerSize() == 0);
  xassert(v1.containerIsEmpty());
  xassert(v1.getKind() == GDVK_SET);
  xassert(v1.isSet());
  xassert(v1.setGet() == GDVSet());
  testSerializeRoundtrip(v1);

  GDValue v2(v1);
  xassert(v1 == v2);

  v2.setInsert(GDValue(1));
  xassert(v2.setContains(GDValue(1)));
  xassert(v2.containerSize() == 1);
  xassert(v1 < v2);

  v2.setInsert(GDValue(2));
  xassert(v2.asString() == "{1 2}");
  testSerializeRoundtrip(v2);

  v2.setRemove(GDValue(1));
  xassert(v2.asString() == "{2}");

  v2.setClear();
  xassert(v2.asString() == "{}");
  xassert(v1 == v2);
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
  DIAG(v2);
  xassert(v2.asString() == R"({10 "x" [2 3 4]})");
  testSerializeRoundtrip(v2);

  {
    GDVSet tmpSet{3,2,1};

    // Construct a GDValue from a `const&` set.
    v2 = tmpSet;
    EXPECT_EQ(v2.asString(), "{1 2 3}");

    v2.setGetMutable().erase(2);
    EXPECT_EQ(v2.asString(), "{1 3}");

    // Exercise `setCBegin` and `setCEnd`.
    std::vector<GDValue> expectVec{1,3};
    auto expectIt = expectVec.begin();
    for (auto it = v2.setCBegin(); it != v2.setCEnd(); ++it) {
      EXPECT_EQ(*it, *expectIt);
      ++expectIt;
    }
    xassert(expectIt == expectVec.end());

    // Exercise `setBegin` and `setEnd`.
    expectIt = expectVec.begin();
    for (auto it = v2.setBegin(); it != v2.setEnd(); ++it) {
      EXPECT_EQ(*it, *expectIt);
      ++expectIt;
    }
    xassert(expectIt == expectVec.end());

    // Exercise `setInsert(const&)`.
    GDValue four(4);
    v2.setInsert(four);
    EXPECT_EQ(v2.asString(), "{1 3 4}");
  }
}


void testMap()
{
  GDValue v1((GDVMap()));
  DIAG("empty map: " << v1);
  xassert(v1.asString() == "{:}");
  xassert(v1.containerSize() == 0);
  xassert(v1.containerIsEmpty());
  xassert(v1.getKind() == GDVK_MAP);
  xassert(v1.isMap());
  xassert(v1.mapGet() == GDVMap());
  testSerializeRoundtrip(v1);

  GDValue v2(v1);
  xassert(v1 == v2);

  v2.mapSetValueAt(GDValue("one"), GDValue(1));
  xassert(v2.containerSize() == 1);
  xassert(v2.mapGetValueAt(GDValue("one")) == GDValue(1));
  DIAG(v2);
  xassert(v2.asString() == R"({"one":1})");
  xassert(v2.mapContains(GDValue("one")));
  xassert(v2 > v1);
  testSerializeRoundtrip(v2);

  v2.mapSetValueAt(GDValue("one"), GDValue(2));
  xassert(v2.asString() == R"({"one":2})");
  xassert(v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.mapSetValueAt(GDValue("two"), GDValue(2));
  xassert(v2.asString() == R"({"one":2 "two":2})");
  xassert(v2.containerSize() == 2);
  testSerializeRoundtrip(v2);

  v2.mapRemoveKey(GDValue("one"));
  xassert(v2.asString() == R"({"two":2})");
  xassert(!v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.mapClear();
  xassert(v1 == v2);
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
  DIAG(v2);
  xassert(v2.asString() == "{2:3 \"a\":1 [10 11]:ten_eleven}");
  testSerializeRoundtrip(v2);

  xassert(v2.mapGetValueAt(
            GDValue(GDVSequence({GDValue(10), GDValue(11)}))) ==
          GDValue(GDVSymbol("ten_eleven")));

  // Call the ctor that accepts a `map const &`.
  GDVMap tmpMap{ {1,2}, {3,4} };
  v2 = tmpMap;
  EXPECT_EQ(v2.asString(), "{1:2 3:4}");
  testSerializeRoundtrip(v2);

  // Call 'mapIterableC'.
  auto expectIt = tmpMap.begin();
  for (auto const &kv : v2.mapIterableC()) {
    EXPECT_EQ(kv.first, (*expectIt).first);
    EXPECT_EQ(kv.second, (*expectIt).second);
    ++expectIt;
  }
  xassert(expectIt == tmpMap.end());

  // Call 'mapIterable'.
  expectIt = tmpMap.begin();
  for (auto &kv : v2.mapIterable()) {
    EXPECT_EQ(kv.first, (*expectIt).first);
    EXPECT_EQ(kv.second, (*expectIt).second);
    ++expectIt;
  }
  xassert(expectIt == tmpMap.end());

  // Call `mapGetMutable`.
  v2.mapGetMutable().insert({5,6});
  EXPECT_EQ(v2.asString(), "{1:2 3:4 5:6}");

  testSerializeRoundtrip(v2);
  // Call `mapGetValueAt() const`.
  GDValue const &v2c = v2;
  EXPECT_EQ(v2c.mapGetValueAt(3), GDValue(4));

  // Call `mapSetValueAt(..., const&)`.
  GDValue eight(8);
  v2.mapSetValueAt(7, eight);
  EXPECT_EQ(v2.asString(), "{1:2 3:4 5:6 7:8}");
  testSerializeRoundtrip(v2);

  // Again, but this time replacing an existing value.
  v2.mapSetValueAt(5, eight);
  EXPECT_EQ(v2.asString(), "{1:2 3:4 5:8 7:8}");
  testSerializeRoundtrip(v2);

  // Various containers as keys.
  v2 = GDVMap{
    { GDVSequence{1,2,3}, 4 },
    { GDVTuple{1,2,3},    4 },
    { GDVSet{1,2,3},      4 },
    { GDVMap{{1,2}},      4 },
  };
  EXPECT_EQ(v2.asString(), "{[1 2 3]:4 (1 2 3):4 {1 2 3}:4 {1:2}:4}");
  testSerializeRoundtrip(v2);

  // Various containers as first keys.
  v2 = GDVMap{
    { GDVTuple{1,2,3},    4 },
  };
  EXPECT_EQ(v2.asString(), "{(1 2 3):4}");
  testSerializeRoundtrip(v2);

  v2 = GDVMap{
    { GDVSet{1,2,3},      4 },
  };
  EXPECT_EQ(v2.asString(), "{{1 2 3}:4}");
  testSerializeRoundtrip(v2);

  v2 = GDVMap{
    { GDVMap{{1,2}},      4 },
  };
  EXPECT_EQ(v2.asString(), "{{1:2}:4}");
  testSerializeRoundtrip(v2);

  v2 = GDVMap{
    { GDVTaggedSequence(GDVSymbol("A"), {1}), GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "{A[1]:one}");
  testSerializeRoundtrip(v2);

  v2 = GDVMap{
    { GDVTaggedTuple(GDVSymbol("A"), {1}), GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "{A(1):one}");
  testSerializeRoundtrip(v2);

  v2 = GDVMap{
    { GDVTuple{1}, GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "{(1):one}");
  testSerializeRoundtrip(v2);
}


void testTaggedMap()
{
  GDValue v(GDVK_TAGGED_MAP);
  EXPECT_EQ(v.asString(), "null{:}");
  xassert(v.isMap());
  xassert(v.isTaggedContainer());
  xassert(v.isTaggedMap());
  xassert(v.containerIsEmpty());
  xassert(v.taggedContainerGetTag() == GDVSymbol());
  xassert(v.taggedContainerGetTag() == GDVSymbol("null"));
  v.selfCheck();
  testSerializeRoundtrip(v);

  v = GDVTaggedMap(GDVSymbol("x"), {{1,2}});
  EXPECT_EQ(v.asString(), "x{1:2}");
  xassert(v.isMap());
  xassert(v.isTaggedContainer());
  xassert(v.isTaggedMap());
  xassert(!v.containerIsEmpty());
  v.selfCheck();
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("y"));
  EXPECT_EQ(v.asString(), "y{1:2}");
  testSerializeRoundtrip(v);

  GDVTaggedMap tm(GDVSymbol("z"), {{3,4}, {5,6}});
  v = tm;
  EXPECT_EQ(v.asString(), "z{3:4 5:6}");
  testSerializeRoundtrip(v);

  {
    GDValue v2(tm);
    xassert(v == v2);

    v2.mapClear();
    EXPECT_EQ(v2.asString(), "z{:}");
    testSerializeRoundtrip(v2);
  }

  xassert(v.taggedMapGet().m_container.size() == 2);

  xassert(v.mapContains(3));
  xassert(!v.mapContains(4));

  v.mapSetValueAt(5,7);
  EXPECT_EQ(v.asString(), "z{3:4 5:7}");
  testSerializeRoundtrip(v);

  v.mapGetMutable().insert({8,9});
  EXPECT_EQ(v.asString(), "z{3:4 5:7 8:9}");
  testSerializeRoundtrip(v);

  v.taggedMapGetMutable().m_container.erase(5);
  EXPECT_EQ(v.asString(), "z{3:4 8:9}");
  testSerializeRoundtrip(v);

  // Use the constructor for GDVTaggedMap that takes a `const&`.
  GDVMap m{{"a","b"}};
  GDVTaggedMap tm2(GDVSymbol("_"), m);
  v = tm2;
  EXPECT_EQ(v.asString(), "_{\"a\":\"b\"}");

  // Exercise `GDVTaggedMap::operator=(const&)`.
  v.taggedMapGetMutable() = tm;
  EXPECT_EQ(v.asString(), "z{3:4 5:6}");

  {
    // GDValue copy ctor with a tagged map.
    GDValue v2(v);
    EXPECT_EQ(v2.asString(), "z{3:4 5:6}");
  }

  // Exercise `GDVTaggedMap::operator=(&&)`.
  v.taggedMapGetMutable() = GDVTaggedMap(GDVSymbol("a"), {});
  EXPECT_EQ(v.asString(), "a{:}");

  // Exercise `GDVTaggedMap::swap`.
  v.taggedMapGetMutable().swap(tm2);
  EXPECT_EQ(v.asString(), "_{\"a\":\"b\"}");
  v.taggedMapGetMutable().swap(tm2);
  EXPECT_EQ(v.asString(), "a{:}");

  // `GDValue::mapSet(&&)` when the value already has a map.
  v.mapSet(GDVMap{{-1,-2}});
  EXPECT_EQ(v.asString(), "a{-1:-2}");

  // `GDValue::mapSet(const&)` when the value already has a map.
  v.mapSet(m);
  EXPECT_EQ(v.asString(), "a{\"a\":\"b\"}");

  // `GDValue::taggedMapSet(&&)` when the value is already a tagged map.
  v.taggedMapSet(GDVTaggedMap(GDVSymbol("j"), {}));
  EXPECT_EQ(v.asString(), "j{:}");

  // `GDValue::taggedMapSet(const&)` when the value is already a tagged
  // map.
  v.taggedMapSet(tm2);
  EXPECT_EQ(v.asString(), "_{\"a\":\"b\"}");
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol(""));
  EXPECT_EQ(v.asString(), "``{\"a\":\"b\"}");
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("some{crazy}char[act]ers"));
  EXPECT_EQ(v.asString(), "`some{crazy}char[act]ers`{\"a\":\"b\"}");
  testSerializeRoundtrip(v);
}


void testOrderedMap()
{
  GDValue v1((GDVOrderedMap()));
  DIAG("empty ordered map: " << v1);
  xassert(v1.asString() == "[:]");
  xassert(v1.containerSize() == 0);
  xassert(v1.containerIsEmpty());
  xassert(v1.getKind() == GDVK_ORDERED_MAP);
  xassert(!v1.isMap());
  xassert(v1.isOrderedMap());
  xassert(v1.orderedMapGet() == GDVOrderedMap());
  testSerializeRoundtrip(v1);

  GDValue v2(v1);
  xassert(v1 == v2);

  v2.orderedMapSetValueAt(GDValue("one"), GDValue(1));
  v2.selfCheck();
  xassert(v2.containerSize() == 1);
  xassert(v2.orderedMapGetValueAt(GDValue("one")) == GDValue(1));
  DIAG(v2);
  xassert(v2.asString() == R"(["one":1])");
  xassert(v2.orderedMapContains(GDValue("one")));
  xassert(v2 > v1);
  testSerializeRoundtrip(v2);

  // The normal "map" functions work too.
  v2.mapSetValueAt(GDValue("one"), GDValue(2));
  EXPECT_EQ(v2.asString(), R"(["one":2])");
  xassert(v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.mapSetValueAt(GDValue("two"), GDValue(2));
  xassert(v2.asString() == R"(["one":2 "two":2])");
  xassert(v2.containerSize() == 2);
  testSerializeRoundtrip(v2);

  v2.mapRemoveKey(GDValue("one"));
  xassert(v2.asString() == R"(["two":2])");
  xassert(!v2.mapContains(GDValue("one")));
  testSerializeRoundtrip(v2);

  v2.orderedMapClear();
  xassert(v1 == v2);
  testSerializeRoundtrip(v2);

  // Unlike with a normal map, the ordered map remembers the order of
  // the entries.
  v2 = GDValue(GDVOrderedMap{
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
  DIAG(v2);
  xassert(v2.asString() == "[\"a\":1 2:3 [10 11]:ten_eleven]");
  testSerializeRoundtrip(v2);

  xassert(v2.mapGetValueAt(
            GDValue(GDVSequence({GDValue(10), GDValue(11)}))) ==
          GDValue(GDVSymbol("ten_eleven")));

  // Call the ctor that accepts a `OrderedMap const &`.
  GDVOrderedMap tmpMap{ {1,2}, {3,4} };
  v2 = tmpMap;
  EXPECT_EQ(v2.asString(), "[1:2 3:4]");
  testSerializeRoundtrip(v2);

  // Call 'orderedMapIterableC'.
  auto expectIt = tmpMap.begin();
  for (auto const &kv : v2.orderedMapIterableC()) {
    EXPECT_EQ(kv.first, (*expectIt).first);
    EXPECT_EQ(kv.second, (*expectIt).second);
    ++expectIt;
  }
  xassert(expectIt == tmpMap.end());

  // Call 'orderedMapIterable'.
  expectIt = tmpMap.begin();
  for (auto &kv : v2.orderedMapIterable()) {
    EXPECT_EQ(kv.first, (*expectIt).first);
    EXPECT_EQ(kv.second, (*expectIt).second);
    ++expectIt;
  }
  xassert(expectIt == tmpMap.end());

  // Call `orderedMapGetMutable`.
  v2.orderedMapGetMutable().insert({-5,6});
  EXPECT_EQ(v2.asString(), "[1:2 3:4 -5:6]");

  testSerializeRoundtrip(v2);
  // Call `orderedMapGetValueAt() const`.
  GDValue const &v2c = v2;
  EXPECT_EQ(v2c.orderedMapGetValueAt(3), GDValue(4));
  EXPECT_EQ(v2c.mapGetValueAt(3), GDValue(4));

  // Call `orderedMapSetValueAt(..., const&)` (indirectly).
  GDValue eight(8);
  v2.mapSetValueAt(7, eight);
  EXPECT_EQ(v2.asString(), "[1:2 3:4 -5:6 7:8]");
  testSerializeRoundtrip(v2);

  // Again, but this time replacing an existing value.
  v2.mapSetValueAt(-5, eight);
  EXPECT_EQ(v2.asString(), "[1:2 3:4 -5:8 7:8]");
  testSerializeRoundtrip(v2);

  // Various containers as keys.
  v2 = GDVOrderedMap{
    { GDVTuple{1,2,3},    4 },     // Regular map would rearrange this.
    { GDVSequence{1,2,3}, 4 },
    { GDVSet{1,2,3},      4 },
    { GDVMap{{1,2}},      4 },
  };
  EXPECT_EQ(v2.asString(), "[(1 2 3):4 [1 2 3]:4 {1 2 3}:4 {1:2}:4]");
  testSerializeRoundtrip(v2);

  // Various containers as first keys.
  v2 = GDVOrderedMap{
    { GDVTuple{1,2,3},    4 },
  };
  EXPECT_EQ(v2.asString(), "[(1 2 3):4]");
  testSerializeRoundtrip(v2);

  v2 = GDVOrderedMap{
    { GDVSet{1,2,3},      4 },
  };
  EXPECT_EQ(v2.asString(), "[{1 2 3}:4]");
  testSerializeRoundtrip(v2);

  v2 = GDVOrderedMap{
    { GDVMap{{1,2}},      4 },
  };
  EXPECT_EQ(v2.asString(), "[{1:2}:4]");
  testSerializeRoundtrip(v2);

  v2 = GDVOrderedMap{
    { GDVTaggedSequence(GDVSymbol("A"), {1}), GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "[A[1]:one]");
  testSerializeRoundtrip(v2);

  v2 = GDVOrderedMap{
    { GDVTaggedTuple(GDVSymbol("A"), {1}), GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "[A(1):one]");
  testSerializeRoundtrip(v2);

  v2 = GDVOrderedMap{
    { GDVTuple{1}, GDVSymbol("one") }
  };
  EXPECT_EQ(v2.asString(), "[(1):one]");
  testSerializeRoundtrip(v2);
}


void testTaggedOrderedMap()
{
  GDValue v(GDVK_TAGGED_ORDERED_MAP);
  EXPECT_EQ(v.asString(), "null[:]");
  xassert(!v.isMap());
  xassert(v.isTaggedContainer());
  xassert(!v.isTaggedMap());
  xassert(v.isTaggedOrderedMap());
  xassert(v.containerIsEmpty());
  xassert(v.taggedContainerGetTag() == GDVSymbol());
  xassert(v.taggedContainerGetTag() == GDVSymbol("null"));
  v.selfCheck();
  testSerializeRoundtrip(v);

  v = GDVTaggedOrderedMap(GDVSymbol("x"), {{1,2}});
  EXPECT_EQ(v.asString(), "x[1:2]");
  xassert(!v.isMap());
  xassert(v.isTaggedContainer());
  xassert(!v.isTaggedMap());
  xassert(v.isTaggedOrderedMap());
  xassert(!v.containerIsEmpty());
  v.selfCheck();
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("y"));
  EXPECT_EQ(v.asString(), "y[1:2]");
  testSerializeRoundtrip(v);

  GDVTaggedOrderedMap tm(GDVSymbol("z"), {{3,4}, {-5,6}});
  v = tm;
  EXPECT_EQ(v.asString(), "z[3:4 -5:6]");
  testSerializeRoundtrip(v);

  {
    GDValue v2(tm);
    xassert(v == v2);

    v2.mapClear();
    EXPECT_EQ(v2.asString(), "z[:]");
    testSerializeRoundtrip(v2);
  }

  xassert(v.taggedOrderedMapGet().m_container.size() == 2);

  xassert(v.mapContains(3));
  xassert(!v.mapContains(4));

  v.mapSetValueAt(-5,7);
  EXPECT_EQ(v.asString(), "z[3:4 -5:7]");
  testSerializeRoundtrip(v);

  v.orderedMapGetMutable().insert({8,9});
  EXPECT_EQ(v.asString(), "z[3:4 -5:7 8:9]");
  testSerializeRoundtrip(v);

  xassert(v.taggedOrderedMapGetMutable().m_container.eraseKey(-5));
  EXPECT_EQ(v.asString(), "z[3:4 8:9]");
  testSerializeRoundtrip(v);

  // Use the constructor for GDVTaggedOrderedMap that takes a `const&`.
  GDVOrderedMap m{{"a","b"}};
  GDVTaggedOrderedMap tm2(GDVSymbol("_"), m);
  v = tm2;
  EXPECT_EQ(v.asString(), "_[\"a\":\"b\"]");

  // Exercise `GDVTaggedOrderedMap::operator=(const&)`.
  v.taggedOrderedMapGetMutable() = tm;
  EXPECT_EQ(v.asString(), "z[3:4 -5:6]");

  {
    // GDValue copy ctor with a tagged ordered map.
    GDValue v2(v);
    EXPECT_EQ(v2.asString(), "z[3:4 -5:6]");
  }

  // Exercise `GDVTaggedOrderedMap::operator=(&&)`.
  v.taggedOrderedMapGetMutable() = GDVTaggedOrderedMap(GDVSymbol("a"), {});
  EXPECT_EQ(v.asString(), "a[:]");

  // Exercise `GDVTaggedMap::swap`.
  v.taggedOrderedMapGetMutable().swap(tm2);
  EXPECT_EQ(v.asString(), "_[\"a\":\"b\"]");
  v.taggedOrderedMapGetMutable().swap(tm2);
  EXPECT_EQ(v.asString(), "a[:]");

  // `GDValue::orderedMapSet(&&)` when the value already has a map.
  v.orderedMapSet(GDVOrderedMap{{-1,-2}});
  EXPECT_EQ(v.asString(), "a[-1:-2]");

  // `GDValue::orderedMapSet(const&)` when the value already has a map.
  v.orderedMapSet(m);
  EXPECT_EQ(v.asString(), "a[\"a\":\"b\"]");

  // `GDValue::taggedOrderedMapSet(&&)` when the value is already a
  // tagged map.
  v.taggedOrderedMapSet(GDVTaggedOrderedMap(GDVSymbol("j"), {}));
  EXPECT_EQ(v.asString(), "j[:]");

  // `GDValue::taggedOrderedMapSet(const&)` when the value is already a
  // tagged map.
  v.taggedOrderedMapSet(tm2);
  EXPECT_EQ(v.asString(), "_[\"a\":\"b\"]");
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol(""));
  EXPECT_EQ(v.asString(), "``[\"a\":\"b\"]");
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("some{crazy}char[act]ers"));
  EXPECT_EQ(v.asString(), "`some{crazy}char[act]ers`[\"a\":\"b\"]");
  testSerializeRoundtrip(v);
}


void testTaggedSequence()
{
  GDValue v(GDVK_TAGGED_SEQUENCE);
  EXPECT_EQ(v.asString(), "null[]");
  xassert(v.isSequence());
  xassert(v.isTaggedContainer());
  xassert(v.isTaggedSequence());
  xassert(v.containerIsEmpty());
  xassert(v.taggedContainerGetTag() == GDVSymbol());
  xassert(v.taggedContainerGetTag() == GDVSymbol("null"));
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("x"));
  EXPECT_EQ(v.asString(), "x[]");
  testSerializeRoundtrip(v);

  v.sequenceAppend(1);
  EXPECT_EQ(v.asString(), "x[1]");
  testSerializeRoundtrip(v);
}


void writeManyWidths(GDValue const &v)
{
  // Try writing the value with every column width in [1,80].  This is
  // particularly intended to stress the logic that prints maps by
  // checking which of key, key+value, etc., will fit on a line.
  GDValueWriteOptions options;
  for (int w=1; w <= 80; ++w) {
    options.m_targetLineWidth = w;

    // Just throw away the result.  The point is to check that no
    // assertion fails.
    v.writeLines(nullOStream, options);
  }
}


// Write `v` with many widths where it is both the key and value of a
// map entry.
void writeAsMapElementManyWidths(GDValue const &v)
{
  GDValue map = GDVMap{{v,v}};
  writeManyWidths(map);
}


void testTaggedTuple()
{
  GDValue v(GDVK_TAGGED_TUPLE);
  EXPECT_EQ(v.asString(), "null()");
  xassert(v.isTuple());
  xassert(v.isTaggedContainer());
  xassert(v.isTaggedTuple());
  xassert(v.containerIsEmpty());
  xassert(v.taggedContainerGetTag() == GDVSymbol());
  xassert(v.taggedContainerGetTag() == GDVSymbol("null"));
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("x"));
  EXPECT_EQ(v.asString(), "x()");
  testSerializeRoundtrip(v);

  v.tupleAppend(1);
  EXPECT_EQ(v.asString(), "x(1)");
  testSerializeRoundtrip(v);
  writeAsMapElementManyWidths(v);

  v.tupleAppend(2);
  EXPECT_EQ(v.asString(), "x(1 2)");
  testSerializeRoundtrip(v);
  writeAsMapElementManyWidths(v);
}


void testTaggedSet()
{
  GDValue v(GDVK_TAGGED_SET);
  EXPECT_EQ(v.asString(), "null{}");
  xassert(v.isSet());
  xassert(v.isTaggedContainer());
  xassert(v.isTaggedSet());
  xassert(v.containerIsEmpty());
  xassert(v.taggedContainerGetTag() == GDVSymbol());
  xassert(v.taggedContainerGetTag() == GDVSymbol("null"));
  testSerializeRoundtrip(v);

  v.taggedContainerSetTag(GDVSymbol("x"));
  EXPECT_EQ(v.asString(), "x{}");
  testSerializeRoundtrip(v);

  v.setInsert(1);
  EXPECT_EQ(v.asString(), "x{1}");
  testSerializeRoundtrip(v);
}


// Print a little ruler to help judge the behavior.
void printRuler(int width)
{
  if (!verbose) {
    return;
  }

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


void toutLinesPlus(
  GDValue const &v,
  GDValueWriteOptions const &options)
{
  v.writeLines(tout, options);

  writeManyWidths(v);
}


// This is an ad-hoc collection of things to print, meant for
// interactive experimentation and verification.
void testPrettyPrint(int width)
{
  DIAG("pretty print target width: " << width);
  printRuler(width);

  GDValueWriteOptions options;
  options.m_targetLineWidth = width;

  GDValue v(GDVSequence{1,2,3});
  toutLinesPlus(v, options);

  GDValue m2(GDVMap{
               {GDVSymbol("a"), v},
               {v, v},
             });
  m2.writeLines(tout, options);

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
  toutLinesPlus(v, options);

  GDValue m(GDVMap{
              { 8, 9},
              {10,11},
              {12,13},
              {14,15},
            });
  m.writeLines(tout, options);

  GDValue s(GDVSet{"eins", "zwei", "drei"});

  v = GDValue(GDVMap{
        {GDVSymbol("v"),        v},
        {"four",                4},
        {"x",                   m},
        {m,                     m},
        {GDVSymbol("counting"), s},
      });
  toutLinesPlus(v, options);

  v = GDValue(GDVMap{ {1,2} });
  v = GDValue(GDVMap{ {v,s} });
  toutLinesPlus(v, options);

  // Exercise printing where a map value is long but is not a container.
  // This is meant to barely not fit in 20 columns, thereby causing the
  // key/value pair to be split onto two lines (even though that does
  // not help here).
  v = GDValue(GDVMap{ {1, "long-ish value"},
                      {2, 3} });
  toutLinesPlus(v, options);

  v = GDVSequence{
        GDVTaggedMap(GDVSymbol("tagName__"),
                     {{1,2}, {3,4}}),
        5
      };
  toutLinesPlus(v, options);

  v = GDVTaggedMap(
        GDVSymbol("outer"), {
          {
            "x",
            GDVTaggedMap(
              GDVSymbol("inner12345678"), {
                {1,2},
                {3,4}
              })
          }
        });
  toutLinesPlus(v, options);

  printRuler(width);
}


std::string linesStringFor(GDValue const &value, int targetWidth)
{
  return value.asLinesString(GDValueWriteOptions()
                               .setTargetLineWidth(targetWidth));
}


void checkLinesStringFor(
  GDValue const &value,
  int targetWidth,
  std::string expect)
{
  EXN_CONTEXT_EXPR(targetWidth);

  std::string actual = linesStringFor(value, targetWidth);
  if (actual != expect) {
    DIAG("expect:\n" << expect);       // gcov-ignore
    DIAG("actual:\n" << actual);       // gcov-ignore
  }
  EXPECT_EQ(actual, expect);
}


// Test printing specific structures against expectation.
//
// This test mechanism is very crude.  I should be able to replace it
// with something better once I have text deserialization.
void testPrettyExpect()
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

  checkLinesStringFor(m, 10, R"({
  {1:2}:
    {
      "drei"
      "ein"
      "zwei"
    }
}
)");

  checkLinesStringFor(m, 11, R"({
  {1:2}: {
    "drei"
    "ein"
    "zwei"
  }
}
)");

  checkLinesStringFor(m, 12, R"({
  {1:2}: {
    "drei"
    "ein"
    "zwei"
  }
}
)");

  checkLinesStringFor(m, 24, R"({
  {1:2}: {
    "drei"
    "ein"
    "zwei"
  }
}
)");

  checkLinesStringFor(m, 25, R"({
  {1:2}:
    {"drei" "ein" "zwei"}
}
)");
}


void testOneErrorSubstrOrRegex(
  char const *input,
  int expectLine,
  int expectColumn,
  char const * NULLABLE expectErrorSubstring,
  char const * NULLABLE expectErrorRegex)
{
  EXN_CONTEXT("input=" << doubleQuote(input));

  try {
    try {
      GDValue::readFromString(input);
      xfailure("should have failed");
    }
    catch (ReaderException &e) {
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
  catch (XBase &x) {
    x.prependContext(stringb("input=" << doubleQuote(input)));
    throw;
  }
}


void testOneErrorSubstr(
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


void testOneErrorRegex(
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


/* Invoke `testOneErrorRegex` on `input` followed by each character in
   `nextChar`.  A character can be -1 to mean EOF.
*/
void testMultiErrorRegex(
  char const *inputPrefix,
  std::vector<int> nextChar,
  int expectLine,
  int expectColumn,
  char const *expectErrorRegexSuffix)
{
  for (int c : nextChar) {
    try {
      std::string input(inputPrefix);
      if (c >= 0) {
        input += (char)c;
      }

      std::string expectRegex;
      if (c < 0) {
        expectRegex = "end of file";
      }
      else {
        expectRegex = escapeForRegex(stringb("'" << (char)c << "'"));
      }
      expectRegex += ".*";
      expectRegex += expectErrorRegexSuffix;

      testOneErrorRegex(
        input.c_str(),
        expectLine,
        expectColumn,
        expectRegex.c_str());
    }
    catch (XBase &x) {
      x.prependContext(stringb("c=" << c));
      throw;
    }
  }
}


void testSyntaxErrors()
{
  // This test is meant to correspond to gdvalue-reader.cc, exercising
  // each of the error paths evident in each function.  Basically, I
  // search for "err(" (case-insensitively) and then target each
  // occurrence.


  // skipWhitespaceAndComments
  {
    // unexpectedCharErr(c, "looking for character after '/'");
    testOneErrorRegex(" /", 1, 3, "end of file.*after '/'");
    testOneErrorRegex("/-", 1, 2, "'-'.*after '/'");

    // Comment-related: check that they do not mess up locations.
    testOneErrorSubstr("//x\n3.4", 2, 2, "Unexpected '.'");
  }

  // skipCStyleComment
  {
    // First `readCommentCharNoEOFOrErr()`, just inside the loop.
    testOneErrorSubstr("/*/1", 1, 5,
      R"(inside "/*" comment, looking for corresponding "*/")");
    testOneErrorSubstr("/*/**/", 1, 7,
      R"(inside "/*" comment, which contains 1 child comments, looking for corresponding "*/")");
    testOneErrorSubstr("/*/**//**/", 1, 11,
      R"(inside "/*" comment, which contains 2 child comments, looking for corresponding "*/")");
    testOneErrorSubstr("/*/*/**//**/", 1, 13,
      R"(inside "/*" comment, nested inside 1 other comments of the same kind, which contains 2 child comments, looking for corresponding "*/")");

    // Second `readCommentCharNoEOFOrErr()`, after seeing '/'.
    testOneErrorSubstr("/*/", 1, 4,
      R"(inside "/*" comment, looking for corresponding "*/")");

    // Third `readCommentCharNoEOFOrErr()`, after seeing '*'.
    testOneErrorSubstr("/*/*", 1, 5,
      R"(inside "/*" comment, nested inside 1 other comments of the same kind, looking for corresponding "*/")");
  }

  // readSequenceAfterFirstValue
  {
    // readCharOrErr(']', "looking for ']' at end of sequence");
    testMultiErrorRegex("[1 ", {-1, '}'}, 1, 4, "looking for '\\]' at end of sequence");
    testMultiErrorRegex("[1 2", {-1, '}'}, 1, 5, "looking for '\\]' at end of sequence");
  }

  // readNextTuple
  {
    // readCharOrErr(')', "looking for ')' at end of tuple");
    testMultiErrorRegex("(", {-1, '}'}, 1, 2, "looking for '\\)' at end of tuple");
    testMultiErrorRegex("(1 ", {-1, '}'}, 1, 4, "looking for '\\)' at end of tuple");
  }

  // readNextPossibleMap
  {
    // processCharOrErr(skipWhitespaceAndComments(), closingDelim,
    //   ordered?
    //     "looking for ']' after ':' of empty ordered map" :
    //     "looking for '}' after ':' of empty map");
    testMultiErrorRegex("{:", {-1, ']'}, 1, 3, "looking for '\\}' after ':'");
    testMultiErrorRegex("{ :", {-1, ']'}, 1, 4, "looking for '\\}' after ':'");
    testMultiErrorRegex("{ : ", {-1, ']'}, 1, 5, "looking for '\\}' after ':'");
    testMultiErrorRegex("[:", {-1, '}'}, 1, 3, "looking for '\\]' after ':'");
    testMultiErrorRegex("[ :", {-1, '}'}, 1, 4, "looking for '\\]' after ':'");
    testMultiErrorRegex("[ : ", {-1, '}'}, 1, 5, "looking for '\\]' after ':'");

    // unexpectedCharErr(readChar(), ordered?
    //   "looking for a value after '['" :
    //   "looking for a value after '{'");
    testMultiErrorRegex("{", {-1, ']'}, 1, 2, "looking for a value after '\\{'");
    testMultiErrorRegex("{ ", {-1, ']'}, 1, 3, "looking for a value after '\\{'");
    testMultiErrorRegex("[", {-1, '}'}, 1, 2, "looking for a value after '\\['");
    testMultiErrorRegex("[ ", {-1, ')'}, 1, 3, "looking for a value after '\\['");

    testOneErrorRegex("{3-", 1, 3, "'-' after a value");
  }

  // readSetAfterFirstValue
  {
    // readCharOrErr('}', "looking for '}' at end of set");
    testMultiErrorRegex("{1", {-1, ']'}, 1, 3, "looking for '\\}' at end of set");
    testMultiErrorRegex("{1 2", {-1, ']'}, 1, 5, "looking for '\\}' at end of set");
  }

  // readPossiblyOrderedMapAfterFirstKey
  {
    // unexpectedCharErr(readChar(), ordered?
    //   "looking for value after ':' in ordered map entry" :
    //   "looking for value after ':' in map entry");
    testMultiErrorRegex("{1:", {-1, ']'}, 1, 4, "looking for value after ':' in map");
    testMultiErrorRegex("{1: ", {-1, ']'}, 1, 5, "looking for value after ':' in map");
    testMultiErrorRegex("[1:", {-1, '}'}, 1, 4, "looking for value after ':' in ordered map");
    testMultiErrorRegex("[1: ", {-1, ')'}, 1, 5, "looking for value after ':' in ordered map");

    // readCharOrErr(closingDelim, ordered?
    //   "looking for ']' at end of ordered map" :
    //   "looking for '}' at end of map");
    testOneErrorRegex("{1:2]", 1, 5, "'\\]'.*looking for '\\}' at end of map");
    testMultiErrorRegex("{1:2 ", {-1, ']'}, 1, 6, "looking for '\\}' at end of map");
    testMultiErrorRegex("{1:2", {-1, ']'}, 1, 5, "looking for '\\}' at end of map");
    testOneErrorRegex("[1:2}", 1, 5, "'\\}'.*looking for '\\]' at end of ordered map");
    testMultiErrorRegex("[1:2 ", {-1, '}'}, 1, 6, "looking for '\\]' at end of ordered map");
    testMultiErrorRegex("[1:2", {-1, '}'}, 1, 5, "looking for '\\]' at end of ordered map");

    // processCharOrErr(colon, ':', ordered?
    //   "looking for ':' in ordered map entry" :
    //   "looking for ':' in map entry");
    testOneErrorRegex("{1:2 3", 1, 7, "end of file.*looking for ':'");
    testOneErrorRegex("{1:2 3}", 1, 7, "'\\}'.*looking for ':'");
    testOneErrorRegex("{1:2 3]", 1, 7, "'\\]'.*looking for ':'");
    testOneErrorRegex("{1:2 3-", 1, 7, "'-' after a value");
    testOneErrorRegex("{1:2 3 4", 1, 8, "'4'.*looking for ':' in map");
    testOneErrorRegex("[1:2 3 4", 1, 8, "'4'.*looking for ':' in ordered map");

    // unexpectedCharErr(readChar(), ordered?
    //   "looking for value after ':' in ordered map entry" :
    //   "looking for value after ':' in map entry");
    testOneErrorRegex("{1:2 3:", 1, 8, "end of file.*after ':'");
    testOneErrorRegex("{1:2 3 : ", 1, 10, "end of file.*after ':'");
    testOneErrorRegex("{1:2 3:]", 1, 8, "'\\]'.*after ':'");
    testOneErrorRegex("{1:2 3:}", 1, 8, "'\\}'.*after ':'");
    testOneErrorRegex("{1:2 3::", 1, 8, "':'.*start of a value");
    testOneErrorRegex("{1:2 3: }", 1, 9, "'\\}'.*after ':' in map");
    testOneErrorRegex("[1:2 3: ]", 1, 9, "'\\]'.*after ':' in ordered");

    // locErr(loc, stringb("Duplicate " << (ordered? "ordered " : "") <<
    //                     "map key: " << keyAsString));
    testOneErrorSubstr("{1:2 1:2}", 1, 6, "Duplicate map key: 1");
    testOneErrorSubstr("{1:2 3:4 1:2}", 1, 10, "Duplicate map key: 1");
    testOneErrorSubstr("{1:2 {4:4}:4 11:2 {4:4}:5}", 1, 19, "Duplicate map key: {4:4}");
    testOneErrorSubstr("[1:2 {4:4}:4 11:2 {4:4}:5]", 1, 19, "Duplicate ordered map key: {4:4}");
  }

  // readNextQuotedStringContents
  {
    // int c = readNotEOFCharOrErr(
    //   delim == '"'?
    //     "looking for closing '\"' in double-quoted string" :
    //     "looking for closing '`' in backtick-quoted symbol");
    testOneErrorRegex("\"", 1, 2, "end of file.*looking for closing '\"'");
    testOneErrorRegex("\"\\\"", 1, 4, "end of file.*looking for closing '\"'");
    testOneErrorRegex("\"\n", 2, 1, "end of file.*looking for closing '\"'");
    //
    testOneErrorRegex("`", 1, 2, "end of file.*looking for closing '`'");
    testOneErrorRegex("`\\`", 1, 4, "end of file.*looking for closing '`'");
    testOneErrorRegex("`\n", 2, 1, "end of file.*looking for closing '`'");

    // c = readNotEOFCharOrErr(lookingForCharAfterBackslash);
    testOneErrorRegex("\"\\", 1, 3, "end of file.*looking for character after '\\\\' in double");
    //
    testOneErrorRegex("`\\", 1, 3, "end of file.*looking for character after '\\\\' in backtick");

    // unexpectedCharErr(c, lookingForCharAfterBackslash);
    testOneErrorRegex("\"\\z", 1, 3, "'z'.*looking for character after '\\\\' in double");
    //
    testOneErrorRegex("`\\z", 1, 3, "'z'.*looking for character after '\\\\' in backtick");
  }

  // readNextUniversalCharacterEscape
  {
    // readCharOrErr('\\', "expecting '\\'");
    testOneErrorRegex("\"\\ud800", 1, 8, "After high surrogate.*uD800.*end of file.*expecting '\\\\'");
    testOneErrorRegex("\"\\ud8000", 1, 8, "After high surrogate.*uD800.*'0'.*expecting '\\\\'");

    // readCharOrErr('u', "expecting 'u' after '\\'");
    testOneErrorRegex("\"\\ud800\\", 1, 9, "After high surrogate.*uD800.*end of file.*expecting 'u'");
    testOneErrorRegex("\"\\ud800\\n", 1, 9, "After high surrogate.*uD800.*'n'.*expecting 'u'");

    // err(stringf(
    //   "Expected low surrogate in [U+DC00,U+DFFF], "
    testOneErrorRegex("\"\\uDABC\\uDbad", 1, 13, "After high surrogate.*uDABC.*Expected low surrogate.*DBAD");

    // err(stringf(
    //   "Found low surrogate \"\\u%04X\" that is not preceded by "
    testOneErrorRegex("\"\\uDEAD", 1, 7, "Found low surrogate.*uDEAD.*not preceded");
  }

  // readNextU4Escape
  {
    // unexpectedCharErr(c, "looking for digits in \"\\u\" escape sequence in double-quoted string");
    testMultiErrorRegex("\"\\u", {-1, 'x', ' '}, 1, 4, "looking for digits");
    testMultiErrorRegex("\"\\u1", {-1, 'x', ' '}, 1, 5, "looking for digits");
    testMultiErrorRegex("\"\\u123", {-1, 'x', ' '}, 1, 7, "looking for digits");
  }

  // readNextDelimitedCharacterEscape
  {
    // unexpectedCharErr(c,
    //   R"(looking for hex digit immediately after "\u{")");
    testMultiErrorRegex("\"\\u{", {-1, 'x'}, 1, 5, "looking for hex digit immediately");

    // unexpectedCharErr(c,
    //   R"(looking for hex digit or '}' after "\u{")");
    testMultiErrorRegex("\"\\u{A", {-1, 'x'}, 1, 6, "looking for hex digit or '\\}'");
    testMultiErrorRegex("\"\\u{A3", {-1, 'x'}, 1, 7, "looking for hex digit or '\\}'");

    // err(R"(value is larger than 0x10FFFF in "\u{N+}" escape sequence)");
    testOneErrorSubstr("\"\\u{110000", 1, 10, "value is larger");
    testOneErrorSubstr("\"\\u{0110000", 1, 11, "value is larger");
    testOneErrorSubstr("\"\\u{200000", 1, 10, "value is larger");
    testOneErrorSubstr("\"\\u{FFFFFF", 1, 10, "value is larger");
    testOneErrorSubstr("\"\\u{aaaaaa", 1, 10, "value is larger");
  }

  // readNextInteger
  {
    // unexpectedCharErr(firstChar,
    //   "looking for digit after minus sign that starts an integer");
    testMultiErrorRegex("-", {-1, ' ', 'x', ']', '['}, 1, 2, "looking for digit after minus");

    // c = readNotEOFCharOrErr(
    //   "looking for digit after radix indicator in integer");
    testOneErrorRegex("0x", 1, 3, "looking for digit after radix");
    testOneErrorRegex("0o", 1, 3, "looking for digit after radix");

    // putbackAfterValueOrErr(c);
    testMultiErrorRegex("1", {'a', '[', '(', '-'}, 1, 2, "after a value");

    // There is one more call to `err` in `readNextInteger`, in the
    // exception handler, but I believe it is unreachable
  }

  // readNextSymbolOrTaggedContainer
  {
    // These are no longer tied to a unique "err" site, but were in the
    // past, so I keep them as tests.
    testOneErrorSubstr("x{", 1, 3, "value after '{'");
    testOneErrorRegex("true[", 1, 6, "end of file.*value after '\\['");

    // putbackAfterValueOrErr(c);       // Could be EOF, fine.
    testOneErrorSubstr("true!", 1, 5, "'!' after a value");
  }

  // readNextValue
  {
    // unexpectedCharErr(c, "looking for the start of a value");
    testOneErrorSubstr(";", 1, 1, "';' while looking for the start of a value");
  }

  // readExactlyOneValue
  {
    // unexpectedCharErr(readChar(), "looking for the start of a value");
    testMultiErrorRegex("", {-1, ']', '}', ';'}, 1, 1, "looking for the start of a value");
    testOneErrorSubstr("\001", 1, 1, "(0x01)");

    // readEOFOrErr();
    testOneErrorSubstr("1 2", 1, 3, "only have one value");
  }

  // Check for a problem with tagged containers and error locations.
  testOneErrorSubstr("[\nx\n2;1]", 3, 2, "Unexpected ';'");
}


// Check that deserializing 'input' succeeds and yields 'expect'.
void testOneDeserialize(
  char const *input,
  GDValue expect)
{
  try {
    GDValue actual = GDValue::readFromString(input);
    EXPECT_EQ(actual, expect);

    // Exercise `readNextValue`.
    std::istringstream iss((std::string(input)));
    std::optional<GDValue> actualOpt = GDValue::readNextValue(iss);
    xassert(actualOpt.has_value());
    EXPECT_EQ(*actualOpt, expect);

    actualOpt = GDValue::readNextValue(iss);
    xassert(!actualOpt.has_value());
    xassert(iss.eof());
  }
  catch (XBase &x) {
    x.prependContext(stringb("input=" << doubleQuote(input)));
    throw;
  }
}


void testDeserializeMisc()
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

  testOneDeserialize("/***/ 1", 1);
  testOneDeserialize("/* */ 1", 1);
  testOneDeserialize("/* / */ 1", 1);
  testOneDeserialize("/* /* */ */ 1", 1);
  testOneDeserialize("/*/**/*/1", 1);
  testOneDeserialize("/* /** */ */ 1", 1);
  testOneDeserialize("/* /***/ */ 1", 1);

  // Check that we don't somehow recognize comments inside strings.
  testOneDeserialize("\"/*\"", GDVString("/*"));

  // TODO: More here.  I've barely exercised the parser for the cases
  // that are not syntax errors.
}


void testDeserializeIntegers()
{
  // Confirm we can deserialize a value near the top of the range for a
  // signed 64-bit integer.
  testOneDeserialize("1234567890123456789", INT64_C(1234567890123456789));

  // For reference, the maximum uint64_t is 18446744073709551615.

  // Now using smbase::Integer, there is no range limit.
  GDVInteger n123(1234567890);
  GDVInteger n1e10(INT64_C(10000000000));
  GDVInteger n123123 = n123 + n123*n1e10;
  GDVInteger n123123123 = n123 + n123123*n1e10;
  testOneDeserialize("1234567890", n123);
  testOneDeserialize("12345678901234567890", n123123);
  testOneDeserialize("123456789012345678901234567890", n123123123);
  testOneDeserialize("-123456789012345678901234567890", -n123123123);

  // Test comparison between small and large integers.
  xassert(GDValue(    n123) < GDValue(    n123123));
  xassert(GDValue( n123123) < GDValue( n123123123));
  xassert(GDValue(    n123) < GDValue( n123123123));
  xassert(GDValue(   -n123) > GDValue(   -n123123));
  xassert(GDValue(-n123123) > GDValue(-n123123123));
  xassert(GDValue(   -n123) > GDValue(-n123123123));

  // Test the radix decoder.
  testOneDeserialize("0", 0);
  testOneDeserialize("00", 0);
  testOneDeserialize("0099", 99);   // Not octal.
  testOneDeserialize("0b1111", 15);
  testOneDeserialize("0o377", 255);
  testOneDeserialize("0xFF", 255);
  testOneDeserialize("0XFF", 255);
  testOneDeserialize("-0b1111", -15);
  testOneDeserialize("-0o377", -255);
  testOneDeserialize("-0xFF", -255);

  testOneDeserialize(
    "0x1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF",
    GDVInteger::fromRadixDigits(
      "1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF", 16));
}


void testDeserializeStrings()
{
  // Surrogate pair.
  testOneDeserialize(R"("\uD800\uDC00")", utf8EncodeVector({0x10000}));
  testOneDeserialize(R"("\uDBFF\uDFFF")", utf8EncodeVector({0x10FFFF}));

  // Delimited escape
  testOneDeserialize(R"("\u{0}")", utf8EncodeVector({0x0}));
  testOneDeserialize(R"("\u{00000000}")", utf8EncodeVector({0x0}));
  testOneDeserialize(R"("\u{00000000000000001}")", utf8EncodeVector({0x1}));
  testOneDeserialize(R"("\u{10000}")", utf8EncodeVector({0x10000}));
  testOneDeserialize(R"("\u{10FFFF}")", utf8EncodeVector({0x10FFFF}));
  testOneDeserialize(R"("\u{000000000010ffff}")", utf8EncodeVector({0x10FFFF}));
}


// Test the string encoding/escaping.  `plain` is a string to encode as
// a double-quoted string in GDVN.  `expectEncodedNoQuotes` is what it
// should yield, without the double-quotes (just so the tests are a
// little less cluttered).
void testOneStringEscapes(
  std::string const &plain,
  std::string const &expectEncodedNoQuotes)
{
  std::string expectEncoded =
    stringb('"' << expectEncodedNoQuotes << '"');

  std::string actualEncoded =
    GDValue(plain).asString();
  EXPECT_EQ(actualEncoded, expectEncoded);

  std::string actualPlain =
    GDValue::readFromString(actualEncoded).stringGet();
  EXPECT_EQ(actualPlain, plain);
}


// Test decode only.  This is useful when I want to test the
// interpretation of a form my encoder does not produce.
void testOneDecode(
  std::string const &encodedNoQuotes,
  std::string const &expect)
{
  std::string encoded = stringb('"' << encodedNoQuotes << '"');

  std::string actual =
    GDValue::readFromString(encoded).stringGet();
  EXPECT_EQ(actual, expect);
}


// Encode `c` as a UTF-8 string, encoded it as GDVN, decode that, and
// check the result is the original UTF-8 string.
void testOneDecodeCodePoint(int c)
{
  std::string plain = utf8EncodeVector({c});
  std::string encoded = GDValue(plain).asString();
  std::string decoded = GDValue::readFromString(encoded).stringGet();
  try {
    EXPECT_EQ(decoded, plain);
  }
  catch (XBase &x) {
    x.prependContext(stringb("c=" << c));
  }
}


void testStringEscapes()
{
  testOneStringEscapes("", "");
  testOneStringEscapes("\"\\\001\037",
                       "\\\"\\\\\\u{1}\\u{1F}");
  testOneStringEscapes("\t\r\n\f\b/\\\"",
                       "\\t\\r\\n\\f\\b/\\\\\\\"");
  testOneStringEscapes(std::string("\0", 1),
                       "\\u{0}");

  EXPECT_EQ(GDValue(GDVString("has\0nul", 7)).asString(
              GDValueWriteOptions().setUseUndelimitedHexEscapes(true)),
    "\"has\\u0000nul\"");

  // The JSON syntax, and hence GDVN, allows forward slash to be escaped
  // with backslash; I do not know why.
  testOneDecode("\\/", "/");

  //     [--]   [--][-   -][--]
  //        1      2      3   4
  // 1110xxxx 10xxxxxx 10xxxxxx
  //    E   1    8   8    B   4
  // [--][--] [--][--] [--][--]
  testOneDecode("\\u1234", "\xE1\x88\xB4");
  testOneDecode("\\u{1234}", "\xE1\x88\xB4");

  testOneDecodeCodePoint(0x01);
  testOneDecodeCodePoint(0x7F);
  testOneDecodeCodePoint(0x80);
  testOneDecodeCodePoint(0x7FF);
  testOneDecodeCodePoint(0x800);
  testOneDecodeCodePoint(0x1234);
  testOneDecodeCodePoint(0xFFFF);
  testOneDecodeCodePoint(0x10000);
  testOneDecodeCodePoint(0x10FFFF);
}


void testGDValueKindToString()
{
  EXPECT_EQ(toString(GDVK_SYMBOL), "GDVK_SYMBOL");
  EXPECT_EQ(toString(GDVK_MAP), "GDVK_MAP");
}


void testWriteReadFile()
{
  SMFileUtil sfu;
  sfu.createDirectoryAndParents("out/gdvn");

  GDValue v(GDVSequence{1,2,"three"});
  std::string fname("out/gdvn/123.gdvn");
  v.writeToFile(fname);

  GDValue v2 = GDValue::readFromFile(fname);
  EXPECT_EQ(v, v2);

  string contents = sfu.readFileAsString(fname);
  EXPECT_EQ(contents, "[1 2 \"three\"]\n");

  try {
    v.writeToFile("out/gvdn/dir-does-not-exist/123.gdvn");
    xfailure("should have failed");
  }
  catch (XSysError &x) {
    VPVAL(x);
  }

  try {
    GDValue::readFromFile("out/gvdn/file-does-not-exist.gdvn");
    xfailure("should have failed");
  }
  catch (XSysError &x) {
    VPVAL(x);
  }
}


// There is already a test of `readNextValue` above, but it only returns
// one value.  Here I want it to return multiple values before hitting
// EOF.
void testReadNextValue()
{
  std::string input("1 2 3");
  std::istringstream iss(input);

  std::optional<GDValue> v = GDValue::readNextValue(iss);
  EXPECT_EQ(*v, GDValue(1));

  v = GDValue::readNextValue(iss);
  EXPECT_EQ(*v, GDValue(2));

  v = GDValue::readNextValue(iss);
  EXPECT_EQ(*v, GDValue(3));

  v = GDValue::readNextValue(iss);
  xassert(!v.has_value());
  xassert(iss.eof());
}


void testGDValueWriter()
{
  GDValue small = GDVInteger::fromDigits("12345");
  EXPECT_EQ(small.asString(), "12345");

  GDValue big =
    GDVInteger::fromDigits("0x1234567890ABCDEF1234567890ABCDEF");
  EXPECT_EQ(big.asString(), "0x1234567890ABCDEF1234567890ABCDEF");

  EXPECT_EQ(big.asString(
              GDValueWriteOptions().setWriteLargeIntegersAsDecimal(true)),
    "24197857200151252728969465429440056815");
}


void testAsIndentedString()
{
  GDValue v(GDVSequence{1,2});
  GDValueWriteOptions options;
  options.m_targetLineWidth = 1;

  // No final newline.
  EXPECT_EQ(v.asIndentedString(options), "[\n  1\n  2\n]");

  // In contrast:
  EXPECT_EQ(v.asLinesString(options), "[\n  1\n  2\n]\n");
}


struct Has_asGDValue {
  GDValue asGDValue() const
  {
    return GDValue("Has_asGDValue::asGDValue()");
  }
};

struct Has_operator_GDValue {
  operator GDValue() const
  {
    return GDValue("Has_operator_GDValue::operator GDValue()");
  }
};


// Another class with an `operator GDValue()` that I can put into maps
// and has slightly interesting behavior.
struct A {
  int m_i;
  static inline int s_count = 0;

  A()
    : m_i(++s_count)
  {}

  operator GDValue() const
  {
    return GDVTaggedTuple(GDVSymbol("A"), {m_i});
  }

  bool operator< (A const &obj) const
  {
    return m_i < obj.m_i;
  }
};


void testToGDValue()
{
  EXPECT_EQ(toGDValue(Has_asGDValue()).asString(),
            "\"Has_asGDValue::asGDValue()\"");
  EXPECT_EQ(toGDValue(Has_operator_GDValue()).asString(),
            "\"Has_operator_GDValue::operator GDValue()\"");
  EXPECT_EQ(toGDValue(123).asString(),
            "123");
  EXPECT_EQ(toGDValue(GDVSymbol("abc")).asString(),
            "abc");

  EXPECT_EQ(toGDValue(std::set<A>{A(), A(), A()}).asString(),
            "{A(1) A(2) A(3)}");
  EXPECT_EQ(toGDValue(std::vector<A>{A(), A(), A()}).asString(),
            "[A(4) A(5) A(6)]");
  EXPECT_EQ(toGDValue(std::map<A,int>{{A(),17}, {A(),18}}).asString(),
            "{A(7):17 A(8):18}");
  EXPECT_EQ(toGDValue(std::make_pair(std::string("hi"), 9)).asString(),
            "(\"hi\" 9)");

  EXPECT_EQ(toGDValue(true).asString(), "true");
  EXPECT_EQ(toGDValue(false).asString(), "false");
}


void testDefaultWriteOptions()
{
  GDValue v(GDVSequence{1,2,3});
  EXPECT_EQ(v.asString(), "[1 2 3]");

  SAVE_RESTORE(GDValue::s_defaultWriteOptions);
  GDValue::s_defaultWriteOptions.m_targetLineWidth = 4;
  GDValue::s_defaultWriteOptions.m_enableIndentation = true;
  EXPECT_EQ(v.asString(), "[\n  1\n  2\n  3\n]");
}


// Test the operations on maps dedicated to symbol keys.
void testMapSymbolOps()
{
  GDValue m(GDVK_MAP);

  xassert(!m.mapContainsSym("x"));

  m.mapSetSym("x", 1);
  xassert(m.mapContainsSym("x"));
  EXPECT_EQ(m.mapGetSym("x"), GDValue(1));

  {
    GDValue const &cm = m;
    EXPECT_EQ(cm.mapGetSym("x"), GDValue(1));
  }

  GDValue two(2);
  m.mapSetSym("x", two);
  EXPECT_EQ(m.mapGetSym("x"), GDValue(2));

  xassert(!m.mapRemoveSym("y"));
  xassert(m.mapRemoveSym("x"));
  xassert(!m.mapContainsSym("x"));
}


// Test the operations on ordered maps dedicated to symbol keys.
void testOrderedMapSymbolOps()
{
  GDValue m(GDVK_ORDERED_MAP);

  xassert(!m.orderedMapContainsSym("x"));

  m.orderedMapSetSym("x", 1);
  xassert(m.orderedMapContainsSym("x"));
  EXPECT_EQ(m.orderedMapGetSym("x"), GDValue(1));

  {
    GDValue const &cm = m;
    EXPECT_EQ(cm.orderedMapGetSym("x"), GDValue(1));
  }

  GDValue two(2);
  m.orderedMapSetSym("x", two);
  EXPECT_EQ(m.orderedMapGetSym("x"), GDValue(2));

  xassert(!m.orderedMapRemoveSym("y"));
  xassert(m.orderedMapRemoveSym("x"));
  xassert(!m.orderedMapContainsSym("x"));
}


void testScopedSetIndent()
{
  SET_RESTORE(GDValue::s_defaultWriteOptions.m_targetLineWidth, 20);
  SET_RESTORE(GDValue::s_defaultWriteOptions.m_enableIndentation, true);

  GDValue v(GDVSequence{
    "a somewhat long string", "a somewhat long string"
  });
  EXPECT_EQ(v.asString(),
    "[\n"
    "  \"a somewhat long string\"\n"
    "  \"a somewhat long string\"\n"
    "]");

  {
    GDVALUE_SCOPED_SET_INDENT(1);
    EXPECT_EQ(v.asString(),
      "[\n"
      "    \"a somewhat long string\"\n"
      "    \"a somewhat long string\"\n"
      "  ]");
  }

  EXPECT_EQ(v.asString(),
    "[\n"
    "  \"a somewhat long string\"\n"
    "  \"a somewhat long string\"\n"
    "]");
}


void testSymbolLiteralOperator()
{
  EXPECT_EQ(GDVSymbol("name"), "name"_sym);
}


void testGDV_SKV()
{
  int two = 2;
  GDValue v(GDVMap{
    GDV_SKV("one", 1),
    GDV_SKV_EXPR(two),
  });
  EXPECT_EQ(v.asString(), "{one:1 two:2}");
}


void testValueKindCategories()
{
  EXN_CONTEXT("testValueKindCategories");

  struct TestElement {
    // Input kind.
    GDValueKind m_kind;

    // Expected categorizations.
    bool m_isContainer;
    bool m_isTaggedContainer;
    bool m_isOrderedContainer;
    bool m_isUnorderedContainer;
  };

  static TestElement const arr[] = {
    //kind                     c      tc     oc     uc
    { GDVK_SYMBOL,             false, false, false, false },
    { GDVK_INTEGER,            false, false, false, false },
    { GDVK_SMALL_INTEGER,      false, false, false, false },
    { GDVK_STRING,             false, false, false, false },
    { GDVK_SEQUENCE,           true,  false, true,  false },
    { GDVK_TAGGED_SEQUENCE,    true,  true,  true,  false },
    { GDVK_TUPLE,              true,  false, true,  false },
    { GDVK_TAGGED_TUPLE,       true,  true,  true,  false },
    { GDVK_SET,                true,  false, false, true  },
    { GDVK_TAGGED_SET,         true,  true,  false, true  },
    { GDVK_MAP,                true,  false, false, true  },
    { GDVK_TAGGED_MAP,         true,  true,  false, true  },
    { GDVK_ORDERED_MAP,        true,  false, true,  false },
    { GDVK_TAGGED_ORDERED_MAP, true,  true,  true,  false },
  };
  ASSERT_TABLESIZE(arr, NUM_GDVALUE_KINDS);

  for (TestElement const &te : arr) {
    EXN_CONTEXT("kind=" << toString(te.m_kind));
    GDValue v(te.m_kind);

    EXPECT_EQ(v.isContainer(),          te.m_isContainer);
    EXPECT_EQ(v.isTaggedContainer(),    te.m_isTaggedContainer);
    EXPECT_EQ(v.isOrderedContainer(),   te.m_isOrderedContainer);
    EXPECT_EQ(v.isUnorderedContainer(), te.m_isUnorderedContainer);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue()
{
  verbose = !!std::getenv("VERBOSE");

  if (char const *widthStr = std::getenv("GDVALUE_TEST_WIDTH")) {
    // With envvar set, treat it as the target width for the
    // pretty-print tests so I can interactively experiment.
    int width = std::atoi(widthStr);   // gcov-ignore

    testPrettyPrint(width);            // gcov-ignore
  }
  else {
    testNull();
    testBool();
    testSymbol();
    testInteger();
    testString();
    testSequence();
    testTuple();
    testSet();
    testMap();
    testOrderedMap();
    testTaggedSequence();
    testTaggedTuple();
    testTaggedSet();
    testTaggedMap();
    testTaggedOrderedMap();
    testSyntaxErrors();
    testDeserializeMisc();
    testDeserializeIntegers();
    testDeserializeStrings();
    testStringEscapes();
    testGDValueKindToString();
    testWriteReadFile();
    testReadNextValue();
    testGDValueWriter();
    testAsIndentedString();
    testToGDValue();
    testDefaultWriteOptions();
    testMapSymbolOps();
    testOrderedMapSymbolOps();
    testScopedSetIndent();
    testSymbolLiteralOperator();
    testGDV_SKV();
    testValueKindCategories();

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
}


// EOF
