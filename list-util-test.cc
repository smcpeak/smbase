// list-util-test.cc
// Tests for `list-util.h`.

#include "list-util.h"                 // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ

#include <list>                        // std::list
#include <string>                      // std::string


using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_listMoveFront()
{
  std::list<std::string> lst { "one", "two" };

  EXPECT_EQ(listMoveFront(lst), "one");
  EXPECT_EQ(listMoveFront(lst), "two");
  xassert(lst.empty());
}


void test_listAt()
{
  std::list<std::string> lst { "one", "two", "three" };

  EXPECT_EQ(listAtC(lst, 0), "one");
  EXPECT_EQ(listAtC(lst, 1), "two");
  EXPECT_EQ(listAtC(lst, 2), "three");

  EXPECT_EXN_SUBSTR(listAtC(lst, 3),
    XAssert, "fewer than 3 elements");

  listAt(lst, 1) = "TWO";
  EXPECT_EQ(listAtC(lst, 1), "TWO");
  EXPECT_EQ(listAt(lst, 1), "TWO");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_list_util()
{
  test_listMoveFront();
  test_listAt();
}


// EOF
