// sm-regex-test.cc
// Tests for sm-regex.

#include "sm-regex.h"                  // module under test

#include <stdlib.h>                    // exit
#include <stdio.h>                     // printf


static void matchVector(char const *str, char const *exp, bool expect)
{
  bool actual = regexpMatch(str, exp);
  if (actual != expect) {
    printf("regexp failure\n");
    printf("  str: %s\n", str);
    printf("  exp: %s\n", exp);
    printf("  expect: %s\n", (expect? "true" : "false"));
    printf("  actual: %s\n", (actual? "true" : "false"));
    exit(2);
  }
}


// Called by unit-tests.cc.
void test_sm_regex()
{
  if (!smregexpModuleWorks()) {
    printf("sm_regex does not work on this platform, skipping test\n");
    return;
  }

  matchVector("abc", "a", true);
  matchVector("abc", "b", true);
  matchVector("abc", "c", true);
  matchVector("abc", "d", false);

  matchVector("abc", "^a", true);
  matchVector("abc", "^b", false);
  matchVector("abc", "b$", false);
  matchVector("abc", "c$", true);
  matchVector("abc", "^d", false);
}


// EOF
