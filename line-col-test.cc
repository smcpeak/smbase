// line-col-test.cc
// Tests for `line-col` module.

// TODO: Split `line-col` off.
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
    LineCol lc;
    lc.selfCheck();
    EXPECT_EQ(lc.m_line, 1);
    EXPECT_EQ(lc.m_column, 1);
    EXPECT_EQ(lc.m_byteOffset, 0);
  }

  {
    LineCol lc(1, 2, 3);
    lc.selfCheck();
    EXPECT_EQ(lc.m_line, 1);
    EXPECT_EQ(lc.m_column, 2);
    EXPECT_EQ(lc.m_byteOffset, 3);
  }
}


void test_copy_assign()
{
  TEST_CASE(__func__);

  LineCol a(2, 3, 10);
  LineCol b(a);   // copy ctor
  b.selfCheck();
  EXPECT_EQ(b.m_line, 2);
  EXPECT_EQ(b.m_column, 3);
  EXPECT_EQ(b.m_byteOffset, 10);

  LineCol c;
  c = a;          // copy assignment
  c.selfCheck();
  EXPECT_EQ(c.m_line, 2);
  EXPECT_EQ(c.m_column, 3);
  EXPECT_EQ(c.m_byteOffset, 10);
}


void test_comparisons()
{
  TEST_CASE(__func__);

  LineCol a(1, 2, 0);
  LineCol b(1, 2, 5);   // Differing byte offset.
  LineCol c(2, 1, 0);
  LineCol d(2, 2, 0);   // Differing column.
  LineCol e(3, 2, 0);   // Differing line.

  EXPECT_TRUE(a < b);   // compares by byteOffset after line/col equal
  EXPECT_TRUE(a < c);   // compares by line
  EXPECT_TRUE(c > b);
  EXPECT_TRUE(a == LineCol(1,2,0));
  EXPECT_TRUE(a != b);

  EXPECT_STRICTLY_ORDERED(LineCol, a, b, c, d, e);
}


void test_asString_and_write()
{
  TEST_CASE(__func__);

  LineCol lc(3, 7, 100);
  EXPECT_EQ(lc.asString(), "3:7");

  std::ostringstream oss;
  oss << lc;
  EXPECT_EQ(oss.str(), "3:7");
}


void test_incrementForChar()
{
  TEST_CASE(__func__);

  LineCol lc(1, 1, 0);

  lc.incrementForChar('a');  // non-newline
  lc.selfCheck();
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 2);
  EXPECT_EQ(lc.m_byteOffset, 1);

  lc.incrementForChar('\n'); // newline
  lc.selfCheck();
  EXPECT_EQ(lc.m_line, 2);
  EXPECT_EQ(lc.m_column, 1);
  EXPECT_EQ(lc.m_byteOffset, 2);
}


void test_decrementColumn()
{
  TEST_CASE(__func__);

  LineCol lc(1, 2, 10);
  lc.decrementColumn();
  EXPECT_EQ(lc.m_column, 1);
  EXPECT_EQ(lc.m_byteOffset, 9);

  lc.decrementColumn();
  EXPECT_EQ(lc.m_column, 0);
  EXPECT_EQ(lc.m_byteOffset, 8);

  lc.decrementColumn();      // should stay at 0
  EXPECT_EQ(lc.m_column, 0);
  EXPECT_EQ(lc.m_byteOffset, 7);
}


void test_decrementForChar()
{
  TEST_CASE(__func__);

  LineCol lc(1, 1, 10);

  // Increment with 'x', then undo it.
  lc.incrementForChar('x');
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 2);
  EXPECT_EQ(lc.m_byteOffset, 11);
  lc.decrementForChar('x');
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 1);
  EXPECT_EQ(lc.m_byteOffset, 10);

  // Further decrements stay on the same line if non-newline.
  lc.decrementForChar('x');
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 0);
  EXPECT_EQ(lc.m_byteOffset, 9);
  lc.decrementForChar('x');
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 0);
  EXPECT_EQ(lc.m_byteOffset, 8);

  // Increment with '\n', then undo it.
  lc.incrementForChar('\n');
  EXPECT_EQ(lc.m_line, 2);
  EXPECT_EQ(lc.m_column, 1);
  EXPECT_EQ(lc.m_byteOffset, 9);
  lc.decrementForChar('\n');
  EXPECT_EQ(lc.m_line, 1);
  EXPECT_EQ(lc.m_column, 0);
  EXPECT_EQ(lc.m_byteOffset, 8);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_line_col()
{
  test_ctor();
  test_copy_assign();
  test_comparisons();
  test_asString_and_write();
  test_incrementForChar();
  test_decrementColumn();
  test_decrementForChar();
}


// EOF
