// file-line-col-test.cc
// Tests for `file-line-col` module.

#include "smbase/file-line-col.h"      // module under test

#include "smbase/compare-util.h"       // smbase::compare
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test-order.h"      // EXPECT_STRICTLY_ORDERED
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_TRUE, EXPECT_FALSE

#include <sstream>                     // std::ostringstream

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_ctor()
{
  TEST_CASE(__func__);

  {
    FileLineCol flc;
    flc.selfCheck();
    EXPECT_EQ(flc.m_fileName.has_value(), false);
    EXPECT_EQ(flc.m_lc.m_line, 1);
    EXPECT_EQ(flc.m_lc.m_column, 1);
    EXPECT_EQ(flc.m_lc.m_byteOffset, 0);
  }

  {
    FileLineCol flc("foo.txt", 3, 7, 42);
    flc.selfCheck();
    EXPECT_EQ(flc.m_fileName.has_value(), true);
    EXPECT_EQ(*flc.m_fileName, "foo.txt");
    EXPECT_EQ(flc.m_lc.m_line, 3);
    EXPECT_EQ(flc.m_lc.m_column, 7);
    EXPECT_EQ(flc.m_lc.m_byteOffset, 42);
  }
}


void test_copy_assign()
{
  TEST_CASE(__func__);

  FileLineCol a("bar.txt", 2, 5, 10);
  FileLineCol b(a);   // copy ctor
  b.selfCheck();
  EXPECT_TRUE(b.m_fileName.has_value());
  EXPECT_EQ(*b.m_fileName, "bar.txt");
  EXPECT_EQ(b.m_lc.m_line, 2);

  FileLineCol c;
  c = a;              // copy assignment
  c.selfCheck();
  EXPECT_EQ(c.m_lc.m_column, 5);
}


void test_comparisons()
{
  TEST_CASE(__func__);

  FileLineCol a("a.txt", 1, 2, 0);
  FileLineCol b("a.txt", 1, 2, 5);
  FileLineCol c("b.txt", 1, 1, 0);
  FileLineCol d(std::nullopt, 1, 2, 0);

  EXPECT_TRUE(a < b);    // compare by lc
  EXPECT_TRUE(a < c);    // compare by fileName
  EXPECT_TRUE(c > b);
  EXPECT_TRUE(d < a);    // no filename sorts before one with filename
  EXPECT_TRUE(a == FileLineCol("a.txt", 1, 2, 0));
  EXPECT_TRUE(a != b);

  EXPECT_STRICTLY_ORDERED(FileLineCol, d, a, b, c);
}


void test_asString_and_write()
{
  TEST_CASE(__func__);

  {
    FileLineCol flc("baz.cpp", 4, 8, 99);
    EXPECT_EQ(flc.asString(), "baz.cpp: 4:8");

    std::ostringstream oss;
    oss << flc;
    EXPECT_EQ(oss.str(), "baz.cpp: 4:8");
  }

  {
    FileLineCol flc(std::nullopt, 2, 6, 10);
    EXPECT_EQ(flc.asString(), "2:6");

    std::ostringstream oss;
    oss << flc;
    EXPECT_EQ(oss.str(), "2:6");
  }
}


void test_linecol_delegation()
{
  TEST_CASE(__func__);

  FileLineCol flc(std::nullopt, 1, 1, 0);

  flc.incrementForChar('x');
  EXPECT_EQ(flc.m_lc.m_column, 2);

  flc.incrementForChar('\n');
  EXPECT_EQ(flc.m_lc.m_line, 2);
  EXPECT_EQ(flc.m_lc.m_column, 1);

  flc.decrementColumn();
  EXPECT_EQ(flc.m_lc.m_column, 0);

  flc.decrementForChar('\n');
  EXPECT_EQ(flc.m_lc.m_line, 1);
  EXPECT_EQ(flc.m_lc.m_column, 0);
}


void test_get_set_LineCol()
{
  TEST_CASE(__func__);

  FileLineCol flc("foo.cpp", 1, 1, 0);
  LineCol lc(10, 20, 30);
  flc.setLineCol(lc);

  LineCol const &got = flc.getLineCol();
  EXPECT_EQ(got.m_line, 10);
  EXPECT_EQ(got.m_column, 20);
  EXPECT_EQ(got.m_byteOffset, 30);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_file_line_col()
{
  test_ctor();
  test_copy_assign();
  test_comparisons();
  test_asString_and_write();
  test_linecol_delegation();
  test_get_set_LineCol();
}


// EOF
