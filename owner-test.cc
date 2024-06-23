// owner-test.cc            see license.txt for copyright and terms of use
// test owner stuff

#include "owner.h"                     // module to test

#include "sm-macros.h"                 // PRETEND_USED, OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // dummy_printf
#include "xassert.h"                   // xassert

#include <stdio.h>                     // printf
#include <stdlib.h>                    // exit


// Silence test.
#define printf dummy_printf


OPEN_ANONYMOUS_NAMESPACE


// a simple class to play with
class Foo {
public:
  static int count;    // # of Foos there are
  int x;

public:
  Foo(int a);
  ~Foo();
};

int Foo::count = 0;

Foo::Foo(int ax)
  : x(ax)
{
  printf("created Foo at %p\n", this);
  count++;
}

Foo::~Foo()
{
  printf("destroying Foo at %p\n", this);
  count--;
}


void printFoo(Foo *f)
{
  printf("Foo at %p, x=%d\n", f, f? f->x : 0);
}

void printFooC(Foo const *f)
{
  printf("const Foo at %p, x=%d\n", f, f? f->x : 0);
}

void printInt(int x)
{
  printf("int x is %d\n", x);
}


// make it, forget to free it
void test1()
{
  printf("----------- test1 -----------\n");
  Owner<Foo> f;

  // Exercise the conversion to bool.
  xassert(!f);
  if (f) {
    xfailure("should be false");
  }
  bool b = f;
  xassert(!b);

  f = new Foo(4);

  xassert(f);
  if (!f) {
    xfailure("should be true");
  }
  b = f;
  xassert(b);

  // Also make sure it works with a 'const' Owner.
  Owner<Foo> const &cf = f;
  xassert(cf);
  if (!cf) {
    xfailure("should be true");
  }
  b = cf;
  xassert(b);
}

// access all of the operators as non-const
void test2()
{
  printf("----------- test2 -----------\n");
  Owner<Foo> f(new Foo(6));

  printFoo(f);
  (*f).x = 9;
  f->x = 12;
}

// access all of the operators as const
void test3()
{
  printf("----------- test3 -----------\n");
  Owner<Foo> f(new Foo(8));
  Owner<Foo> const &g = f;

  printFooC(g);
  printInt((*g).x);      // egcs-1.1.2 allows this for non-const operator fn!!!
  printInt(g->x);
}

// test exchange of ownership
void test4()
{
  printf("----------- test4 -----------\n");
  //Owner<Foo> f = new Foo(3);     // egcs-1.1.2 does the wrong thing here
  Owner<Foo> f(new Foo(3));
  Owner<Foo> g;
  g = f;
  printFoo(f);    // should be null
  f = g.xfr();
  printFoo(g);    // should be null
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_owner()
{
  test1();
  test2();
  test3();
  test4();

  printf("%d Foos leaked\n", Foo::count);
  if (Foo::count) {
    exit(2);
  }
}


// EOF
