// text-table-test.cc
// Tests for `text-table` module.

#include "smbase/text-table.h"         // module under test

#include "smbase/exc.h"                // smbase::XAssert
#include "smbase/sm-integer.h"         // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN
#include "smbase/string-util.h"        // removeTestCaseIndentation

#include <string>                      // std::string
#include <utility>                     // std::move
#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// -------------------------- TextTableCell ----------------------------
void test_cellInteger()
{
  TextTableCell cell(Integer(123));
  EXPECT_EQ(cell.getKind(), TTCK_INTEGER);
  EXPECT_EQ(cell.getInteger(), Integer(123));
  EXPECT_EXN(cell.getString(), XAssert);

  // Negative and zero.
  EXPECT_EQ(TextTableCell(Integer(-45)).getInteger(), Integer(-45));
  EXPECT_EQ(TextTableCell(Integer()).getInteger(), Integer(0));

  // A value too large for any primitive type.
  Integer big = Integer::fromDigits("123456789012345678901234567890");
  TextTableCell bigCell{Integer(big)};
  EXPECT_EQ(bigCell.getKind(), TTCK_INTEGER);
  EXPECT_EQ(bigCell.getInteger(), big);
}


void test_cellString()
{
  TextTableCell cell(std::string("hello"));
  EXPECT_EQ(cell.getKind(), TTCK_STRING);
  EXPECT_EQ(cell.getString(), "hello");
  EXPECT_EXN(cell.getInteger(), XAssert);

  // Construction from a string literal goes through `std::string`.
  TextTableCell litCell("lit");
  EXPECT_EQ(litCell.getKind(), TTCK_STRING);
  EXPECT_EQ(litCell.getString(), "lit");

  // Empty string.
  TextTableCell emptyCell{std::string()};
  EXPECT_EQ(emptyCell.getKind(), TTCK_STRING);
  EXPECT_EQ(emptyCell.getString(), "");
}


void test_cellCopyAndMove()
{
  TextTableCell const intCell(Integer(7));
  TextTableCell const strCell("seven");

  // Copy preserves kind and data, and leaves the source unchanged.
  TextTableCell intCopy(intCell);
  EXPECT_EQ(intCopy.getKind(), TTCK_INTEGER);
  EXPECT_EQ(intCopy.getInteger(), Integer(7));
  EXPECT_EQ(intCell.getInteger(), Integer(7));

  TextTableCell strCopy(strCell);
  EXPECT_EQ(strCopy.getKind(), TTCK_STRING);
  EXPECT_EQ(strCopy.getString(), "seven");
  EXPECT_EQ(strCell.getString(), "seven");

  // Move preserves kind and data.  The source's data is unspecified
  // afterward, but its kind is unchanged.
  TextTableCell intMoved(std::move(intCopy));
  EXPECT_EQ(intMoved.getKind(), TTCK_INTEGER);
  EXPECT_EQ(intMoved.getInteger(), Integer(7));
  EXPECT_EQ(intCopy.getKind(), TTCK_INTEGER);

  TextTableCell strMoved(std::move(strCopy));
  EXPECT_EQ(strMoved.getKind(), TTCK_STRING);
  EXPECT_EQ(strMoved.getString(), "seven");
  EXPECT_EQ(strCopy.getKind(), TTCK_STRING);

  // Cells can be stored in a vector, which requires copy and/or move.
  std::vector<TextTableCell> vec;
  vec.push_back(intCell);
  vec.push_back(strCell);
  vec.push_back(TextTableCell(Integer(8)));
  vec.reserve(100);                    // Force relocation.
  EXPECT_EQ(vec.at(0).getInteger(), Integer(7));
  EXPECT_EQ(vec.at(1).getString(), "seven");
  EXPECT_EQ(vec.at(2).getInteger(), Integer(8));
}


void test_cellCopyAssign()
{
  TextTableCell const intCell(Integer(7));
  TextTableCell const strCell("seven");

  // String over integer: the kind changes along with the data, and the
  // source is unchanged.
  TextTableCell c(Integer(1));
  c = strCell;
  EXPECT_EQ(c.getKind(), TTCK_STRING);
  EXPECT_EQ(c.getString(), "seven");
  EXPECT_EXN(c.getInteger(), XAssert);
  EXPECT_EQ(strCell.getString(), "seven");

  // Integer over string.
  c = intCell;
  EXPECT_EQ(c.getKind(), TTCK_INTEGER);
  EXPECT_EQ(c.getInteger(), Integer(7));
  EXPECT_EXN(c.getString(), XAssert);
  EXPECT_EQ(intCell.getInteger(), Integer(7));

  // Same kind, different value.
  TextTableCell s("old");
  s = strCell;
  EXPECT_EQ(s.getKind(), TTCK_STRING);
  EXPECT_EQ(s.getString(), "seven");

  // Self-assignment, through a reference so the compiler does not
  // complain about the obvious case.
  TextTableCell &sAlias = s;
  s = sAlias;
  EXPECT_EQ(s.getKind(), TTCK_STRING);
  EXPECT_EQ(s.getString(), "seven");

  // Chained assignment, which relies on the return value.
  TextTableCell a(Integer(0));
  TextTableCell b("b");
  a = b = intCell;
  EXPECT_EQ(a.getKind(), TTCK_INTEGER);
  EXPECT_EQ(a.getInteger(), Integer(7));
  EXPECT_EQ(b.getKind(), TTCK_INTEGER);
  EXPECT_EQ(b.getInteger(), Integer(7));

  // Copies are independent: overwriting the copy does not affect the
  // cell it was copied from.
  b = strCell;
  EXPECT_EQ(a.getInteger(), Integer(7));
}


void test_cellMoveAssign()
{
  // String over integer.
  TextTableCell c(Integer(1));
  TextTableCell strSrc("moved string");
  c = std::move(strSrc);
  EXPECT_EQ(c.getKind(), TTCK_STRING);
  EXPECT_EQ(c.getString(), "moved string");
  EXPECT_EXN(c.getInteger(), XAssert);

  // The source's data is unspecified, but its kind is unchanged.
  EXPECT_EQ(strSrc.getKind(), TTCK_STRING);

  // Integer over string, with a value too large for a primitive.
  Integer big = Integer::fromDigits("123456789012345678901234567890");
  TextTableCell intSrc{Integer(big)};
  c = std::move(intSrc);
  EXPECT_EQ(c.getKind(), TTCK_INTEGER);
  EXPECT_EQ(c.getInteger(), big);
  EXPECT_EXN(c.getString(), XAssert);
  EXPECT_EQ(intSrc.getKind(), TTCK_INTEGER);

  // Assigning from a temporary.
  c = TextTableCell("temp");
  EXPECT_EQ(c.getKind(), TTCK_STRING);
  EXPECT_EQ(c.getString(), "temp");

  // Self move-assignment leaves the cell unchanged.
  TextTableCell &cAlias = c;
  c = std::move(cAlias);
  EXPECT_EQ(c.getKind(), TTCK_STRING);
  EXPECT_EQ(c.getString(), "temp");

  // A moved-from cell can be assigned a new value.
  strSrc = TextTableCell(Integer(5));
  EXPECT_EQ(strSrc.getKind(), TTCK_INTEGER);
  EXPECT_EQ(strSrc.getInteger(), Integer(5));
}


// Vector operations that shift elements, and hence need assignment.
void test_cellAssignInVector()
{
  std::vector<TextTableCell> vec;
  vec.push_back(TextTableCell("a"));
  vec.push_back(TextTableCell(Integer(2)));
  vec.push_back(TextTableCell("c"));

  // Erasing from the front move-assigns the later elements down.
  vec.erase(vec.begin());
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec.at(0).getKind(), TTCK_INTEGER);
  EXPECT_EQ(vec.at(0).getInteger(), Integer(2));
  EXPECT_EQ(vec.at(1).getString(), "c");

  // Inserting at the front shifts them back up.
  vec.insert(vec.begin(), TextTableCell("z"));
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec.at(0).getString(), "z");
  EXPECT_EQ(vec.at(1).getInteger(), Integer(2));
  EXPECT_EQ(vec.at(2).getString(), "c");

  // Assigning a braced list copy-assigns elements into existing ones.
  vec = { TextTableCell(Integer(9)), TextTableCell("y") };
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec.at(0).getInteger(), Integer(9));
  EXPECT_EQ(vec.at(1).getString(), "y");
}


// ---------------------------- TextTable ------------------------------
// Table data, for tests that build a complete set of rows at once.
using Cells = std::vector<std::vector<TextTableCell>>;


// Assigning a braced list directly to `m_cells`, which requires
// `TextTableCell` to be copy-assignable.
void test_bracedListAssign()
{
  TextTable tt;
  tt.m_columnTitles = {"k", "v"};
  tt.m_cells = {
    { TextTableCell("one"), TextTableCell(Integer(1)) },
  };

  // Assign again so the existing elements are assigned over.
  tt.m_cells = {
    { TextTableCell("two"), TextTableCell(Integer(22)) },
    { TextTableCell("three") },
  };

  // I'll leave this test using ordinary string literals instead of
  // `removeTestCaseIndentation` just so there is one that is a little
  // more explicit about where whitespace is.
  EXPECT_EQ(tt.render(),
    "k      v\n"
    "-----  --\n"
    "two    22\n"
    "three\n");
}


// Build the table shown in the `render` documentation.
void test_docExample()
{
  TextTable tt;

  tt.m_columnTitles = std::vector<std::string>{
    "col1", "col2", "col3",
  };

  tt.m_cells = Cells{
    {
      TextTableCell("celldata11"),
      TextTableCell("d12"),
      TextTableCell("d13"),
    },
    {
      TextTableCell("short row"),
    },
    {
      TextTableCell("text"),
      TextTableCell(Integer(123)),
      TextTableCell(Integer(456)),
    },
    {
      TextTableCell("more text"),
      TextTableCell(Integer(9)),
      TextTableCell(Integer(10)),
    },
  };

  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(
    col1        col2  col3
    ----------  ----  ----
    celldata11  d12   d13
    short row
    text         123   456
    more text      9    10
  )"));
}


// No titles and no data: there are zero columns, but the title and
// separator lines are still present.
void test_emptyTable()
{
  TextTable tt;
  EXPECT_EQ(tt.render(), "\n\n");

  // Empty rows add empty lines.
  tt.m_cells.resize(2);
  EXPECT_EQ(tt.render(), "\n\n\n\n");
}


// Titles but no data rows.
void test_titlesOnly()
{
  TextTable tt;
  tt.m_columnTitles = {"a", "bbb", "cc"};
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(
    a  bbb  cc
    -  ---  --
  )"));
}


// Data but no column titles: the title line is empty, but the
// separators still span the data columns.
void test_dataWithoutTitles()
{
  TextTable tt;
  tt.m_cells = Cells{
    { TextTableCell("x"), TextTableCell(Integer(10)) },
    { TextTableCell("longer"), TextTableCell(Integer(5)) },
  };
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(

    ------  --
    x       10
    longer   5
  )"));
}


// Rows with more cells than there are titles.  The extra columns get
// empty titles.
void test_moreCellsThanTitles()
{
  TextTable tt;
  tt.m_columnTitles = {"name"};
  tt.m_cells = Cells{
    { TextTableCell("a"), TextTableCell("b"), TextTableCell("c") },
    { TextTableCell("d") },
  };
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(
    name
    ----  -  -
    a     b  c
    d
  )"));
}


// Alignment is per cell kind, so one column can mix left-aligned
// strings and right-aligned integers.
void test_mixedColumn()
{
  TextTable tt;
  tt.m_columnTitles = {"v", "end"};
  tt.m_cells = Cells{
    { TextTableCell("abcd"),       TextTableCell("e") },
    { TextTableCell(Integer(5)),   TextTableCell("e") },
    { TextTableCell(Integer(-12)), TextTableCell("e") },
  };
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(
    v     end
    ----  ---
    abcd  e
       5  e
     -12  e
  )"));
}


// Columns that are wholly or partly empty.
void test_emptyCells()
{
  TextTable tt;
  tt.m_cells = Cells{
    { TextTableCell("a"), TextTableCell(""), TextTableCell("b") },
    { TextTableCell(""),  TextTableCell(""), TextTableCell("c") },
    { TextTableCell("d"), TextTableCell(""), TextTableCell("") },
  };

  // The middle column has width 0.  An empty leading cell still yields
  // padding, but empty trailing cells do not.
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(

    -    -
    a    b
         c
    d
  )"));
}


// Spaces that are part of a cell's string are kept, even at the end of
// a line.  Only padding is removed.
void test_trailingSpacesInData()
{
  TextTable tt;
  tt.m_columnTitles = {"t1", "t2"};
  tt.m_cells = Cells{
    { TextTableCell("x"), TextTableCell("y ") },
    { TextTableCell("z ") },
  };

  // This does not use `removeTestCaseIndentation` since that would
  // obscure the trailing space behavior being tested.
  EXPECT_EQ(tt.render(),
    "t1  t2\n"
    "--  --\n"
    "x   y \n"
    "z \n");
}


// An integer too large for any primitive type.
void test_bigInteger()
{
  TextTable tt;
  tt.m_columnTitles = {"n"};
  tt.m_cells = Cells{
    { TextTableCell(Integer::fromDigits("123456789012345678901234567890")) },
    { TextTableCell(Integer(1)) },
  };
  EXPECT_EQ(tt.render(), removeTestCaseIndentation(R"(
    n
    ------------------------------
    123456789012345678901234567890
                                 1
  )"));
}


// `render` does not modify the table, so it can be called repeatedly.
void test_renderTwice()
{
  TextTable tt;
  tt.m_columnTitles = {"a"};
  tt.m_cells = Cells{{ TextTableCell(Integer(1)) }};
  std::string first = tt.render();
  EXPECT_EQ(tt.render(), first);
  EXPECT_EQ(first, removeTestCaseIndentation(R"(
    a
    -
    1
  )"));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_text_table()
{
  test_cellInteger();
  test_cellString();
  test_cellCopyAndMove();
  test_cellCopyAssign();
  test_cellMoveAssign();
  test_cellAssignInVector();

  test_docExample();
  test_bracedListAssign();
  test_emptyTable();
  test_titlesOnly();
  test_dataWithoutTitles();
  test_moreCellsThanTitles();
  test_mixedColumn();
  test_emptyCells();
  test_trailingSpacesInData();
  test_bigInteger();
  test_renderTwice();
}


// EOF
