// sm-unique-ptr-test.cc
// Tests for `sm-unique-ptr` module.

#include "sm-unique-ptr-fwd.h"         // forward decl under test

static smbase::UniquePtr<int> *g_UniquePtr_ptr;

#include "sm-unique-ptr-iface.h"       // data structure under test

static smbase::UniquePtr<int> g_UniquePtr;

#include "sm-unique-ptr.h"             // operations under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ
#include "xassert.h"                   // xassert

#include <utility>                     // std::{move, swap}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Test the globals created above when only using reduced interfaces.
void testGlobals()
{
  g_UniquePtr_ptr = new UniquePtr<int>(new int(1));
  g_UniquePtr = UniquePtr<int>(new int(2));

  xassert(**g_UniquePtr_ptr == 1);
  xassert(*g_UniquePtr == 2);

  g_UniquePtr_ptr->reset();
  delete g_UniquePtr_ptr;
  g_UniquePtr_ptr = nullptr;

  g_UniquePtr.reset();
}


struct Super {
  static int s_count;

  int one = 1;

  Super(int n = 1)
    : one(n)
  {
    ++s_count;
  }

  ~Super()
  {
    --s_count;
  }
};

int Super::s_count = 0;

struct Sub : Super {};


void testCtors()
{
  UniquePtr<int> p1;
  xassert(p1.get() == nullptr);

  UniquePtr<int> p2(nullptr);
  xassert(p2.get() == nullptr);

  UniquePtr<int> p3((int*)nullptr);
  xassert(p3.get() == nullptr);

  UniquePtr<int> p4(new int(4));
  xassert(*p4 == 4);

  UniquePtr<int> p5(std::move(p4));
  xassert(!p4.get());
  xassert(*p5 == 4);

  UniquePtr<Sub> p6(new Sub);
  xassert(p6.get());

  UniquePtr<Super> p7(std::move(p6));
  xassert(!p6.get());
  xassert(p7.get());
}


void testAssign()
{
  UniquePtr<int> p1(new int(1));
  UniquePtr<int> p2;
  p2 = std::move(p1);

  xassert(!p1.get());
  xassert(*p2 == 1);

  UniquePtr<Sub> p3(new Sub);
  UniquePtr<Super> p4;
  p4 = std::move(p3);

  xassert(!p3.get());
  xassert(p4.get());

  p4 = nullptr;
  xassert(!p4.get());
}


void testAccess()
{
  UniquePtr<Super> p1(new Super);
  xassert(p1->one == 1);

  xassert((bool)p1);
  xassert(bool(p1));
  xassert(static_cast<bool>(p1));
  xassert(!!p1);

  p1.reset();

  xassert(!p1);

  bool ok = true;
  try {
    *p1;
    ok = false; // Should have failed.
  }
  catch (XAssert &) {}
  xassert(ok);
}


void testRelease()
{
  UniquePtr<Super> p1(new Super);
  Super *sp = p1.release();
  xassert(!p1);
  xassert(sp);
  delete sp;

  xassert(p1.release() == nullptr);
}


void testReset()
{
  UniquePtr<Super> p1(new Super);
  p1.reset();
  xassert(!p1);

  p1.reset(new Super);
  xassert(!!p1);

  p1.reset(nullptr);
  xassert(!p1);
}


void testSwap()
{
  UniquePtr<Super> p1(new Super(1));
  UniquePtr<Super> p2(new Super(2));

  xassert(p1->one == 1);
  xassert(p2->one == 2);

  p1.swap(p2);

  xassert(p1->one == 2);
  xassert(p2->one == 1);

  std::swap(p1, p2);

  xassert(p1->one == 1);
  xassert(p2->one == 2);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_unique_ptr()
{
  testGlobals();
  testCtors();
  testAssign();
  testAccess();
  testRelease();
  testReset();
  testSwap();

  xassert(Super::s_count == 0);
}


// EOF
