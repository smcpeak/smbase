// test-refct-serf.cc
// Test code for 'test-refct' module.

#include "refct-serf.h"                // module to test

#include "array.h"                     // ArrayStack
#include "macros.h"                    // Restorer
#include "owner.h"                     // Owner
#include "sm-iostream.h"               // cout, etc.
#include "test.h"                      // USUAL_TEST_MAIN


// Placeholder data class.  This one does not explicitly call any of
// the SerfRefCount methods.
class Integer : public SerfRefCount {
public:      // data
  int m_i;

public:      // funcs
  Integer(int i) : m_i(i) {}
  Integer(Integer const &obj) : m_i(obj.m_i) {}

  Integer& operator= (Integer const &obj)
  {
    m_i = obj.m_i;
    return *this;
  }

  bool operator== (Integer const &obj)
  {
    return m_i == obj.m_i;
  }
};


// Another placeholder data class, explicitly calling SerfRefCount.
class Float : public SerfRefCount {
public:      // data
  // 'double' rather than 'float' just because that is what the literals
  // are, avoiding some overloading issues.
  double m_f;

public:      // funcs
  Float(float f) :
    SerfRefCount(),
    m_f(f)
  {}

  Float(Float const &obj)
    : SerfRefCount(obj),
      m_f(obj.m_f)
  {}

  Float& operator= (Float const &obj)
  {
    SerfRefCount::operator=(obj);
    m_f = obj.m_f;
    return *this;
  }

  bool operator== (Float const &obj)
  {
    return SerfRefCount::operator==(obj) &&
           m_f == obj.m_f;
  }
};


// Number of times we have "aborted".  This gets cleared during each
// test's setup phase to ensure independence of tests.
static int failCount = 0;

// Set of outstanding serf pointers that need to be cleared when we
// detect a failure.
static ArrayStack<RCSerfBase*> failingSerfs;

// Called when an expected failure happens.  It has to repair the
// condition causing the failure so we don't actually abort.
static void incFailCount()
{
  failCount++;
  while (failingSerfs.isNotEmpty()) {
    RCSerfBase *s = failingSerfs.pop();

    // Nullify the RCSerf so it decrements the refct and releases the
    // object.
    s->operator=(NULL);
  }
}

// Prepare for a failure to be reported.
#define PREPARE_TO_FAIL()                                       \
  failCount = 0;                                                \
  Restorer< void (*)() > abortRestorer(                         \
    SerfRefCount::s_preAbortFunction, &incFailCount) /* user ; */

// Add an RCSerf to the set of those that we know are about to dangle
// due to an intentional failure.
#define PUSH_FAIL_SERF(serf) \
  failingSerfs.push(&( (serf).unsafe_getRCSerfBase() )) /* user ; */


// Exercise the operators.
static void testOperatorsInteger()
{
  Owner<Integer> o1(new Integer(3));
  RCSerf<Integer> s1(o1);
  EXPECT_EQ(o1->m_i, 3);
  EXPECT_EQ(o1->getRefCount(), 1);

  // operator T*
  Integer *p1 = s1;
  EXPECT_EQ(p1->m_i, 3);

  // Use operator T* as part of conversion to bool.
  EXPECT_EQ(!!s1, true);

  // operator->
  EXPECT_EQ(s1->m_i, 3);

  // operator*
  EXPECT_EQ((*s1).m_i, 3);

  // ptr() method
  EXPECT_EQ(s1.ptr()->m_i, 3);

  // Copy constructor.
  RCSerf<Integer> s2(s1);
  EXPECT_EQ(o1->getRefCount(), 2);
  EXPECT_EQ(s2->m_i, 3);

  // Copy assignment operator.
  RCSerf<Integer> s3;
  s3 = s1;
  EXPECT_EQ(o1->getRefCount(), 3);
  EXPECT_EQ(s3->m_i, 3);

  // Let it all clean up automatically.
}

// Same thing but using Float.
static void testOperatorsFloat()
{
  Owner<Float> o1(new Float(3.75));
  RCSerf<Float> s1(o1);
  EXPECT_EQ(o1->m_f, 3.75);

  // operator T*
  Float *p1 = s1;
  EXPECT_EQ(p1->m_f, 3.75);

  // Use operator T* as part of conversion to bool.
  EXPECT_EQ(!!s1, true);

  // operator->
  EXPECT_EQ(s1->m_f, 3.75);

  // operator*
  EXPECT_EQ((*s1).m_f, 3.75);

  // ptr() method
  EXPECT_EQ(s1.ptr()->m_f, 3.75);

  // Copy constructor.
  RCSerf<Float> s2(s1);
  EXPECT_EQ(o1->getRefCount(), 2);
  EXPECT_EQ(s2->m_f, 3.75);

  // Copy assignment operator.
  RCSerf<Float> s3;
  s3 = s1;
  EXPECT_EQ(o1->getRefCount(), 3);
  EXPECT_EQ(s3->m_f, 3.75);

  // Let it all clean up automatically.
}


// Test RCSerf referring to Owner.
static void testOwnerPointerSuccess()
{
  Owner<Integer> i(new Integer(9));
  RCSerf<Integer> s;
  s = i;
  EXPECT_EQ(s->m_i, 9);
}

static void testOwnerPointerFailure()
{
  {
    RCSerf<Integer> s;
    PREPARE_TO_FAIL();
    PUSH_FAIL_SERF(s);

    Owner<Integer> i(new Integer(9));
    s = i;
    EXPECT_EQ(s->m_i, 9);

    // Let both go out of scope, causing a failure since 'i' is
    // destroyed first.
  }

  xassert(failCount == 1);
}


// Test RCSerf pointing at a local.
static void testLocalObjSuccess()
{
  Integer i(5);
  RCSerf<Integer> s(&i);
  EXPECT_EQ(s->m_i, 5);
}

static void testLocalObjFailure()
{
  {
    RCSerf<Integer> s;
    PREPARE_TO_FAIL();
    PUSH_FAIL_SERF(s);

    Integer i(9);
    s = &i;
    EXPECT_EQ(s->m_i, 9);

    // Let both go out of scope, causing a failure since 'i' is
    // destroyed first.
  }

  xassert(failCount == 1);
}


static void deallocate(Integer *i)
{
  delete i;
}

// Test RCSerf pointing at something allocate with 'new' and
// deallocated with 'delete' in a callee.
static void testPlainPointerSuccess()
{
  Integer *i = new Integer(12);
  {
    RCSerf<Integer> s(i);
    EXPECT_EQ(s->m_i, 12);
  }
  deallocate(i);
}

static void testPlainPointerFailure()
{
  {
    Integer *i = new Integer(12);
    RCSerf<Integer> s(i);
    EXPECT_EQ(s->m_i, 12);
    PREPARE_TO_FAIL();
    PUSH_FAIL_SERF(s);
    deallocate(i);
  }

  xassert(failCount == 1);
}


// Test nullifying a serf.
static void testNullify()
{
  Integer i(7);
  RCSerf<Integer> s1(&i);
  EXPECT_EQ(!!s1, true);
  EXPECT_EQ(i.getRefCount(), 1);

  s1 = NULL;
  EXPECT_EQ(!!s1, false);
  EXPECT_EQ(i.getRefCount(), 0);
}


static void paramCallee(RCSerf<Integer> s)
{
  EXPECT_EQ(s->m_i, 8);
}

// Test passing RCSerf as a parameter.
static void testParam()
{
  Integer i(8);
  paramCallee(&i);

  RCSerf<Integer> s(&i);
  paramCallee(s);
}


// Test storing RCSerfs in a container.
static void testManyPointersSuccess()
{
  Integer obj(14);
  ArrayStack<RCSerf<Integer> > arr;
  for (int i=0; i < 10; i++) {
    arr.push(RCSerf<Integer>(&obj));
  }
}

static void testManyPointersFailure()
{
  Integer *obj = new Integer(14);
  ArrayStack<RCSerf<Integer> > arr;
  for (int i=0; i < 10; i++) {
    arr.push(RCSerf<Integer>(obj));
  }

  // Push the fail serfs now, after all have been allocated in the
  // stack, since the array is done resizing.
  for (int i=0; i < 10; i++) {
    PUSH_FAIL_SERF(arr[i]);
  }

  PREPARE_TO_FAIL();
  delete obj;
  xassert(failCount == 1);
}


// Exercise both 'swapWith' method and 'swap' global function.
static void testSwapWithSuccess()
{
  Integer *o1 = new Integer(16);
  Integer *o2 = new Integer(17);

  {
    RCSerf<Integer> s1(o1);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(s1->m_i, 16);

    RCSerf<Integer> s2(o2);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(s2->m_i, 17);

    s1.swapWith(s2);
    EXPECT_EQ(s1->m_i, 17);
    EXPECT_EQ(s2->m_i, 16);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(o2->getRefCount(), 1);

    RCSerf<Integer> s3;
    swap(s3, s1);
    EXPECT_EQ(!!s1, false);
    EXPECT_EQ(s3->m_i, 17);
    EXPECT_EQ(s2->m_i, 16);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(o2->getRefCount(), 1);
  }

  delete o2;
  delete o1;
}

static void testSwapWithFailure()
{
  Integer *o1 = new Integer(16);
  Integer *o2 = new Integer(17);

  {
    RCSerf<Integer> s1(o1);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(s1->m_i, 16);

    RCSerf<Integer> s2(o2);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(s2->m_i, 17);

    s1.swapWith(s2);
    EXPECT_EQ(s1->m_i, 17);
    EXPECT_EQ(s2->m_i, 16);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(o2->getRefCount(), 1);

    RCSerf<Integer> s3;
    swap(s3, s1);
    EXPECT_EQ(!!s1, false);
    EXPECT_EQ(s3->m_i, 17);
    EXPECT_EQ(s2->m_i, 16);
    EXPECT_EQ(o1->getRefCount(), 1);
    EXPECT_EQ(o2->getRefCount(), 1);

    PREPARE_TO_FAIL();
    PUSH_FAIL_SERF(s3);
    delete o2;
  }

  delete o1;
}


static void testRelease()
{
  RCSerf<Integer> i = new Integer(18);
  EXPECT_EQ(!!i, true);
  delete i.release();
  EXPECT_EQ(!!i, false);
}


static void entry()
{
  testOperatorsInteger();
  testOperatorsFloat();
  testOwnerPointerSuccess();
  testOwnerPointerFailure();
  testLocalObjSuccess();
  testLocalObjFailure();
  testPlainPointerSuccess();
  testPlainPointerFailure();
  testNullify();
  testParam();
  testManyPointersSuccess();
  testManyPointersFailure();
  testSwapWithSuccess();
  testSwapWithFailure();
  testRelease();

  cout << "test-refct-serf ok" << endl;
}

USUAL_TEST_MAIN


// EOF
