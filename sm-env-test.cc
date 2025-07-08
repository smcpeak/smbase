// sm-env-test.cc
// Tests for `sm-env`.

#include "sm-env.h"                    // module under test

#include "save-restore.h"              // SET_RESTORE
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // VPVAL

#include <map>                         // std::map


using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


typedef std::map<std::string, std::string> EnvMap;

// Map to query during testing.
EnvMap testEnvMap;


// Note: The return value is invalidated if `testEnvMap` is modified.
char const *test_getenv(char const *var)
{
  if (auto it = testEnvMap.find(var); it != testEnvMap.end()) {
    return (*it).second.c_str();
  }
  else {
    return nullptr;
  }
}


void testOneEnvAsBool(char const *name, bool expect)
{
  EXN_CONTEXT("testOneEnvAsBool");
  EXN_CONTEXT_EXPR(name);

  EXPECT_EQ(envAsBool(name), expect);
}


void testEnvAsBool()
{
  testEnvMap = EnvMap{
    { "zero", "0" },
    { "one", "1" },
    { "two", "1" },
    { "zero_one", "01" },
    { "ten", "10" },
    { "alpha", "alpha" },
    { "empty", "" },
  };

  testOneEnvAsBool("zero", false);
  testOneEnvAsBool("one", true);
  testOneEnvAsBool("two", true);
  testOneEnvAsBool("zero_one", true);
  testOneEnvAsBool("ten", true);
  testOneEnvAsBool("alpha", false);
  testOneEnvAsBool("empty", false);
  testOneEnvAsBool("unset", false);
}


void testOneEnvOrEmpty(char const *name, std::string expect)
{
  EXN_CONTEXT("testOneEnvOrEmpty");
  EXN_CONTEXT_EXPR(name);

  std::string actual(envOrEmpty(name));
  EXPECT_EQ(actual, expect);
}


void testEnvOrEmpty()
{
  testEnvMap = EnvMap{
    { "zero", "0" },
    { "alpha", "alpha" },
    { "empty", "" },
  };

  testOneEnvOrEmpty("alpha", "alpha");
  testOneEnvOrEmpty("zero", "0");
  testOneEnvOrEmpty("empty", "");
  testOneEnvOrEmpty("unset", "");
}


void testGetXDGConfigHome()
{
  testEnvMap = EnvMap{
    { "XDG_CONFIG_HOME", "/xdg/config/home" },
    { "HOME", "/home/user" },
  };

  EXPECT_EQ(getXDGConfigHome(), "/xdg/config/home");

  testEnvMap.erase("XDG_CONFIG_HOME");

  EXPECT_EQ(getXDGConfigHome(), "/home/user/.config");

  testEnvMap.erase("HOME");

  EXPECT_EQ(getXDGConfigHome(), ".config");
}


void testActualEnv()
{
  // By setting envvar VERBOSE=1, these can be tested interactively.
  // Otherwise, they at least confirm the functions can be called
  // without crashing.
  VPVAL(envAsBool("VAR"));
  VPVAL(envOrEmpty("VAR"));
  VPVAL(getXDGConfigHome());
}


CLOSE_ANONYMOUS_NAMESPACE

// Called from unit-tests.cc.
void test_sm_env()
{
  // Run these tests with the mock environment.
  {
    SET_RESTORE(sm_getenv_func, &test_getenv);

    testEnvAsBool();
    testEnvOrEmpty();
    testGetXDGConfigHome();
  }

  // Run this with the real environment.
  testActualEnv();
}


// EOF
