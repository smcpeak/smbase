// std-list-fwd-test.cc
// Tests for `std-list-fwd.h`.

#include "std-list-fwd.h"              // module under test

// Declare a function using the forward declaration.
static stdfwd::list<int> getAList();

// Here, it is not possible to use `std`.
//static std::list<int> getAList2();

#include "xassert.h"                   // xassert

#include <list>                        // std::list

// Define it using the usual name.
static std::list<int> getAList()
{
  return {1,2,3};
}

// Called from unit-tests.cc.
void test_std_list_fwd()
{
  xassert(getAList() == (std::list<int>{1,2,3}));
}


// EOF
