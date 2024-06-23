// stringset-test.cc
// Test code for 'stringset' module.

#include "stringset.h"                 // module to test

#include "exc.h"                       // smbase::xbase

using namespace smbase;


static void testAddUnique()
{
  StringSet ss;
  ss.addUnique("a");
  ss.addUnique("b");
  try {
    ss.addUnique("b");
    xbase("That should have failed!");
  }
  catch (XAssert&) {
    // as expected
  }
  ss.addUnique("c");
  xassert(ss.size() == 3);
}


// Called from unit-tests.cc.
void test_stringset()
{
  // This is not thorough at all right now.
  testAddUnique();
}


// EOF
