// htt-test.cc
// Tests for `htt` module.

// This file is in the public domain.

#include "smbase/htt.h"                // module under test

#include "smbase/fasttime.h"           // smbase::{fastTimeInitialize, fastTimeMilliseconds}
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Enter a busy-wait loop until the timer ticks.
void shortBusyWaitLoop()
{
  fastTimeInitialize();
  unsigned start = fastTimeMilliseconds;
  while (fastTimeMilliseconds == start)
    {}
}


void test_basics()
{
  HTT_Tree tree;
  tree.selfCheck();

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
    tree.selfCheck();

    {
      [[maybe_unused]] HTT_StartStopLabel ss2(tree, two);

      {
        [[maybe_unused]] HTT_StartStopLabel ss3(tree, three);
        tree.selfCheck();

        // Ensure not all the counts are zero.
        shortBusyWaitLoop();
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
  DIAG(tree.asTableString());
  tree.selfCheck();
}


void test_macros()
{
  {
    HTT_TASK("one");

    {
      HTT_TASK("two");

      {
        HTT_TASK("three");

        shortBusyWaitLoop();
      }
    }
  }

  // These should work in a loop.
  smbase_loopi(3) {
    HTT_TASK("two");

    // Another in the same scope shouldn't cause problems.
    HTT_TASK("two point five");

    {
      HTT_TASK("three");

      shortBusyWaitLoop();
    }
  }

  {
    HTT_TASK("three");
  }

  HTT_Tree &tree = HTT_Singleton::instanceTree();
  DIAG(tree.asTableString());
  tree.selfCheck();
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_htt()
{
  test_basics();
  test_macros();
}


// EOF
