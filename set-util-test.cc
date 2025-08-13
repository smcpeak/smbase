// set-util-test.cc
// Tests for `set-util`.

#include "set-util.h"                  // module under test

#include "smbase/gdvalue-set.h"        // gdv::GDValue(std::set)
#include "smbase/gdvalue-span.h"       // gdv::GDValue(smbase::Span)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/exc.h"                // smbase::XAssert
#include "smbase/sm-env.h"             // smbase::envAsIntOr
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-random.h"          // smbase::sm_random
#include "smbase/sm-test.h"            // EXPECT_EQ, envRandomizedTestIters
#include "smbase/stringb.h"            // stringb
#include "smbase/vector-util.h"        // operator<<(vector)
#include "smbase/xassert.h"            // xassert

#include <cstdlib>                     // std::atoi
#include <set>                         // std::set

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testSetInsert()
{
  std::set<int> s;

  bool b = setInsert(s, 1);
  xassert(b);

  b = setInsert(s, 1);
  xassert(!b);

  EXPECT_EQ(stringb(s), "{1}");
}


void testSetInsertUnique()
{
  std::set<int> s;

  setInsertUnique(s, 1);
  EXPECT_EQ(stringb(s), "{1}");

  bool ok = true;
  try {
    setInsertUnique(s, 1);
    ok = false;
  }
  catch (XAssert &x) {}
  xassert(ok);

  EXPECT_EQ(stringb(s), "{1}");
}


void testSetInsertAll()
{
  std::set<int> s;

  xassert(setInsertAll(s, std::set<int>{1,2,3}) == true);
  EXPECT_EQ(stringb(s), "{1, 2, 3}");

  xassert(setInsertAll(s, std::set<int>{1,2,3}) == false);
  EXPECT_EQ(stringb(s), "{1, 2, 3}");
}


void testSetErase()
{
  std::set<int> s;
  EXPECT_EQ(setErase(s, 2), false);

  setInsert(s, 2);
  EXPECT_EQ(setErase(s, 2), true);
  EXPECT_EQ(setErase(s, 2), false);

  xassert(s.empty());
}


void testSetContains()
{
  std::set<int> s{1,3,5};

  xassert(setContains(s, 1));
  xassert(!setContains(s, 2));
  xassert(setContains(s, 3));
}


struct Base {};
struct Derived : Base {};

// Test that `setContains` works when passed a pointer to a derived
// class when the set is declared to contain base class pointers.
void testSetContainsDerived()
{
  Derived d;

  std::set<Base const *> s{&d};

  xassert(setContains(s, &d));
  xassert(!setContains(s, nullptr));
}


void testIsSubsetOf()
{
  std::set<int> s0;
  xassert(isSubsetOf(s0, s0));

  std::set<int> s1{1};
  xassert(isSubsetOf(s0, s1));
  xassert(isSubsetOf(s1, s1));
  xassert(!isSubsetOf(s1, s0));

  std::set<int> s2{1,2};
  xassert(isSubsetOf(s0, s2));
  xassert(isSubsetOf(s1, s2));
  xassert(!isSubsetOf(s2, s1));
}


void testIsSubsetOf_getExtra()
{
  std::set<int> s1{1};
  std::set<int> s12{1,2};

  int extra = 0;
  xassert(isSubsetOf_getExtra(extra /*OUT*/, s1, s12));
  xassert(extra == 0);

  xassert(!isSubsetOf_getExtra(extra /*OUT*/, s12, s1));
  xassert(extra == 2);
}


void testSetHasElementNotIn()
{
  std::set<int> s1{1};
  std::set<int> s12{1,2};
  std::set<int> s23{2,3};

  xassert(setHasElementNotIn(s12, s1) == std::make_optional(2));
  xassert(setHasElementNotIn(s1, s12).has_value() == false);

  xassert(setHasElementNotIn(s12, s23) == std::make_optional(1));
  xassert(setHasElementNotIn(s23, s12) == std::make_optional(3));
  xassert(setHasElementNotIn(s23, s1) == std::make_optional(2));
}


void testSetMapElements()
{
  std::set<char const *> strings{"1", "2", "3"};

  std::set<int> numbers = setMapElements<int>(strings,
    [](char const *s) -> int {
      return std::atoi(s);
    });

  EXPECT_EQ(stringb(numbers), "{1, 2, 3}");
}


void testSetToVector()
{
  std::vector<int> v = setToVector(std::set<int>{1,2,3});
  EXPECT_EQ(stringb(v), "[1 2 3]");
}


void test_setRemove()
{
  std::set<int> s{1,2};
  EXPECT_EQ(setRemove(s, 1), true);
  EXPECT_EQ(setRemove(s, 1), false);
  setRemoveExisting(s, 2);
  xassert(s.empty());

  EXPECT_EXN_SUBSTR(setRemoveExisting(s, 2), XAssert, "erased");
}


void testOstreamInsert()
{
  std::set<int> s;
  EXPECT_EQ(stringb(s), "{}");

  s.insert(1);
  EXPECT_EQ(stringb(s), "{1}");

  s.insert(2);
  EXPECT_EQ(stringb(s), "{1, 2}");
}


void testSetWriter()
{
  std::set<int> s{1,2};
  auto printElement = [](std::ostream &os, int i) -> void {
    os << "(" << i << ")";
  };
  EXPECT_EQ(stringb(setWriter(s, printElement)), "{(1), (2)}");
}


void testOne_setIsDisjointWith(
  std::set<int> const &a,
  std::set<int> const &b,
  bool expect)
{
  TEST_CASE_EXPRS("testOne_setIsDisjointWith", a, b);
  EXPECT_EQ(setIsDisjointWith(a, b), expect);
  EXPECT_EQ(setIsDisjointWith(b, a), expect);

  // Test the general algorithm with just two sets.
  std::vector<ConstIterAndEnd<std::set<int>>> iterAndEnds = {
    constIterAndEnd(a),
    constIterAndEnd(b)
  };

  // We need to explicitly say `Span` here because otherwise the
  // compiler fails to deduce the template arguments for
  // `setsAreDisjoint` before it gets to the stage of considering
  // implicit conversions.
  EXPECT_EQ(setsAreDisjoint(Span(iterAndEnds)), expect);

  // Swap the order.  (Note that the previous iterators have been
  // modified, so we need to recreate them.)
  iterAndEnds[0] = constIterAndEnd(b);
  iterAndEnds[1] = constIterAndEnd(a);
  EXPECT_EQ(setsAreDisjoint(Span(iterAndEnds)), expect);
}


void test_setIsDisjointWith()
{
  testOne_setIsDisjointWith({}, {}, true);
  testOne_setIsDisjointWith({1}, {}, true);
  testOne_setIsDisjointWith({1}, {2}, true);
  testOne_setIsDisjointWith({1,2}, {2}, false);
  testOne_setIsDisjointWith({1,2,3,4}, {3,5,6}, false);
  testOne_setIsDisjointWith({1,2,4}, {3,5,6}, true);
  testOne_setIsDisjointWith({1,2,6}, {3,5,6}, false);
  testOne_setIsDisjointWith({1,2,6}, {1,5,7}, false);
  testOne_setIsDisjointWith({1,2,6}, {1,2,6}, false);
}


void testOne_setsAreDisjoint(
  std::set<int> const &a,
  std::set<int> const &b,
  std::set<int> const &c,
  bool expect)
{
  TEST_CASE_EXPRS("testOne_setsAreDisjoint", a, b, c);

  std::set<int> const *sets[] = { &a, &b, &c };

  // All permutations of the sets should yield the same result.
  int permutations[][3] = {
    { 0, 1, 2 },
    { 0, 2, 1 },
    { 1, 0, 2 },
    { 1, 2, 0 },
    { 2, 0, 1 },
    { 2, 1, 0 },
  };

  for (int (&permutation)[3] : permutations) {
    EXN_CONTEXT(toGDValue(Span(permutation)));
    ConstIterAndEnd<std::set<int>> iterAndEnds[] = {
      constIterAndEnd(*( sets[permutation[0]] )),
      constIterAndEnd(*( sets[permutation[1]] )),
      constIterAndEnd(*( sets[permutation[2]] ))
    };
    EXPECT_EQ(setsAreDisjoint(Span(iterAndEnds)), expect);
  }
}


void test_setsAreDisjoint()
{
  testOne_setsAreDisjoint(
    {},
    {},
    {},
    true);

  testOne_setsAreDisjoint(
    {1},
    {},
    {},
    true);

  testOne_setsAreDisjoint(
    {1},
    {2},
    {},
    true);

  testOne_setsAreDisjoint(
    {2},
    {2},
    {},
    false);

  testOne_setsAreDisjoint(
    {1},
    {2},
    {3},
    true);

  testOne_setsAreDisjoint(
    {1},
    {2},
    {2},
    false);

  testOne_setsAreDisjoint(
    {2},
    {2},
    {2},
    false);

  testOne_setsAreDisjoint(
    {1,4},
    {2},
    {3},
    true);

  testOne_setsAreDisjoint(
    {1,4},
    {2},
    {3},
    true);

  testOne_setsAreDisjoint(
    {1,4},
    {2,5},
    {3},
    true);

  testOne_setsAreDisjoint(
    {1,4},
    {2,5},
    {3,6},
    true);

  testOne_setsAreDisjoint(
    {1,2},
    {2,5},
    {3,6},
    false);

  testOne_setsAreDisjoint(
    {1,4},
    {2,5},
    {3,2},
    false);
}


void test_setsAreDisjointRandomized()
{
  TEST_CASE("test_setsAreDisjointRandomized");

  // Overall iteration count; orthogonal to other parameters.
  int const numIters = envRandomizedTestIters(100, "SADR_ITERS", 1);

  // Sizes that need to be kept balanced for the test to have diagnostic
  // value.  "Balanced" means the final `numDisjoint` is approximately
  // half of `numIters`.
  int const numInsertions = envAsIntOr(10, "SADR_INSERTIONS");
  int const numValues = envAsIntOr(50, "SADR_VALUES");
  int const numSets = envAsIntOr(4, "SADR_SETS");

  VPVAL(numIters);
  VPVAL(numInsertions);
  VPVAL(numValues);
  VPVAL(numSets);

  // Number of sets of sets that ended up being disjoint.  Ideally,
  // about half of `numSets` would be disjoint.
  int numDisjoint = 0;

  smbase_loopi(numIters) {
    EXN_CONTEXT_EXPR(i);

    // All elements in all sets (running union).
    std::set<int> allElements;

    // The set of sets to test disjointness of.
    std::vector<std::set<int>> sets(numSets);

    // True until we cause `sets` to not be disjoint.
    bool isDisjoint = true;

    // Populate the sets by performing a total of `numInsertions`
    // insertions, although some may be duplicates.
    smbase_loopj(numInsertions) {
      int v = sm_random(numValues);
      int index = sm_random(numSets);

      if (setInsert(sets.at(index), v)) {
        // We inserted `v`.  Was it already in another set?
        if (!setInsert(allElements, v)) {
          // It was, so we know the sets will not be disjoint.
          isDisjoint = false;
        }
      }
    }

    // Prepare to call `setsAreDisjoint`.
    std::vector<ConstIterAndEnd<std::set<int>>> iterAndEnds;
    smbase_loopj(numSets) {
      iterAndEnds.push_back(constIterAndEnd(sets.at(j)));
    }

    // See if we get the right answer.
    EXPECT_EQ(setsAreDisjoint(Span(iterAndEnds)), isDisjoint);

    if (isDisjoint) {
      ++numDisjoint;
    }
  }

  VPVAL(numDisjoint);
  if (numIters > 0) {
    DIAG("ratio: " << (float)numDisjoint / (float)numIters);
  }
}


void test_setUnion()
{
  EXPECT_EQ(
    stringb(setUnion(
      std::set<int>{1,2,3},
      std::set<int>{3,4,5}
    )),
    "{1, 2, 3, 4, 5}");
}


void test_setRemoveMany()
{
  std::set<int> s1{1,2,3,4};
  EXPECT_EQ(setRemoveMany(s1, std::set<int>{3,4,5,6}), 2);
  EXPECT_EQ(stringb(s1), "{1, 2}");
}


void test_setInsertMany()
{
  // Use strings to at least motivate the use of element moves, but I'm
  // not actually measuring it.
  std::set<std::string> s1{"a","b","c","d"};
  std::set<std::string> s2{"c","d","e","f"};

  setInsertMany(s1, std::move(s2));

  EXPECT_EQ(stringb(s1), "{a, b, c, d, e, f}");

  // Although not guaranteed, I expect `s2` to retain two elements.
  EXPECT_EQ(stringb(s2), "{c, d}");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_set_util()
{
  testSetInsert();
  testSetInsertUnique();
  testSetInsertAll();
  testSetErase();
  testSetContains();
  testSetContainsDerived();
  testIsSubsetOf();
  testIsSubsetOf_getExtra();
  testSetHasElementNotIn();
  testSetMapElements();
  testSetToVector();
  test_setRemove();
  testOstreamInsert();
  testSetWriter();
  test_setIsDisjointWith();
  test_setsAreDisjoint();
  test_setsAreDisjointRandomized();
  test_setUnion();
  test_setRemoveMany();
  test_setInsertMany();
}


// EOF
