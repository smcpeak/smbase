// vdtllist-test.cc
// Tests for vdtllist.

#include "vdtllist.h"                  // module under test

#include <stdio.h>                     // printf


// Called from unit-tests.cc.
void test_vdtllist()
{
  VoidTailList list;
  int zero, one, two, three;

  // This isn't a very exhaustive test; it's mainly to check that
  // selfCheck doesn't do anything really stupid (it used to).

  list.selfCheck();

  list.append(&two);     list.selfCheck();
  list.prepend(&one);    list.selfCheck();
  list.append(&three);   list.selfCheck();
  list.prepend(&zero);   list.selfCheck();

  xassert(list.nth(0) == &zero);
  xassert(list.nth(1) == &one);
  xassert(list.nth(2) == &two);
  xassert(list.nth(3) == &three);

  list.removeAll();
  list.selfCheck();

  printf("vdtllist works\n");
}


// EOF
