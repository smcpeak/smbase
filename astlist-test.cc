// astlist-test.cc
// Tests for astlist module.

#include "astlist.h"                   // module under test

#include "smbase/sm-macros.h"          // NO_OBJECT_COPIES
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/xassert.h"            // xassert


namespace {
  class Integer {
    NO_OBJECT_COPIES(Integer);

  public:      // class data
    static int s_objectCount;

  public:      // instance data
    int m_i;

  public:      // methods
    Integer(int i)
      : m_i(i)
    {
      s_objectCount++;
    }

    ~Integer()
    {
      s_objectCount--;
    }
  };

  int Integer::s_objectCount = 0;
}


// Exercise the STL compatibility methods.
static void testSTLBasics()
{
  ASTList<Integer> list;

  xassert(list.empty());
  xassert(list.size() == 0);
  xassert(Integer::s_objectCount == 0);

  list.push_back(new Integer(1));

  xassert(!list.empty());
  xassert(list.size() == 1);
  xassert(list.front()->m_i == 1);
  xassert(list.back()->m_i == 1);
  xassert(list.at(0)->m_i == 1);

  list.push_back(new Integer(2));

  xassert(!list.empty());
  xassert(list.size() == 2);
  xassert(list.front()->m_i == 1);
  xassert(list.back()->m_i == 2);
  xassert(list.at(0)->m_i == 1);
  xassert(list.at(1)->m_i == 2);

  list.push_back(new Integer(3));

  xassert(!list.empty());
  xassert(list.size() == 3);
  xassert(list.front()->m_i == 1);
  xassert(list.back()->m_i == 3);
  xassert(list.at(0)->m_i == 1);
  xassert(list.at(1)->m_i == 2);
  xassert(list.at(2)->m_i == 3);
  xassert(Integer::s_objectCount == 3);

  ASTList<Integer> const &clist = list;

  xassert(!clist.empty());
  xassert(clist.size() == 3);
  xassert(clist.front()->m_i == 1);
  xassert(clist.back()->m_i == 3);
  xassert(clist.at(0)->m_i == 1);
  xassert(clist.at(1)->m_i == 2);
  xassert(clist.at(2)->m_i == 3);

  list.clear();

  xassert(list.empty());
  xassert(list.size() == 0);
  xassert(Integer::s_objectCount == 0);
}


static void testStealingCtor()
{
  ASTList<Integer> *list1 = new ASTList<Integer>(new Integer(1));
  list1->append(new Integer(2));

  // Deallocates 'list1'.
  ASTList<Integer> list2(list1);
  xassert(list2.size() == 2);
}

static void testSteal()
{
  ASTList<Integer> *list1 = new ASTList<Integer>(new Integer(1));
  list1->append(new Integer(2));

  ASTList<Integer> list2;

  // Deallocates 'list1'.
  list2.steal(list1);
  xassert(list2.size() == 2);
}

static void testStealElements()
{
  ASTList<Integer> list1(new Integer(1));
  list1.append(new Integer(2));

  ASTList<Integer> list2;
  list2.stealElements(&list1);

  xassert(list1.size() == 0);
  xassert(list2.size() == 2);
}

static void testMoveCtor()
{
  ASTList<Integer> list1;
  list1.append(new Integer(1));
  list1.append(new Integer(2));
  EXPECT_EQ(list1.count(), 2);

  ASTList<Integer> list2(std::move(list1));
  EXPECT_EQ(list1.count(), 0);         // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_EQ(list2.count(), 2);
}


// TODO: Test other things!


void test_astlist()
{
  testSTLBasics();
  testStealingCtor();
  testSteal();
  testStealElements();
  testMoveCtor();

  xassert(Integer::s_objectCount == 0);
}


// EOF
