// exc-test.cc
// Tests for exc module.

// This file is in the public domain.

#include "exc.h"                       // module under test

#include "sm-test.h"                   // DIAG

#include <iostream>                    // std::{cout, endl}

using std::cout;
using std::endl;


static bool verbose = false;


// Called from unit-tests.cc.
void test_exc()
{
  XMessage x("yadda");
  DIAG(x);

  try {
    THROW(x);
  }
  catch (XBase &x) {
    DIAG("caught XBase: " << x);
  }
}


// EOF
