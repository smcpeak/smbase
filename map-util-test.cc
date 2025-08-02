// map-util-test.cc
// Tests for map-util.h.

#include "map-util.h"                  // module under test

#include "exc.h"                       // smbase::XBase
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "stringb.h"                   // stringb

#include <memory>                      // std::unique_ptr

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testInsertMapUnique()
{
  std::map<int,int> m;
  mapInsertUnique(m, 1,1);
  mapInsertUnique(m, 2,1);
  mapInsertUnique(m, 3,3);
  xassert(m.size() == 3);
  xassert(m[1] == 1);
  xassert(m[2] == 1);
  xassert(m[3] == 3);

  bool inserted = false;
  try {
    // Should fail.
    mapInsertUnique(m, 2,2);
    inserted = true;
  }
  catch (XBase &)
  {}
  xassert(!inserted);
}


void testMapFindOrNull()
{
  std::map<int, char const *> m;

  char const *one = "one";
  mapInsert(m, 1, one);

  xassert(mapFindOrNull(m, 1) == one);
  xassert(mapFindOrNull(m, 3) == nullptr);

  // Also test `mapRemove`.
  xassert(mapRemove(m, 3) == false);

  try {
    mapRemoveExisting(m, 3);
    xmessage("should have failed");
  }
  catch (XAssert &) {
    // As expected.
  }

  mapRemoveExisting(m, 1);
  xassert(m.empty());
}


void testMapMoveValueAt()
{
  std::map<int, std::string> m {
    { 1, "one" },
    { 2, "two" },
  };
  EXPECT_EQ(mapMoveValueAt(m, 1), "one");
  EXPECT_EQ(mapMoveValueAt(m, 2), "two");
  xassert(m.empty());
}


void testOstreamInsert()
{
  std::map<int, char const *> m;
  EXPECT_EQ(stringb(m), "{}");

  m.insert({1, "one"});
  EXPECT_EQ(stringb(m), "{ 1: one }");

  m.insert({2, "two"});
  EXPECT_EQ(stringb(m), "{ 1: one, 2: two }");
}


void test_mapGetValueAt()
{
  std::map<int, int> m = {
    { 1, 2 },
    { 3, 4 },
  };
  EXPECT_EQ(mapGetValueAtC(m, 1), 2);
  EXPECT_EQ(mapGetValueAtC(m, 3), 4);
  EXPECT_EXN(mapGetValueAtC(m, 5), XAssert);

  EXPECT_EQ(mapGetValueAt(m, 3), 4);

  // When passed non-const, result is mutable.
  mapGetValueAt(m, 3) = 44;

  std::map<int, int> const &cm = m;

  EXPECT_EQ(mapGetValueAt(cm, 3), 44);
  EXPECT_EQ(mapGetValueAtC(cm, 3), 44);

  // Also test `mapContains`.
  EXPECT_EQ(mapContains(m, 1), true);
  EXPECT_EQ(mapContains(m, 5), false);
  EXPECT_EQ(mapContains(cm, 1), true);
  EXPECT_EQ(mapContains(cm, 5), false);
}


void test_mapInsertMove_unique_ptr()
{
  std::map<int, std::unique_ptr<int>> m;

  // For reasons I don't understand, Clang accepts this even without the
  // "Move" part.  GCC requires "Move", which seems more correct.
  mapInsertMove(m, 1, std::make_unique<int>(2));

  EXPECT_EQ(mapGetValueAt(m, 1).get()[0], 2);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_map_util()
{
  testInsertMapUnique();
  testMapFindOrNull();
  testMapMoveValueAt();
  testOstreamInsert();
  test_mapGetValueAt();
  test_mapInsertMove_unique_ptr();
}


// EOF
