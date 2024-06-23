// reader-test.cc
// Tests for `reader` module.

#include "reader.h"                    // module under test

#include "sm-test.h"                   // EXPECT_EQ

#include <sstream>                     // std::istringstream

using namespace smbase;


static void testSimple()
{
  std::istringstream iss(std::string("abc"));

  Reader r(iss, std::string("fname"));
  EXPECT_EQ(r.m_location.m_lc.m_line, 1);
  EXPECT_EQ(r.m_location.m_lc.m_column, 1);
  EXPECT_EQ(r.m_location.m_fileName.value(), "fname");

  EXPECT_EQ(r.readChar(), 'a');
  EXPECT_EQ(r.m_location.m_lc.m_column, 2);

  EXPECT_EQ(r.readChar(), 'b');
  EXPECT_EQ(r.m_location.m_lc.m_column, 3);

  r.putback('b');
  EXPECT_EQ(r.m_location.m_lc.m_column, 2);

  EXPECT_EQ(r.readChar(), 'b');
  EXPECT_EQ(r.m_location.m_lc.m_column, 3);

  EXPECT_EQ(r.readChar(), 'c');
  EXPECT_EQ(r.m_location.m_lc.m_column, 4);

  EXPECT_EQ(r.readChar(), r.eofCode());
  EXPECT_EQ(r.m_location.m_lc.m_column, 5);

  r.putback(r.eofCode());
  EXPECT_EQ(r.m_location.m_lc.m_column, 4);
}


static void testError()
{
  std::istringstream iss(std::string("abc"));

  Reader r(iss);

  try {
    r.readChar();
    r.err("blah");
    xfailure("should have thrown");
  }
  catch (ReaderException &x) {
    EXPECT_HAS_SUBSTRING(x.getMessage(), "blah");
    EXPECT_EQ(x.m_location.m_lc.m_line, 1);
    EXPECT_EQ(x.m_location.m_lc.m_column, 1);
  }

  // I could do more, but the tests in `gdvalue-test` are already
  // pretty thorough.
}


// Called from unit-tests.cc.
void test_reader()
{
  testSimple();
  testError();
}


// EOF
