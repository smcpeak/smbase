// optional-opll-test.cc
// Tests for `optional-opll` module.

#include "smbase/optional-opll.h"           // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/stringb.h"            // stringb

using namespace smbase;


// Called from unit-tests.cc.
void test_optional_opll()
{
  std::optional<int> none;
  std::optional<int> one(1);

  EXPECT_EQ(stringb(none), "null");
  EXPECT_EQ(stringb(one), "1");
}


// EOF
