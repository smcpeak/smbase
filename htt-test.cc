// htt-test.cc
// Tests for `htt` module.

// This file is in the public domain.

#include "smbase/htt.h"                // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  HTT_Tree tree;
  HTT_LabelIndex one = tree.getIndexForLabel("one");
  HTT_LabelIndex two = tree.getIndexForLabel("two");
  HTT_LabelIndex three = tree.getIndexForLabel("three");

  /* Create a tree that looks like:
     - root
       - one
         - two
           - three
       - two
         - three
       - three
  */

  {
    [[maybe_unused]] HTT_StartStopLabel ss1(tree, one);

    {
      [[maybe_unused]] HTT_StartStopLabel ss2(tree, two);

      {
        [[maybe_unused]] HTT_StartStopLabel ss3(tree, three);
      }
    }
  }

  {
    [[maybe_unused]] HTT_StartStopLabel ss2(tree, two);

    {
      [[maybe_unused]] HTT_StartStopLabel ss3(tree, three);
    }
  }

  {
    [[maybe_unused]] HTT_StartStopLabel ss3(tree, three);
  }

  VPVAL(tree.asString());
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_htt()
{
  test_basics();
}


// EOF
