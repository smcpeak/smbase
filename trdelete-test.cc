// trdelete-test.cc
// Tests for trdelete.

#include "trdelete.h"                  // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // tprintf

#include <assert.h>                    // assert
#include <stdlib.h>                    // malloc, exit, getenv


OPEN_ANONYMOUS_NAMESPACE


class Foo {
public:
  TRASHINGDELETE
  int junk[10];         // stay away from malloc's data structures
  int x;
  int moreJunk[10];     // more padding
};

class Bar {
public:
  int junk[10];         // stay away from malloc's data structures
  int x;
  int moreJunk[10];     // more padding
};


// Pointer I can use to read memory after 'delete' has done its thing.
// Using this does not avoid the undefined behavior, but it should
// hide the UB from the compiler so the optimizer will not stray from
// my intended behavior.
int volatile * volatile fieldptr;


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_trdelete()
{
  if (getenv("UNDER_VALGRIND")) {
    // Valgrind sees and complains about the deliberate use-after-free.
    tprintf("skipping test due to UNDER_VALGRIND\n");
    return;
  }

  tprintf("malloc: %p\n", malloc(10));

  Foo *f = new Foo;
  f->x = 5;
  fieldptr = &(f->x);
  assert(*fieldptr == 5);
  delete f;
  if (*fieldptr == 5) {
    tprintf("trashing-delete failed\n");
    exit(2);
  }

  Bar *b = new Bar;
  b->x = 7;
  fieldptr = &(b->x);
  assert(*fieldptr == 7);
  delete b;
  if ((unsigned)(*fieldptr) == 0xAAAAAAAAu) {    // did it trash it anyway?
    tprintf("non-trashing-delete failed\n");
    exit(2);
  }

  tprintf("trashing delete works\n");
}


// EOF
