// astlist-test.cc
// Tests for astlist module.

#include "astlist.h"                   // module under test

#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "xassert.h"                   // xassert


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


// TODO: Test other things!


void test_astlist()
{
  testSTLBasics();
}


// EOF
