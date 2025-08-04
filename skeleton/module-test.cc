// {module}-test.cc
// Tests for `{module}` module.

#include "smbase/{module}.h"           // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  // ...
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_{module_with_underscores}()
{
  test_basics();
}


// EOF
