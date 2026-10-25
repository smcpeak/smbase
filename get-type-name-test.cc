// get-type-name-test.cc
// Tests for `get-type-name` module.

#include "get-type-name.h"             // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ, VPVAL

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class SomeClass {};


CLOSE_ANONYMOUS_NAMESPACE


// Called by unit-tests.cc.
void test_get_type_name()
{
  // For an unsupported compiler, do not fail, but print a warning.
  if (GetTypeName<int>::value == "unknown") {
    cout << "get-type-name tests skipped because it returns \"unknown\".\n";
    cout << "It needs compiler support to work properly.\n";
    return;
  }

  EXPECT_EQ(GetTypeName<int>::value, "int");
  EXPECT_EQ(GetTypeName<unsigned int>::value, "unsigned int");
  EXPECT_EQ(GetTypeName<GetTypeName<int> >::value, "smbase::GetTypeName<int>");

  // The un-normalized name differs by compiler, so we just print it in
  // verbose mode.
  VPVAL(GetTypeName<SomeClass>::value);

  // Test that normalization produces a consistent result.  If this
  // fails, most likely another transformation has to be added to
  // `normalizeTypeName` to handle a new compiler.
  EXPECT_EQ(normalizeTypeName(GetTypeName<SomeClass>::value),
    "{anonymous}::SomeClass");
}


// EOF
