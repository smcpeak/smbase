// vptrmap-test.cc
// Tests for vptrmap.

#include "vptrmap.h"                   // module under test

#include "array.h"                     // ObjArrayStack
#include "ptrmap.h"                    // PtrMap
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE

#include <stdlib.h>                    // rand, qsort
#include <stdio.h>                     // printf


OPEN_ANONYMOUS_NAMESPACE


class Node {
public:
  int *value;
  bool found;

public:
  Node() {
    value = new int(0);
    found = false;
  }
  ~Node() {
    delete value;
  }
};


int doubleCompar(void const *dp1, void const *dp2)
{
  double d1 = *((double*)dp1);
  double d2 = *((double*)dp2);
  if (d1 < d2) return -1;
  if (d1 > d2) return +1;
  return 0;    // almost never happens
}


void test1()
{
  printf("test1: testing PtrMap\n");

  enum { ITERS1=10, ITERS2MAX=200 };

  double avgprobes[ITERS1];

  printf("  iter  iters  entries  lookups  probes  avgprobes\n");
  printf("  ----  -----  -------  -------  ------  ---------\n");

  for (int i=0; i < ITERS1; i++) {
    // I actually test PtrMap, the type-safe wrapper on top
    // of VoidPtrMap, so I can test that class too; the casts
    // that I used to need for VoidPtrMap are now protected by
    // this CAST macro
    //#define CAST(something) (something)
    #define CAST(something) /*nothing*/

    PtrMap<Node,int> map;
    ObjArrayStack<Node> stack;

    int iters2 = rand() % ITERS2MAX;
    for (int j=0; j < iters2; j++) {
      int op = rand() % 100;

      if (op <= 40) {         // insert
        Node *n = new Node;
        stack.push(n);
        map.add(n, n->value);
      }

      else if (op <= 80) {    // find exist
        if (stack.isNotEmpty()) {
          Node *n = stack[rand() % stack.length()];
          int *v = CAST(int*)map.get(n);
          xassert(v && v == n->value);

          if (rand() % 10 == 0) {
            // reassign
            delete n->value;
            n->value = new int(0);
            map.add(n, n->value);
          }
        }
      }

      else if (op <= 90) {    // find non-exist
        Node *n = new Node;
        int *v = CAST(int*)map.get(n);
        xassert(!v);
        delete n;
      }

      else if (op <= 100) {   // traverse
        // clear all 'found'
        int k;
        for (k=0; k < stack.length(); k++) {
          stack[k]->found = false;
        }

        // walk via map; should find each one exactly once
        int numFound = 0;
        //VoidPtrMap::Iter iter(map);
        PtrMap<Node,int>::Iter iter(map);
        for (; !iter.isDone(); iter.adv()) {
          Node *n = CAST(Node*)iter.key();
          int *v = CAST(int*)iter.value();

          xassert(v == n->value);
          xassert(n->found == false);
          n->found = true;
          numFound++;
        }

        // check all 'found' (launch all 'zig')
        for (k=0; k < stack.length(); k++) {
          xassert(stack[k]->found == true);
        }
        xassert(numFound == stack.length());
      }
    }

    xassert(map.getNumEntries() == stack.length());
    //     "  iter  iters  entries  lookups  probes  avgprobes"
    avgprobes[i] = ((double)VoidPtrMap::probes) / ((double)VoidPtrMap::lookups);
    printf("  %4d  %5d  %7d  %7d  %6d    %g\n",
           i,
           iters2,
           map.getNumEntries(),
           VoidPtrMap::lookups,
           VoidPtrMap::probes,
           avgprobes[i]);

    VoidPtrMap::probes = 0;
    VoidPtrMap::lookups = 0;
  }

  // compute median of avgprobes
  qsort(avgprobes, ITERS1, sizeof(avgprobes[0]), doubleCompar);
  printf("median avgprobe: %g\n", avgprobes[ITERS1/2]);
}


struct A {
  int x;
  A(int x0) : x(x0) {}
};

void test2()
{
  printf("test2: testing PtrSet\n");

  PtrSet<A> s;
  xassert(s.isEmpty());
  xassert(s.getNumEntries() == 0);

  A *a1 = new A(1);
  s.add(a1);
  xassert(s.isNotEmpty());
  xassert(s.getNumEntries() == 1);

  A *a2 = new A(2);
  s.add(a2);
  xassert(s.isNotEmpty());
  xassert(s.getNumEntries() == 2);

  xassert(s.contains(a1));
  xassert(s.contains(a2));

  s.empty();                    // make empty

  xassert(!s.contains(a1));
  xassert(!s.contains(a2));
  xassert(s.isEmpty());
  xassert(s.getNumEntries() == 0);

  A *a3 = new A(3);
  s.add(a3);
  xassert(s.isNotEmpty());
  xassert(s.getNumEntries() == 1);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_vptrmap()
{
  printf("testing vptrmap\n");
  test1();
  test2();
  printf("vptrmap is ok\n");
}


// EOF
