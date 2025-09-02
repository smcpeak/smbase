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


void test_prependContextWithExnContext()
{
  try {
    EXN_CONTEXT("outer");

    try {
      {
        EXN_CONTEXT("inner");

        xmessage("msg");
      }
    }
    catch (XMessage &x) {
      // This gets inserted between "outer" and "inner" because "outer"
      // is still on the global context stack.
      x.prependContext("prepended");
      throw x;
    }
  }
  catch (XMessage &x) {
    EXPECT_EQ(x.getMessage(), "outer: prepended: inner: msg");
  }
}


void test_getExceptionTypeName()
{
  EXPECT_EQ(getExceptionTypeName(XMessage("msg")), "XMessage");
  EXPECT_EQ(getExceptionTypeName(XAssert("cond", "fname", 1)), "XAssert");
  EXPECT_EQ(getExceptionTypeName(std::exception()), "std::exception");
}


void test_getRelayMessage()
{
  std::string innerMessage;
  std::string innerRelayMessage;
  std::string innerRelayContext;
  std::string outerMessage;

  {
    EXN_CONTEXT("a");

    try {
      EXN_CONTEXT("b");

      try {
        EXN_CONTEXT("c");

        THROW(XMessage("conflict"));
      }
      catch (XBase &x) {
        innerMessage = x.getMessage();
        innerRelayMessage = x.getRelayMessage();
        innerRelayContext = x.getRelayContext();

        THROW(XMessage(stringb("wrapped(" << x.getRelayMessage() << ")")));
      }
    }
    catch (XBase &x) {
      outerMessage = x.getMessage();
    }
  }

  EXPECT_EQ(innerMessage, "a: b: c: conflict");
  EXPECT_EQ(innerRelayMessage, "c: conflict");
  EXPECT_EQ(innerRelayContext, "c: ");
  EXPECT_EQ(outerMessage, "a: b: wrapped(c: conflict)");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_exc()
{
  test_basics();
  test_getExnContextString();
  test_EXN_CONTEXT_FILE_LINE();
  test_prependContextWithExnContext();
  test_getExceptionTypeName();
  test_getRelayMessage();
}


// EOF
