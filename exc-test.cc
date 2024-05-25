// exc-test.cc
// Tests for exc module.

// This file is in the public domain.

#include "exc.h"                       // module under test

#include <iostream>                    // std::{cout, endl}

using std::cout;
using std::endl;


// Called from unit-tests.cc.
void test_exc()
{
  XMessage x("yadda");
  cout << x << endl;

  try {
    THROW(x);
  }
  catch (XBase &x) {
    cout << "caught XBase: " << x << endl;
  }
}


// EOF
