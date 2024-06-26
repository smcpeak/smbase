// exc-test.cc
// Tests for exc module.

// This file is in the public domain.

#include "exc.h"                       // module under test

#include "sm-test.h"                   // DIAG, verbose

#include <iostream>                    // std::{cout, endl}

using namespace smbase;

using std::cout;
using std::endl;


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
