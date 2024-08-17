// ordered-map-test.cc
// Tests for `ordered-map` module.

#include "smbase/ordered-map-ops.h"    // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, DIAG
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/stringb.h"            // stringb

#include <sstream>                     // std::ostringstream
#include <string>                      // std::string
#include <utility>                     // std::{move, swap}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


std::string toGDVN(int i)
{
  return stringb(i);
}


std::string toGDVN(char const *str)
{
  return doubleQuote(str);
}


// Render as a GDVN string, but without actually using the GDV library
// because I do not want to depend on GDV here, as this component will
// be used by GDV.
template <typename KEY, typename VALUE>
std::string toGDVN(OrderedMap<KEY, VALUE> const &m)
{
  std::ostringstream os;

  os << "[";

  if (m.empty()) {
    os << ":";
  }
  else {
    int ct = 0;
    for (auto const &kv : m) {
      if (ct++ > 0) {
        os << " ";
      }
      os << toGDVN(kv.first) << ":" << toGDVN(kv.second);
    }
  }

  os << "]";

  return os.str();
}


template <typename KEY, typename VALUE>
void printOM(OrderedMap<KEY, VALUE> const &m, char const *label)
{
  DIAG(label << ": " << toGDVN(m));
  m.selfCheck();
}


void testCtors()
{
  // Initializer list ctor.
  OrderedMap<int, int> m{
    {2, 22},
    {1, 11},
    {3, 33},
  };

  printOM(m, "m");
  xassert(!m.empty());
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:33]");

  // Insert.
  xassert(m.insert({9, 99}));
  printOM(m, "m");
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:33 9:99]");

  // Insert when key is already present.
  xassert(!m.insert({9, 999}));
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:33 9:99]");

  // Count.
  xassert(m.count(0) == 0);
  xassert(m.count(1) == 1);
  xassert(m.count(2) == 1);
  xassert(m.count(3) == 1);
  xassert(m.count(4) == 0);

  // Contains.
  xassert(m.contains(0) == false);
  xassert(m.contains(1) == true);
  xassert(m.contains(2) == true);
  xassert(m.contains(3) == true);
  xassert(m.contains(4) == false);

  // Copy ctor.
  OrderedMap<int, int> m2(m);
  EXPECT_EQ(toGDVN(m2), "[2:22 1:11 3:33 9:99]");

  m2.clear();
  EXPECT_EQ(toGDVN(m2), "[:]");

  // Copy assignment.
  m2 = m;
  EXPECT_EQ(toGDVN(m2), "[2:22 1:11 3:33 9:99]");
  m2.selfCheck();

  // Move assignment.
  OrderedMap<int, int> m3;
  m3 = std::move(m2);
  EXPECT_EQ(toGDVN(m3), "[2:22 1:11 3:33 9:99]");
  m3.selfCheck();

  // Move copy ctor.
  OrderedMap<int, int> m4(std::move(m3));
  EXPECT_EQ(toGDVN(m4), "[2:22 1:11 3:33 9:99]");
  m4.selfCheck();
}


void testIteratorInvalidation()
{
  OrderedMap<int, int> m;
  xassert(m.empty());
  EXPECT_EQ(toGDVN(m), "[:]");

  {
    auto it = m.begin();
    xassert(it.isValid());

    m.insert({1,1});
    xassert(!it.isValid());
  }

  xassert(m.size() == 1);
  xassert(!m.empty());

  {
    auto it = m.begin();
    m.clear();
    xassert(!it.isValid());
  }

  xassert(m.size() == 0);
  xassert(m.empty());
}


void testElementAccess()
{
  using Entry = OrderedMap<int, int>::value_type;

  OrderedMap<int, int> m{{2,22}, {1,11}, {3,33}};
  xassert(m.size() == 3);

  OrderedMap<int, int> const &mc = m;

  // Test `entryAtKey`.
  xassert(m.entryAtKey(1) == Entry(1,11));
  xassert(m.entryAtKey(2) == Entry(2,22));
  xassert(m.entryAtKey(3) == Entry(3,33));

  // Test `indexOfKey`.
  xassert(m.indexOfKey(1) == 1);
  xassert(m.indexOfKey(2) == 0);
  xassert(m.indexOfKey(3) == 2);

  // Modify an entry's value when accessed via key.
  m.entryAtKey(2).second = 2222;
  xassert(m.entryAtKey(2) == Entry(2,2222));
  xassert(m.valueAtKey(2) == 2222);
  xassert(mc.valueAtKey(2) == 2222);
  EXPECT_EQ(toGDVN(m), "[2:2222 1:11 3:33]");

  // Test `entryAtIndex`.
  xassert(m.entryAtIndex(0) == Entry(2,2222));
  xassert(m.entryAtIndex(1) == Entry(1,11));
  xassert(m.entryAtIndex(2) == Entry(3,33));

  // Modify an entry's value when accessed via index.
  m.entryAtIndex(1).second = 111;
  xassert(m.entryAtIndex(1) == Entry(1,111));
  xassert(m.valueAtIndex(1) == 111);
  xassert(mc.valueAtIndex(1) == 111);
  EXPECT_EQ(toGDVN(m), "[2:2222 1:111 3:33]");
}


void testErase()
{
  OrderedMap<int, int> m{{2,22}, {1,11}, {3,33}};
  xassert(m.size() == 3);

  auto it = m.begin();
  xassert(it.isValid());

  xassert(!m.eraseKey(0));
  xassert(m.size() == 3);
  xassert(!it.isValid());

  xassert(m.eraseKey(1));
  xassert(m.size() == 2);
  EXPECT_EQ(toGDVN(m), "[2:22 3:33]");
  m.selfCheck();

  it = m.begin();
  xassert(it.isValid());

  m.eraseIndex(1);
  xassert(m.size() == 1);
  EXPECT_EQ(toGDVN(m), "[2:22]");
  xassert(!it.isValid());
  m.selfCheck();
}


void testSwap()
{
  OrderedMap<int, int> m1{{2,22}, {1,11}, {3,33}};
  OrderedMap<int, int> m2{{7,77}, {5,55}};

  m1.swap(m2);

  EXPECT_EQ(toGDVN(m1), "[7:77 5:55]");
  EXPECT_EQ(toGDVN(m2), "[2:22 1:11 3:33]");

  using std::swap;
  swap(m1, m2);

  EXPECT_EQ(toGDVN(m2), "[7:77 5:55]");
  EXPECT_EQ(toGDVN(m1), "[2:22 1:11 3:33]");
}


void testCompare()
{
  EXN_CONTEXT("testCompare");

  // A stricly increasing sequence of ordered maps.
  std::vector<OrderedMap<int, int> > maps{
    {},
    {{1,1}},
    {{1,1}, {0,2}},
    {{1,1}, {2,2}},
    {{1,2}},
    {{2,1}},
    {{2,1}, {3,3}},
    {{2,2}},
  };

  // Test all pairs.
  for (std::size_t i = 0; i < maps.size(); ++i) {
    EXN_CONTEXT_EXPR(i);

    for (std::size_t j = 0; j < maps.size(); ++j) {
      EXN_CONTEXT_EXPR(j);

      EXPECT_EQ(compare(maps[i], maps[j]), compare(i, j));
    }
  }
}


void testInsertAtIndex()
{
  OrderedMap<int, int> m;
  EXPECT_EQ(toGDVN(m), "[:]");

  m.insertAtIndex(0, {2,22});
  EXPECT_EQ(toGDVN(m), "[2:22]");

  m.insertAtIndex(0, {3,33});
  EXPECT_EQ(toGDVN(m), "[3:33 2:22]");

  m.insertAtIndex(1, {4,44});
  EXPECT_EQ(toGDVN(m), "[3:33 4:44 2:22]");

  m.insertAtIndex(3, {5,55});
  EXPECT_EQ(toGDVN(m), "[3:33 4:44 2:22 5:55]");

  m.insertAtIndex(3, {1,11});
  EXPECT_EQ(toGDVN(m), "[3:33 4:44 2:22 1:11 5:55]");

  m.eraseIndex(1);
  EXPECT_EQ(toGDVN(m), "[3:33 2:22 1:11 5:55]");

  try {
    m.insertAtIndex(3, {1,11});
    xfailure("should have failed");
  }
  catch (XBase &x) {
    EXPECT_HAS_SUBSTRING(x.what(), "already mapped");
  }
}


void testReadOnlyIteration()
{
  OrderedMap<int, int> m = {{2,22}, {1,11}, {3,33}};
  OrderedMap<int, int> const &cm = m;

  using Entry = OrderedMap<int, int>::value_type;

  auto cit = cm.begin();
  auto it = m.begin();

  auto cit_end = cm.end();
  auto it_end = m.end();

  xassert(cit.isValid());
  xassert(it.isValid());

  xassert(!cit.isEnd());
  xassert(!it.isEnd());

  xassert(cit != cit_end);
  xassert(it != it_end);

  xassert(*cit == Entry(2,22));
  xassert(*it == Entry(2,22));

  ++cit;
  ++it;

  xassert(!cit.isEnd());
  xassert(!it.isEnd());

  xassert(cit != cit_end);
  xassert(it != it_end);

  xassert(*cit == Entry(1,11));
  xassert(*it == Entry(1,11));

  ++cit;
  ++it;

  xassert(!cit.isEnd());
  xassert(!it.isEnd());

  xassert(cit != cit_end);
  xassert(it != it_end);

  xassert(*cit == Entry(3,33));
  xassert(*it == Entry(3,33));

  ++cit;
  ++it;

  xassert(cit.isEnd());
  xassert(it.isEnd());

  xassert(cit == cit_end);
  xassert(it == it_end);
}


void testMutatingIteration()
{
  OrderedMap<int, int> m = {{2,22}, {1,11}, {3,33}};

  for (auto &kv : m) {
    if (kv.first > 1) {
      kv.second += 100;
    }
  }

  EXPECT_EQ(toGDVN(m), "[2:122 1:11 3:133]");
}


void testInsertRvalue()
{
  OrderedMap<int, int> m = {{2,22}, {1,11}, {3,33}};
  using Entry = OrderedMap<int, int>::value_type;

  xassert(m.insert(Entry(-5,55)));
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:33 -5:55]");

  xassert(!m.insert(Entry(1,1111)));
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:33 -5:55]");
}


void testSetValueAtKey()
{
  OrderedMap<int, int> m = {{2,22}, {1,11}, {3,33}};

  // Rvalue reference.
  m.setValueAtKey(3, 3333);
  EXPECT_EQ(toGDVN(m), "[2:22 1:11 3:3333]");

  // Lvalue reference.
  int k = 2;
  int v = 2222;
  m.setValueAtKey(k, v);
  EXPECT_EQ(toGDVN(m), "[2:2222 1:11 3:3333]");
}


// Lightly exercise the container with a value type different than the
// key.
void testDifferentValueType()
{
  OrderedMap<int, char const *> m = {{1, "one"}};
  EXPECT_EQ(toGDVN(m), "[1:\"one\"]");

  m.insert({-1, "negone"});
  EXPECT_EQ(toGDVN(m), "[1:\"one\" -1:\"negone\"]");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_map()
{
  testCtors();
  testIteratorInvalidation();
  testElementAccess();
  testErase();
  testSwap();
  testCompare();
  testInsertAtIndex();
  testReadOnlyIteration();
  testMutatingIteration();
  testInsertRvalue();
  testSetValueAtKey();
  testDifferentValueType();
}


// EOF
