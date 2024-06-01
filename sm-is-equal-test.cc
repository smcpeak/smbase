// sm-is-equal-test.cc
// Tests for `sm-is-equal` module.

// This file is in the public domain.

#include "sm-is-equal.h"               // module under test

// Note: This test does not include `sm-test.h` because my intention is
// that `sm-test` depends on this module and not vice-versa.
#include "xassert.h"                   // xassert

#include <string>                      // std::string

using namespace smbase;


// Called by unit-tests.cc.
void test_sm_is_equal()
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


// EOF
