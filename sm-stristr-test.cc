// sm-stristr-test.cc
// Test code for sm-stristr.

#include "sm-stristr.h"                // module under test

#include "xassert.h"                   // xassert

#include <cstring>                     // std::strcmp


namespace { // anonymous namespace for test code


void test_equalChars_insens_ascii()
{
  // The test assumes that the C++ implementation's execution character
  // encoding is US-ASCII or a subset of that.
  xassert(equalChars_insens_ascii('A', 'A'));
  xassert(equalChars_insens_ascii('A', 'a'));
  xassert(equalChars_insens_ascii('a', 'A'));
  xassert(!equalChars_insens_ascii('A', 'B'));
}


void test_prefixEquals_insens_ascii()
{
  xassert(prefixEquals_insens_ascii("", ""));
  xassert(prefixEquals_insens_ascii("x", ""));
  xassert(!prefixEquals_insens_ascii("", "x"));

  xassert(prefixEquals_insens_ascii("abc", "abc"));
  xassert(prefixEquals_insens_ascii("abc", "ABC"));
  xassert(prefixEquals_insens_ascii("ABc", "abC"));
  xassert(prefixEquals_insens_ascii("ABc", "ab"));

  xassert(!prefixEquals_insens_ascii("ABc", "abd"));
  xassert(!prefixEquals_insens_ascii("bBc", "abc"));
}


void test_findSubstring_insens_ascii()
{
  xassert(0==std::strcmp(findSubstring_insens_ascii("abcdef", "BcD"), "bcdef"));
  xassert(nullptr==findSubstring_insens_ascii("abcdef", "BcDd"));
}


} // anonymous namespace for test code


void test_sm_stristr()
{
  test_equalChars_insens_ascii();
  test_prefixEquals_insens_ascii();
  test_findSubstring_insens_ascii();
}


// EOF
