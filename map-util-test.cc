// map-util-test.cc
// Tests for map-util.h.

#include "map-util.h"                  // module under test

#include "exc.h"                       // smbase::XBase

using namespace smbase;


static void testInsertMapUnique()
{
  std::map<int,int> m;
  insertMapUnique(m, 1,1);
  insertMapUnique(m, 2,1);
  insertMapUnique(m, 3,3);
  xassert(m.size() == 3);
  xassert(m[1] == 1);
  xassert(m[2] == 1);
  xassert(m[3] == 3);

  bool inserted = false;
  try {
    // Should fail.
    insertMapUnique(m, 2,2);
    inserted = true;
  }
  catch (XBase &)
  {}
  xassert(!inserted);
}


void test_map_utils()
{
  testInsertMapUnique();
}


// EOF
