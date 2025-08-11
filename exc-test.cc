// exc-test.cc
// Tests for exc module.

// This file is in the public domain.

#include "smbase/exc.h"                // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // DIAG, verbose, EXPECT_EQ

#include <iostream>                    // std::{cout, endl}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
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


void test_getExnContextString()
{
  EXPECT_EQ(getExnContextString(), "");

  {
    EXN_CONTEXT("blah");
    EXPECT_EQ(getExnContextString(), "blah: ");

    {
      EXN_CONTEXT("goo");
      EXPECT_EQ(getExnContextString(), "blah: goo: ");
    }

    EXPECT_EQ(getExnContextString(), "blah: ");
  }

  EXPECT_EQ(getExnContextString(), "");
}


void test_EXN_CONTEXT_FILE_LINE()
{
  EXN_CONTEXT_FILE_LINE();
  VPVAL(getExnContextString());
  EXPECT_HAS_SUBSTRING(getExnContextString(), "exc-test.cc:");

  try {
    THROW(XMessage("something"));
  }
  catch (XMessage &x) {
    VPVAL(x);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_exc()
{
  test_basics();
  test_getExnContextString();
  test_EXN_CONTEXT_FILE_LINE();
}


// EOF
