// sm-rc-ptr-test.cc
// Tests for sm-rc-ptr and sm-rc-obj.

#include "sm-rc-ptr.h"                 // module under test

#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout, etc.


class Foo : public RefCountObject {
public:      // data
  int x;

public:      // methods
  Foo()
    : x(0)
  {
    std::cout << "  called Foo::Foo(): " << (void*)this << "\n";
  }

  Foo(int xx)
    : x(xx)
  {
    std::cout << "  called Foo::Foo(int=" << xx << "): " << (void*)this << "\n";
  }

  Foo(Foo const &obj)
    : x(obj.x)
  {
    std::cout << "  called Foo::Foo(Foo const &): " << (void*)this << "\n";
  }

  ~Foo()
  {
    std::cout << "  called Foo::~Foo(): " << (void*)this << "\n";
  }
};


// Like a header for a section.
#define FUNCTION_HEADER() std::cout << __func__ << ":\n" /* user ; */


// One entry in a section.
#define IN_FUNCTION() std::cout << "  in " << __func__ << "\n" /* user ; */


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
    std::cout << "  called HasRCPtr::HasRCPtr()\n";
  }

  HasRCPtr(HasRCPtr const &obj)
    : m_ptr(obj.m_ptr)
  {
    std::cout << "  called HasRCPtr::HasRCPtr(HasRCPtr const &)\n";
  }

  HasRCPtr(HasRCPtr &&obj)
    : m_ptr(obj.m_ptr)
  {
    std::cout << "  called HasRCPtr::HasRCPtr(HasRCPtr &&)\n";
  }

  ~HasRCPtr()
  {
    std::cout << "  called HasRCPtr::~HasRCPtr()\n";
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
  return RCPtr<Foo>(new Foo(7));
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


void test_sm_rc_ptr()
{
  testAssignNew();
  testConstructNew();
  testObjectCopyCtor();
  testPtrCopyCtor();
  testHasRCPtr();
  testPassingAndReturning();
  testRelease();
}


// EOF
