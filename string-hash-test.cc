// string-hash-test.cc
// Tests for `string-hash` module.

#include "string-hash.h"               // module under test

#include "sm-test.h"                   // VPVAL, EXPECT_EQ

using namespace smbase;


// Called from unit-tests.cc.
void test_string_hash()
{
  // I don't really have anything in mind to do with this besides
  // eyeballing the output.
  VPVAL(stringHashNulTerm(""));
  VPVAL(stringHashNulTerm("a"));
  VPVAL(stringHashNulTerm("ab"));
  VPVAL(stringHashNulTerm("abc"));
  VPVAL(stringHashNulTerm("abcdwioqdoqidwqdwiqdw"));

  // I suppose it's worth verifying that we get the same hash for two
  // strings at different locations.
  EXPECT_EQ(stringHash("bc", 2),
            stringHash("abc"+1, 2));
}


// EOF
