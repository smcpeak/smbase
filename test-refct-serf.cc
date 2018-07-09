// test-refct-serf.cc
// Test code for 'test-refct' module.

#include "refct-serf.h"                // module to test

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
  float m_f;

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


static void testOperators()
{
  Owner<Integer> o1(new Integer(3));
  RCSerf<Integer> s1(o1);
  EXPECT_EQ(o1->m_i, 3);

  // operator Integer*
  Integer *p1 = s1;
  EXPECT_EQ(p1->m_i, 3);

  // Use operator Integer* as part of conversion to bool.
  EXPECT_EQ(!!s1, true);

  // operator->
  EXPECT_EQ(s1->m_i, 3);

  // operator*
  EXPECT_EQ((*s1).m_i, 3);

  // ptr() method
  EXPECT_EQ(s1.ptr()->m_i, 3);

  // Let it all clean up automatically.
}


static void testLocalObj()
{
  Integer i(5);
  RCSerf<Integer> s(&i);
  EXPECT_EQ(s->m_i, 5);
}


static void testNull()
{
  Integer i(7);
  RCSerf<Integer> s1(&i);
  EXPECT_EQ(!!s1, true);

  s1 = NULL;
  EXPECT_EQ(!!s1, false);
}


static void paramCallee(RCSerf<Integer> s)
{
  EXPECT_EQ(s->m_i, 8);
}

static void testParam()
{
  Integer i(8);
  paramCallee(&i);

  RCSerf<Integer> s(&i);
  paramCallee(s);
}


// Number of times we have "aborted".
static int failCount = 0;

// Address of the outstanding serf that causes the failure.  We need to
// nullify it in the abort function before letting the object actually
// be deallocated.
static RCSerfBase *failingSerf = NULL;

// Called when an expected failure happens.  It has to repair the
// condition causing the failure so we don't actually abort.
static void incFailCount()
{
  failCount++;
  if (failingSerf) {
    // Nullify the RCSerf so it decrements the refct and releases the
    // object.
    failingSerf->operator=(NULL);

    // Don't hang onto the serf since it will be deallocated soon too.
    failingSerf = NULL;
  }
}

// Prepare for a failure caused by 'serf'.
#define PREPARE_TO_FAIL(serf)                                   \
  failCount = 0;                                                \
  Restorer< RCSerfBase* > serfRestorer(                         \
    failingSerf, &( (serf).unsafe_getRCSerfBase() ));           \
  Restorer< void (*)() > abortRestorer(                         \
    SerfRefCount::s_preAbortFunction, &incFailCount) /* user ; */


static void testDangleLocal()
{
  {
    RCSerf<Integer> s;
    PREPARE_TO_FAIL(s);

    Integer i(9);
    s = &i;
    EXPECT_EQ(s->m_i, 9);

    // Let both go out of scope, causing a failure since 'i' is
    // destroyed first.
  }

  xassert(failCount == 1);
}


static void testDangleHeap()
{
  {
    RCSerf<Integer> s;
    PREPARE_TO_FAIL(s);

    Owner<Integer> i(new Integer(9));
    s = i;
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

static void testDangleDeallocate()
{
  {
    Integer *i = new Integer(12);
    RCSerf<Integer> s(i);
    PREPARE_TO_FAIL(s);
    deallocate(i);
  }

  xassert(failCount == 1);
}


static void entry()
{
  testOperators();
  testLocalObj();
  testNull();
  testParam();
  testDangleLocal();
  testDangleHeap();
  testDangleDeallocate();

  cout << "test-refct-serf ok" << endl;
}

USUAL_TEST_MAIN


// EOF
