// sm-rc-ptr-test.cc
// Tests for sm-rc-ptr and sm-rc-obj.

#include "sm-rc-ptr.h"                 // module under test

#include "sm-test.h"                   // DIAG
#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout, etc.


OPEN_ANONYMOUS_NAMESPACE


bool verbose = false;


class Foo : public RefCountObject {
public:      // data
  int x;

public:      // methods
  Foo()
    : x(0)
  {
    DIAG("  called Foo::Foo(): " << (void*)this);
  }

  Foo(int xx)
    : x(xx)
  {
    DIAG("  called Foo::Foo(int=" << xx << "): " << (void*)this);
  }

  Foo(Foo const &obj)
    : RefCountObject(),
      x(obj.x)
  {
    DIAG("  called Foo::Foo(Foo const &): " << (void*)this);
  }

  ~Foo()
  {
    DIAG("  called Foo::~Foo(): " << (void*)this);
  }
};


// Like a header for a section.
#define FUNCTION_HEADER() DIAG(__func__ << ":")


// One entry in a section.
#define IN_FUNCTION() DIAG("  in " << __func__)


static void testAssignNew()
{
  FUNCTION_HEADER();

  RCPtr<Foo> p;

  // Verify NULL initialization.
  xassert(p.get() == NULL);

  // Implicit conversion to 'bool' is possible via the pointer type.
  xassert(!p);
  if (p) {
    xfailure("no");
  }

  // Assign to a normal pointer.
  p = new Foo;
  if (p) {
  }
  else {
    xfailure("no");
  }

  // Test use of overloaded operators.
  xassert(p->x == 0);
  p->x = 3;

  Foo *fp = p;
  xassert(fp->x == 3);

  xassert( (*p).x == 3 );
  (*p).x = 5;
  xassert(p->x == 5);

  // The object will be deallocated automatically.
}


static void testConstructNew()
{
  FUNCTION_HEADER();

  RCPtr<Foo> p(new Foo);
}


static void testObjectCopyCtor()
{
  FUNCTION_HEADER();

  RCPtr<Foo> p1(new Foo);
  RCPtr<Foo> p2(new Foo(*p1));

  // Comparison is possible via conversion to pointer type.
  xassert(p1 != p2);
}


static void testPtrCopyCtor()
{
  FUNCTION_HEADER();

  RCPtr<Foo> p1(new Foo);
  RCPtr<Foo> p2(p1);

  xassert(p1 == p2);
  xassert(p1->getRefCount() == 2);
}


class HasRCPtr {
public:      // data
  RCPtr<Foo> m_ptr;

public:      // methods
  HasRCPtr()
    : m_ptr(new Foo)
  {
    DIAG("  called HasRCPtr::HasRCPtr()");
  }

  HasRCPtr(HasRCPtr const &obj)
    : m_ptr(obj.m_ptr)
  {
    DIAG("  called HasRCPtr::HasRCPtr(HasRCPtr const &)");
  }

  HasRCPtr(HasRCPtr &&obj)
    : m_ptr(obj.m_ptr)
  {
    DIAG("  called HasRCPtr::HasRCPtr(HasRCPtr &&)");
  }

  ~HasRCPtr()
  {
    DIAG("  called HasRCPtr::~HasRCPtr()");
  }
};


static HasRCPtr getHasRCPtr()
{
  IN_FUNCTION();
  return HasRCPtr();
}

static HasRCPtr takesHasRCPtr(HasRCPtr h)
{
  IN_FUNCTION();
  return h;
}

static void testHasRCPtr()
{
  FUNCTION_HEADER();

  // This activates the move constructor for HasRCPtr.
  HasRCPtr h = takesHasRCPtr(getHasRCPtr());
}


static RCPtr<Foo> getPtr()
{
  IN_FUNCTION();
  return rcptr(new Foo(7));
}

static RCPtr<Foo> passThrough(RCPtr<Foo> p)
{
  IN_FUNCTION();
  return p;
}

static void consumer(RCPtr<Foo> p)
{
  IN_FUNCTION();
  xassert(p->x == 7);
}

static void testPassingAndReturning()
{
  FUNCTION_HEADER();

  consumer(passThrough(getPtr()));
}


static void testRelease()
{
  FUNCTION_HEADER();

  RCPtr<Foo> p(new Foo);
  Foo *q = p.release();

  xassert(p == NULL);
  xassert(q->getRefCount() == 1);

  delete q;
}


static void testSwap()
{
  FUNCTION_HEADER();

  RCPtr<Foo> a(new Foo(1));
  RCPtr<Foo> b(new Foo(2));

  xassert(a->x == 1 && b->x == 2);

  a.swap(b);

  xassert(a->x == 2 && b->x == 1);
}


static void testIncDecFunctions()
{
  FUNCTION_HEADER();

  Foo *p = incRefCount(new Foo(9));
  xassert(p->x == 9);
  decRefCount(p);

  p = incRefCount((Foo*)NULL);
  xassert(p == NULL);
  decRefCount(p);
}


static void testDecOnLeave()
{
  FUNCTION_HEADER();

  Foo *p = incRefCount(new Foo(9));
  DEC_REF_COUNT_ON_LEAVING_SCOPE(p);
}


class Derived : public Foo {};

static void takesFoo(RCPtr<Foo> f)
{
  xassert(f->x == 88);

  // Not allowed, and static assertion confirms that.
  //RCPtr<Derived> no(f);
}

static void testImplicitUpcast()
{
  FUNCTION_HEADER();

  RCPtr<Derived> d(new Derived);
  d->x = 88;
  takesFoo(d);      // allowed

  RCPtr<Foo> f;
  f = d;
  xassert(f->x == 88);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_rc_ptr()
{
  testAssignNew();
  testConstructNew();
  testObjectCopyCtor();
  testPtrCopyCtor();
  testHasRCPtr();
  testPassingAndReturning();
  testRelease();
  testSwap();
  testIncDecFunctions();
  testDecOnLeave();
  testImplicitUpcast();
}


// EOF
