// unit-tests.cc
// Unit test driver program.

// Most modules in smbase are tested in their own separate test
// executables, but I would like to start consolidating them into a
// single unit-test program.  Since the 'overflow' module, imported from
// the 'chess' repo, already works that way, it will be the first module
// to be tested in the combined unit test program.

#include "overflow.h"                  // test_overflow
#include "parsestring.h"               // test_parsestring
#include "test.h"                      // USUAL_TEST_MAIN


static void entry()
{
  test_overflow();
  test_parsestring();

  cout << "unit tests PASSED" << endl;
}


USUAL_TEST_MAIN

// EOF
