// autofile-test.cc
// Tests for autofile.

#include "autofile.h"                  // this module

#include <cstdlib>                     // std::getenv
#include <iostream>                    // std::{cout, endl}

using std::cout;
using std::endl;


void test_autofile()
{
  char const *fname = "autofile.cc";
  char const *mode = "r";

  if (char const *f = std::getenv("AUTOFILE_TEST_FNAME")) {
    fname = f;
  }
  if (char const *m = std::getenv("AUTOFILE_TEST_MODE")) {
    mode = m;
  }

  cout << "about to open " << fname << " with mode " << mode << endl;

  {
    AutoFILE fp(fname, mode);
    cout << fname << " is now open" << endl;
  }

  cout << fname << " is now closed" << endl;
}


// EOF
