// test-stringset.cc
// Test code for 'stringset' module.

#include "stringset.h"                 // module to test

#include "sm-iostream.h"               // cout
#include "sm-test.h"                   // USUAL_MAIN


static void testAddUnique()
{
  StringSet ss;
  ss.addUnique("a");
  ss.addUnique("b");
  try {
    cout << "This should throw:" << endl;
    ss.addUnique("b");
    xbase("That should have failed!");
  }
  catch (x_assert&) {
    // as expected
  }
  ss.addUnique("c");
  xassert(ss.size() == 3);
}


static void entry()
{
  // This is not thorough at all right now.
  testAddUnique();

  cout << "test-stringset passed\n";
}

USUAL_MAIN


// EOF
