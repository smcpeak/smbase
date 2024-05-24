// voidlist-test.cc
// Tests for voidlist.

#include "voidlist.h"                  // module under test

#include "sm-stdint.h"                 // intptr_t

#include <stdio.h>                     // printf
#include <stdlib.h>                    // rand


// assumes we're using pointerAddressDiff as the comparison fn
// (I don't use isSorted because this fn will throw at the disequality,
// whereas isSorted would forget that info)
static void verifySorted(VoidList const &list)
{
  const void* prev = 0;
  VoidListIter iter(list);
  for (; !iter.isDone(); iter.adv()) {
    const void *current = (const void *)iter.data();
    xassert(prev <= current);    // numeric address test
    prev = current;
  }
}


#define PRINT(lst) printf("%s: ", #lst); lst.debugPrint(); printf("\n") /* user ; */

static void testSorting()
{
  enum { ITERS=100, ITEMS=20 };

  smbase_loopi(ITERS) {
    // construct a list (and do it again if it ends up already sorted)
    VoidList list1;
    VoidList list3;     // this one will be constructed sorted one at a time
    int numItems;
    do {
      list1.removeAll();    // clear them in case we have to build it more than once
      list3.removeAll();
      numItems = rand()%ITEMS;
      smbase_loopj(numItems) {
        void *toInsert = (void*)(intptr_t)( (rand()%ITEMS) * 4 );
        list1.prepend(toInsert);
        list3.insertSorted(toInsert, VoidList::pointerAddressDiff);
      }
    } while (list1.isSorted(VoidList::pointerAddressDiff));

    // list3 should be sorted already
    //PRINT(list3);
    verifySorted(list3);

    // duplicate it for use with other algorithm
    VoidList list2;
    list2 = list1;

    // sort them
    list1.insertionSort(VoidList::pointerAddressDiff);
    xassert(list1.equalAsPointerSets(list2));      // verify intermediate equality
    xassert(!list1.equalAsPointerLists(list2));    // but not elementwise
    list2.mergeSort(VoidList::pointerAddressDiff);
    //PRINT(list1);

    // verify structure
    list1.selfCheck();
    list2.selfCheck();

    // verify length
    xassert(list1.count() == numItems && list2.count() == numItems);

    // verify sortedness
    verifySorted(list1);
    verifySorted(list2);

    // verify equality
    xassert(list1.equalAsPointerLists(list2));
    xassert(list1.equalAsPointerLists(list3));

    // to test as-sets equality
    void *first = list1.first();
    while (list1.removeIfPresent(first))
      {}     // remove all occurrances of 'first'
    xassert(!list1.equalAsPointerSets(list2));
  }
}


// Called from unit-tests.cc.
void test_voidlist()
{
  // first set of tests
  {
    // some sample items
    void *a=(void*)4, *b=(void*)8, *c=(void*)12, *d=(void*)16;

    VoidList list;

    // test simple modifiers and info
    list.append(c);     PRINT(list);   // c
    list.prepend(b);   	PRINT(list);   // b c
    list.append(d);	PRINT(list);   // b c d
    list.prepend(a);	PRINT(list);   // a b c d
    list.removeAt(2);	PRINT(list);   // a b d

    xassert( list.count() == 3 &&
             !list.isEmpty() &&
             list.nth(0) == a &&
             list.nth(1) == b &&
             list.nth(2) == d &&
             list.indexOf(a) == 0 &&
             list.indexOf(b) == 1 &&
             list.indexOf(c) == -1 &&
             list.indexOf(d) == 2
           );
    list.selfCheck();

    // test mutator s
    {
      VoidListMutator mut(list);
      mut.adv();
	// now it's pointing at b
      mut.insertAfter(c);
	// now list is (a b c d) and mut points at b still
      verifySorted(list);
      mut.remove();
	// now list is (a c d) and mut points at c
      xassert(mut.data() == c);

      // copy the mutator
      VoidListMutator mut2(mut);
      mut2.adv();
      xassert(mut.data() == c  &&  mut2.data() == d);

      // copy to a normal iterator
      VoidListIter iter(mut);
      iter.adv();
      xassert(iter.data() == d);
      iter.adv();
      xassert(iter.isDone()  &&  mut.data() == c);

      PRINT(list);
    }

    // test appendUnique and prependUnique
    // list starts as (a c d)
    xassert(list.appendUnique(c) == false &&
            list.prependUnique(d) == false &&
            list.prependUnique(b) == true);
      // now it is (b a c d)
    list.removeItem(a);
    xassert(list.removeIfPresent(a) == false);
      // now it is (b c d)
    verifySorted(list);
    PRINT(list);

    // test reverse
    list.reverse();
      // list is now (d c b)
    xassert(list.indexOf(d) == 0 &&
            list.indexOf(c) == 1 &&
            list.indexOf(b) == 2);
    PRINT(list);

    // test stealTailAt
    VoidList thief;
    thief.stealTailAt(1, list);
      // list is now (d)
      // thief is now (c b)
    xassert(list.count() == 1 &&
            list.indexOf(d) == 0 &&
            thief.count() == 2 &&
            thief.indexOf(c) == 0 &&
            thief.indexOf(b) == 1);

    // test appendAll
    list.appendAll(thief);      // list: (d c b)
    PRINT(list);
    xassert(list.count() == 3 &&
            list.indexOf(d) == 0 &&
            list.indexOf(c) == 1 &&
            list.indexOf(b) == 2);

    // test prependAll
    list.prependAll(thief);     // list: (c b d c b)
    PRINT(list);
    xassert(list.count() == 5 &&
            list.nth(0) == c &&
            list.nth(1) == b &&
            list.nth(2) == d &&
            list.nth(3) == c &&
            list.nth(4) == b);

    xassert(thief.count() == 2);    // not modified

    // test removeDuplicatesAsMultiset
    list.removeDuplicatesAsPointerMultiset();     // list: (b c d)
    PRINT(list);
    xassert(list.count() == 3 &&
            list.nth(0) == b &&
            list.nth(1) == c &&
            list.nth(2) == d);
  }

  // this hits most of the remaining code
  // (a decent code coverage tool for C++ would be nice!)
  testSorting();

  printf("voidlist ok\n");
}


// EOF
