// stringf-test.cc
// Tests for stringf module.

// This file is in the public domain.

#include "stringf.h"                   // module under test

#include "sm-test.h"                   // EXPECT_EQ

using namespace smbase;


// Called from unit-tests.cc.
void test_stringf()
{
  std::string actual =
    stringf("int=%d hex=%X str=%s char=%c float=%.1f",
            3, 0xAA, "hi", 'f', 3.5);
  std::string expect = "int=3 hex=AA str=hi char=f float=3.5";
  EXPECT_EQ(actual, expect);

  // Example errors to catch at compile time.
  #if 0
  stringf("%d", 1, 2);
  stringf("%d %d", 1);
  #endif
}


// EOF
