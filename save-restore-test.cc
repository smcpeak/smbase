// save-restore-test.cc
// Tests for `save-restore`.

#include "save-restore.h"              // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "xassert.h"                   // xassert


OPEN_ANONYMOUS_NAMESPACE


// Normally, SaveRestore is used with globals or class statics, so use a
// global for testing.
int n = 0;


void testSaveRestore()
{
  xassert(n == 0);

  {
    SAVE_RESTORE(n);
    n += 3;
    xassert(n == 3);
  }

  xassert(n == 0);
}


void testSetRestore()
{
  xassert(n == 0);

  {
    SET_RESTORE(n, 7);
    xassert(n == 7);
  }

  xassert(n == 0);
}


void testAddRestore()
{
  xassert(n == 0);

  {
    ADD_RESTORE(n, 7);
    xassert(n == 7);

    {
      ADD_RESTORE(n, 11);
      xassert(n == 18);
    }

    xassert(n == 7);
  }

  xassert(n == 0);
}


void testIncRestore()
{
  xassert(n == 0);

  {
    INC_RESTORE(n);
    xassert(n == 1);

    {
      INC_RESTORE(n);
      xassert(n == 2);
    }

    xassert(n == 1);
  }

  xassert(n == 0);
}



CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_save_restore()
{
  testSaveRestore();
  testSetRestore();
  testAddRestore();
  testIncRestore();
}


// EOF
