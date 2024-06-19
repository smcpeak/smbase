// map-util-test.cc
// Tests for map-util.h.

#include "map-util.h"                  // module under test

#include "exc.h"                       // smbase::XBase
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "stringb.h"                   // stringb

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


void testOstreamInsert()
{
  std::map<int, char const *> m;
  EXPECT_EQ(stringb(m), "{}");

  m.insert({1, "one"});
  EXPECT_EQ(stringb(m), "{ 1: one }");

  m.insert({2, "two"});
  EXPECT_EQ(stringb(m), "{ 1: one, 2: two }");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_map_util()
{
  testInsertMapUnique();
  testMapFindOrNull();
  testOstreamInsert();
}


// EOF
