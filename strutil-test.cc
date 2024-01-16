// strutil-test.cc
// Tests for strutil.h.

#include "strutil.h"                   // module under test


namespace { // anonymous namespace for test code


void test_hasSubstring_insens_ascii()
{
  xassert(hasSubstring_insens_ascii("", ""));
  xassert(hasSubstring_insens_ascii("x", ""));
  xassert(!hasSubstring_insens_ascii("", "x"));

  xassert(hasSubstring_insens_ascii("abcdef", "BcD"));
  xassert(!hasSubstring_insens_ascii("abccdef", "BcD"));
}


} // anonymous namespace for test code


void test_strutil()
{
  test_hasSubstring_insens_ascii();

  // TODO: Most of the 'strutil' module lacks tests--add them!
}


// EOF
