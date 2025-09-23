// ordered-set-test.cc
// Tests for `ordered-set` module.

#include "smbase/ordered-set.h"        // module under test

#include "smbase/gdvalue.h"            // gdv::{GDValue, fromGDVN}
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_{EQ,TRUE,FALSE}

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


using OStrings = OrderedSet<std::string, int>;


void checkContents(
  OStrings const &strings, std::vector<std::string> const &expect)
{
  EXPECT_EQ(strings.size(), expect.size());

  std::size_t i=0;
  for (std::string const &s : strings) {
    EXPECT_EQ(s, expect.at(i));
    ++i;
  }

  xassert(i == expect.size());
}


void test_basics()
{
  OStrings s;
  s.selfCheck();
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.size(), 0);
  EXPECT_FALSE(s.contains("abc"));
  EXPECT_FALSE(s.contains("def"));
  checkContents(s, {});
  EXPECT_EQ_GDV(s, fromGDVN(R"( [] )"));

  int i1 = s.insert("abc");
  EXPECT_EQ(i1, 0);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(s.size(), 1);
  EXPECT_TRUE(s.contains("abc"));
  EXPECT_FALSE(s.contains("def"));
  checkContents(s, {"abc"});
  EXPECT_EQ_GDV(s, fromGDVN(R"( ["abc"] )"));

  int i2 = s.insert("def");
  EXPECT_EQ(i2, 1);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(s.size(), 2);
  EXPECT_TRUE(s.contains("abc"));
  EXPECT_TRUE(s.contains("def"));
  checkContents(s, {"abc", "def"});
  EXPECT_EQ_GDV(s, fromGDVN(R"( ["abc" "def"] )"));

  // Insert (append) an element that would intrinsically sort to the
  // beginning (of, say, `std::set`), but does not do so here.
  int i3 = s.insert("012");
  EXPECT_EQ(i3, 2);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(s.size(), 3);
  EXPECT_TRUE(s.contains("012"));
  EXPECT_TRUE(s.contains("def"));
  checkContents(s, {"abc", "def", "012"});
  EXPECT_EQ_GDV(s, fromGDVN(R"( ["abc" "def" "012"] )"));
}


void test_atC_and_getIndex()
{
  OStrings s = {"apple", "banana", "cherry"};
  s.selfCheck();

  EXPECT_EQ(s.atC(0), "apple");
  EXPECT_EQ(s.atC(1), "banana");
  EXPECT_EQ(s.atC(2), "cherry");

  EXPECT_EQ(s.getIndex("apple"), 0);
  EXPECT_EQ(s.getIndex("banana"), 1);
  EXPECT_EQ(s.getIndex("cherry"), 2);

  checkContents(s, {"apple", "banana", "cherry"});
}


void test_insert_and_insertUnique()
{
  OStrings s;

  int i0 = s.insert("x");
  EXPECT_EQ(i0, 0);
  int i1 = s.insert("y");
  EXPECT_EQ(i1, 1);
  int i2 = s.insert("x");    // duplicate -> returns existing index
  EXPECT_EQ(i2, 0);

  checkContents(s, {"x", "y"});

  // insertUnique with new element
  int i3 = s.insertUnique("z");
  EXPECT_EQ(i3, 2);
  checkContents(s, {"x", "y", "z"});

  EXPECT_EXN_SUBSTR(s.insertUnique("z"),
    XAssert, "assertion failed: !contains");
}


void test_clear_and_swap()
{
  OStrings a = {"a", "b"};
  OStrings b = {"x", "y", "z"};

  a.clear();
  EXPECT_TRUE(a.empty());
  checkContents(a, {});

  a.swapWith(b);
  checkContents(a, {"x", "y", "z"});
  checkContents(b, {});

  using std::swap;
  swap(a,b);
  checkContents(a, {});
  checkContents(b, {"x", "y", "z"});
}


void test_copy_and_move()
{
  OStrings a = {"cat", "dog"};
  OStrings b(a);      // copy ctor
  checkContents(b, {"cat", "dog"});

  OStrings c(std::move(a)); // move ctor
  checkContents(c, {"cat", "dog"});

  OStrings d;
  d = b;              // copy assignment
  checkContents(d, {"cat", "dog"});

  OStrings e;
  e = std::move(b);   // move assignment
  checkContents(e, {"cat", "dog"});
}


void test_iterators()
{
  OStrings s = {"p", "q", "r"};

  // Forward iteration with cbegin/cend
  std::vector<std::string> collected;
  for (auto it = s.cbegin(); it != s.cend(); ++it) {
    collected.push_back(*it);
  }
  EXPECT_EQ(collected, (std::vector<std::string>{"p","q","r"}));

  // Range-for uses begin()/end()
  collected.clear();
  for (auto const &elt : s) {
    collected.push_back(elt);
  }
  EXPECT_EQ(collected, (std::vector<std::string>{"p","q","r"}));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_set()
{
  test_basics();
  test_atC_and_getIndex();
  test_insert_and_insertUnique();
  test_clear_and_swap();
  test_copy_and_move();
  test_iterators();
}


// EOF
