// autofile-test.cc
// Tests for autofile.

#include "autofile.h"                  // this module

#include "sm-test.h"                   // DIAG

#include <cstdlib>                     // std::getenv
#include <iostream>                    // std::{cout, endl}

using std::cout;
using std::endl;


static bool verbose = false;


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

  DIAG("about to open " << fname << " with mode " << mode);

  {
    AutoFILE fp(fname, mode);
    DIAG(fname << " is now open");
  }

  DIAG(fname << " is now closed");
}


// EOF
