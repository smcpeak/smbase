// refct-serf-test.cc
// Test code for 'refct-serf' and 'rcserflist' modules.

#include "rcserflist.h"                // module to test
#include "refct-serf.h"                // module to test

#include "array.h"                     // ArrayStack
#include "save-restore.h"              // SET_RESTORE
#include "objlist.h"                   // ObjList
#include "owner.h"                     // Owner
#include "sm-iostream.h"               // cout, etc.
#include "sm-test.h"                   // EXPECT_EQ


namespace {


// Placeholder data class.  This one does not explicitly call any of
// the SerfRefCount methods.
class Integer : public SerfRefCount {
public:      // data
  int m_i;

public:      // funcs
  Integer(int i) : m_i(i) {}
  Integer(Integer const &obj) : SerfRefCount(), m_i(obj.m_i) {}

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


// Build a multiple-inheritance hierarchy.  This replicates a scenario
// where I have two "observer" interfaces, each of which inherits
// SerfRefCount, and then I have another class that implements both of
// the observer interfaces.  We then check that RCSerf is happy and can
// access everything.
class Super1 : virtual public SerfRefCount {
public:
  int x;
};

class Super2 : virtual public SerfRefCount {
public:
  int y;
  virtual ~Super2() {}       // Give this one a vtable.
};

class Sub : public Super1, public Super2 {
public:
  int z;
};


// Number of times we have "aborted".  This gets cleared during each
// test's setup phase to ensure independence of tests.
static int failCount = 0;

// Set of outstanding serf pointer objects that need to be cleared when
// we detect a failure.
static ArrayStack<RCSerf<Integer>*> failingIntegerSerfs;

// Similar for other RCSerf types.  (It is not possible, or at least
// not easy, to make a single structure that works for all of them
// because the location of the SerfRefCount within the containing type
// depends on that type.)
static ArrayStack<RCSerf<Float>*> failingFloatSerfs;
static ArrayStack<RCSerf<Integer const>*> failingIntegerConstSerfs;
static ArrayStack<RCSerf<Super1>*> failingSuper1Serfs;
static ArrayStack<RCSerf<Super2>*> failingSuper2Serfs;
static ArrayStack<RCSerf<Sub>*> failingSubSerfs;


template <class T>
void emptyFailingSerfs(ArrayStack<RCSerf<T>*> &failingSerfs)
{
  while (failingSerfs.isNotEmpty()) {
    RCSerf<T> *s = failingSerfs.pop();

    // Nullify the pointer, decrementing the refct.
    *s = NULL;
  }
}


// Called when an expected failure happens.  It has to repair the
// condition causing the failure so we don't actually abort.
static void incFailCount()
{
  failCount++;

  emptyFailingSerfs(failingIntegerSerfs);
  emptyFailingSerfs(failingIntegerConstSerfs);
  emptyFailingSerfs(failingFloatSerfs);
  emptyFailingSerfs(failingSuper1Serfs);
  emptyFailingSerfs(failingSuper2Serfs);
  emptyFailingSerfs(failingSubSerfs);
}


// Prepare for a failure to be reported.
#define PREPARE_TO_FAIL()                       \
  failCount = 0;                                \
  SET_RESTORE(SerfRefCount::s_preAbortFunction, \
    &incFailCount) /* user ; */

// Add an RCSerf to the set of those that we know are about to dangle
// due to an intentional failure.
//
// This macro only works for 'failingIntegerSerfs', which is the one I
// use in most places.  The others have to be pushed directly.
#define PUSH_FAIL_SERF(serf) \
  failingIntegerSerfs.push(&(serf)) /* user ; */


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

  // Confirm we can modify the object through the pointer.
  s1->m_i = 33;
  EXPECT_EQ(s3->m_i, 33);

  // Let it all clean up automatically.
}

// Same thing but using Float.
static void testOperatorsFloat(bool failure)
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

  if (failure) {
    PREPARE_TO_FAIL();
    failingFloatSerfs.push(&s1);
    failingFloatSerfs.push(&s2);
    failingFloatSerfs.push(&s3);
    o1.del();
    xassert(failCount == 1);
  }

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

  xassert(failCount == 1);
  delete o1;
}


static void testRelease()
{
  RCSerf<Integer> i = new Integer(18);
  EXPECT_EQ(!!i, true);
  delete i.release();
  EXPECT_EQ(!!i, false);
}


static void testConstVersionSuccess()
{
  Owner<Integer> o(new Integer(23));
  RCSerf<Integer const> s(o);
  EXPECT_EQ(s->m_i, 23);

  // Manually test that compiler rejects this by uncommenting it.
  //s->m_i = 44;
}

static void testConstVersionFailure()
{
  Owner<Integer> o(new Integer(23));
  RCSerf<Integer const> s(o);
  EXPECT_EQ(s->m_i, 23);

  PREPARE_TO_FAIL();
  failingIntegerConstSerfs.push(&s);
  o.del();

  xassert(failCount == 1);
}


static void expectSum(RCSerfList<Integer> const &list, int expect)
{
  int sum=0;
  FOREACH_RCSERFLIST(Integer, list, iter) {
    sum += iter.data()->m_i;
  }
  EXPECT_EQ(sum, expect);
}

static void expectSumNC(RCSerfList<Integer> &list, int expect)
{
  int sum=0;
  FOREACH_RCSERFLIST_NC(Integer, list, iter) {
    sum += iter.data()->m_i;
  }
  EXPECT_EQ(sum, expect);
}

// Basic test of RCSerfList.
static void testListSuccess()
{
  Integer o1(1);
  Integer o2(2);
  Integer o4(4);

  RCSerfList<Integer> list;
  expectSum(list, 0);

  list.appendNewItem(&o1);
  expectSum(list, 1);
  EXPECT_EQ(o1.getRefCount(), 1);
  EXPECT_EQ(o2.getRefCount(), 0);
  EXPECT_EQ(o4.getRefCount(), 0);

  list.appendNewItem(&o2);
  expectSum(list, 3);

  list.appendNewItem(&o4);
  expectSum(list, 7);
  expectSumNC(list, 7);
  EXPECT_EQ(list.indexOf(&o1), 0);
  EXPECT_EQ(list.indexOf(&o2), 1);
  EXPECT_EQ(list.indexOf(&o4), 2);
  EXPECT_EQ(list.indexOf(NULL), -1);
  EXPECT_EQ(o1.getRefCount(), 1);
  EXPECT_EQ(o2.getRefCount(), 1);
  EXPECT_EQ(o4.getRefCount(), 1);

  list.removeItem(&o2);
  expectSum(list, 5);
  EXPECT_EQ(list.indexOf(&o1), 0);
  EXPECT_EQ(list.indexOf(&o2), -1);
  EXPECT_EQ(list.indexOf(&o4), 1);

  list.removeItem(&o1);
  expectSum(list, 4);
  EXPECT_EQ(list.indexOf(&o1), -1);
  EXPECT_EQ(list.indexOf(&o2), -1);
  EXPECT_EQ(list.indexOf(&o4), 0);

  list.appendNewItem(&o2);
  expectSum(list, 6);
  EXPECT_EQ(list.indexOf(&o1), -1);
  EXPECT_EQ(list.indexOf(&o2), 1);
  EXPECT_EQ(list.indexOf(&o4), 0);

  list.appendNewItem(&o1);
  expectSum(list, 7);
  EXPECT_EQ(list.indexOf(&o1), 2);
  EXPECT_EQ(list.indexOf(&o2), 1);
  EXPECT_EQ(list.indexOf(&o4), 0);

  try {
    cout << "should throw:" << endl;
    list.appendNewItem(&o4);
    cout << "should have failed" << endl;
    abort();
  }
  catch (XBase &x)
  {}

  try {
    cout << "should throw:" << endl;
    list.removeItem(NULL);
    cout << "should have failed" << endl;
    abort();
  }
  catch (XBase &x)
  {}
}

static void testListFailure()
{
  RCSerfList<Integer> list;
  PREPARE_TO_FAIL();

  {
    Owner<Integer> o1(new Integer(17));
    Owner<Integer> o2(new Integer(18));

    list.appendNewItem(o1);
    list.appendNewItem(o2);

    PUSH_FAIL_SERF(list.nthRef(0));
    PUSH_FAIL_SERF(list.nthRef(1));

    // Let o1 and o2 pass out of scope.
  }

  xassert(failCount == 1);
}


// Variants of 'testLongList'.
enum LLMode {
  LL_BASE,         // Let the dtor remove the items.
  LL_REMOVE,       // Use removeItem.
  LL_REMOVE_ALL,   // Use removeAll.
  LL_FAILURE,      // Use removeItem but forget one.
};

static void testLongList(LLMode mode)
{
  PREPARE_TO_FAIL();      // ok even if not LL_FAILURE
  int isFailure = (mode==LL_FAILURE? 1 : 0);

  {
    ObjList<Integer> olist;
    RCSerfList<Integer> slist;

    int const CT=100;

    for (int i=0; i < CT; i++) {
      Integer *obj = new Integer(i);
      olist.prepend(obj);
      EXPECT_EQ(obj->getRefCount(), 0);
      slist.appendNewItem(obj);
      EXPECT_EQ(obj->getRefCount(), 1);
    }

    EXPECT_EQ(slist.count(), CT);

    if (mode==LL_REMOVE || mode==LL_FAILURE) {
      for (int i=0; i < CT; i++) {
        Integer *obj = olist.nth(i);
        EXPECT_EQ(obj->getRefCount(), 1);
        if (isFailure && i==(CT/2)) {
          // Leave this one.
        }
        else {
          slist.removeItem(obj);
          EXPECT_EQ(obj->getRefCount(), 0);
        }
      }

      EXPECT_EQ(slist.count(), isFailure);
    }
    else if (mode==LL_REMOVE_ALL) {
      slist.removeAll();
    }

    if (isFailure) {
      PUSH_FAIL_SERF(slist.nthRef(0));
    }

    if (mode==LL_BASE) {
      // We will let the slist destructor remove its elements, and then
      // let the olist destructor trigger naturally.
    }
    else {
      // In the success cases, we cleared slist.  In the failure case,
      // we left one in it.  Trigger olist destructor now.
      olist.deleteAll();
    }
  }

  EXPECT_EQ(failCount, isFailure);
}


static void testMultipleInheritance(int failure)
{
  Owner<Super1> s1(new Super1);
  s1->x = 1;

  Owner<Super2> s2(new Super2);
  s2->y = 2;

  Owner<Sub> sub(new Sub);
  sub->x = 3;
  sub->y = 4;
  sub->z = 5;

  RCSerf<Super1> ps1(s1);
  EXPECT_EQ(ps1->x, 1);

  RCSerf<Super2> ps2(s2);
  EXPECT_EQ(ps2->y, 2);

  RCSerf<Sub> psub(sub);
  EXPECT_EQ(psub->x, 3);
  EXPECT_EQ(psub->y, 4);
  EXPECT_EQ(psub->z, 5);

  if (failure == 1) {
    PREPARE_TO_FAIL();
    failingSuper1Serfs.push(&ps1);
    s1.del();
    xassert(failCount == 1);
  }
  else if (failure == 1) {
    PREPARE_TO_FAIL();
    failingSuper2Serfs.push(&ps2);
    s2.del();
    xassert(failCount == 1);
  }
  else if (failure == 3) {
    PREPARE_TO_FAIL();
    failingSubSerfs.push(&psub);
    sub.del();
    xassert(failCount == 1);
  }
}


} // anonymous namespace


// Called from unit-tests.cc.
void test_refct_serf()
{
  testOperatorsInteger();
  testOperatorsFloat(false /*failure*/);
  testOperatorsFloat(true /*failure*/);
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
  testConstVersionSuccess();
  testConstVersionFailure();
  testListSuccess();
  testListFailure();
  testLongList(LL_BASE);
  testLongList(LL_REMOVE);
  testLongList(LL_REMOVE_ALL);
  testLongList(LL_FAILURE);
  testMultipleInheritance(false /*failure*/);
  testMultipleInheritance(true /*failure*/);

  cout << "test-refct-serf ok" << endl;
}


// EOF
