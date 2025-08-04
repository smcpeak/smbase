// sm-is-equal-test.cc
// Tests for `sm-is-equal` module.

// This file is in the public domain.

#include "sm-is-equal.h"               // module under test

// Note: This test does not include `sm-test.h` because my intention is
// that `sm-test` depends on this module and not vice-versa.
#include "smbase/xassert.h"            // xassert

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE

#include <string>                      // std::string

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  // Same type, equal.
  xassert(is_equal(0, 0));
  xassert(is_equal(1, 1));
  xassert(is_equal(-1, -1));

  // Same type, unequal.
  xassert(!is_equal(0, 1));
  xassert(!is_equal(1, 0));
  xassert(!is_equal(0, -1));
  xassert(!is_equal(-1, 0));
  xassert(!is_equal(-1, 1));

  // Different type but same signedness.
  xassert(is_equal(1, static_cast<long long>(1)));
  xassert(!is_equal(0, static_cast<long long>(1)));
  xassert(!is_equal(-1, static_cast<long long>(1)));

  // Different signedness.
  xassert(is_equal(0, 0u));
  xassert(is_equal(1, 1u));
  xassert(is_equal(0x7FFFFFFF, 0x7FFFFFFFu));

  // This is the key test: the values would compare equal if converted
  // to a common type, but I want `is_equal` to recognize they in fact
  // represent different values.
  xassert(!is_equal(-1, static_cast<unsigned>(-1)));

  // Not numeric.
  xassert(is_equal(std::string("x"), std::string("x")));
  xassert(!is_equal(std::string("x"), std::string("y")));
}


struct A {
  bool operator==(A const &a) const
    { return true; }
};

struct B : A {
  // Possible ambiguity between superclass and subclass operators?
  bool operator==(B const &b) const
    { return true; }
};


void test_subclass()
{
  // I had a problem with something similar to this elsewhere, but this
  // did not reproduce it.
  B b1, b2;
  xassert(is_equal(b1, b2));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called by unit-tests.cc.
void test_sm_is_equal()
{
  test_basics();
  test_subclass();
}


// EOF
