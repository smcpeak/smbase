// type-name-and-size-test.cc
// Tests for `type-name-and-size`.

// See license.txt for copyright and terms of use.

#include "smbase/type-name-and-size.h" // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cstdint>                     // std::int8_t


using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testBasics()
{
  TypeNameAndSize tnas("int", 32);
  EXPECT_EQ(tnas.toString(), "\"int\" (32 bits)");

  tnas = makeTypeNameAndSizeForType<unsigned int>();
  EXPECT_EQ(tnas.toString(), "\"unsigned int\" (32 bits)");

  tnas = makeTypeNameAndSizeForType<signed char>();
  EXPECT_EQ(tnas.toString(), "\"signed char\" (8 bits)");

  // The type name extractor sees types as they are described by
  // mangled names, so it does not see typedefs.
  tnas = makeTypeNameAndSizeForType<std::int8_t>();
  EXPECT_EQ(tnas.toString(), "\"signed char\" (8 bits)");
}


CLOSE_ANONYMOUS_NAMESPACE


void test_type_name_and_size()
{
  testBasics();
}


// EOF
