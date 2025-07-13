// string-reader-test.cc
// Tests for `string-reader` module.

#include "smbase/string-reader.h"      // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testBasics()
{
  StringReader reader("abc");

  EXPECT_EQ(reader.readChar(), 'a');
  EXPECT_EQ(reader.readChar(), 'b');
  EXPECT_EQ(reader.m_location.m_lc.m_byteOffset, 2);
  EXPECT_EQ(reader.readChar(), 'c');
  EXPECT_EQ(reader.readChar(), Reader::eofCode());

  // The offset gets incremented when we read EOF.  (This design is
  // pretty questionable...)
  EXPECT_EQ(reader.m_location.m_lc.m_byteOffset, 4);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_string_reader()
{
  testBasics();
}


// EOF
