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


CLOSE_ANONYMOUS_NAMESPACE


void test_list_util()
{
  test_listMoveFront();
}


// EOF
