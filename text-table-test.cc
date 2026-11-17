// text-table-test.cc
// Tests for `text-table` module.

#include "smbase/text-table.h"         // module under test

#include "smbase/exc.h"                // smbase::XAssert
#include "smbase/sm-integer.h"         // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN

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


// ---------------------------- TextTable ------------------------------
// Table data.  Assigning a braced list directly to `m_cells` would
// require `TextTableCell` to be assignable, which it is not, so build a
// vector and move-assign it instead.
using Cells = std::vector<std::vector<TextTableCell>>;


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

  // TODO: Make a Python-like `dedent` for cases like this.
  EXPECT_EQ(tt.render(),
R"(col1        col2  col3
----------  ----  ----
celldata11  d12   d13
short row
text         123   456
more text      9    10
)");
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
  EXPECT_EQ(tt.render(),
    "a  bbb  cc\n"
    "-  ---  --\n");
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
  EXPECT_EQ(tt.render(),
    "\n"
    "------  --\n"
    "x       10\n"
    "longer   5\n");
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
  EXPECT_EQ(tt.render(),
    "name\n"
    "----  -  -\n"
    "a     b  c\n"
    "d\n");
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
  EXPECT_EQ(tt.render(),
    "v     end\n"
    "----  ---\n"
    "abcd  e\n"
    "   5  e\n"
    " -12  e\n");
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
  EXPECT_EQ(tt.render(),
    "\n"
    "-    -\n"
    "a    b\n"
    "     c\n"
    "d\n");
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
  EXPECT_EQ(tt.render(),
    "n\n"
    "------------------------------\n"
    "123456789012345678901234567890\n"
    "                             1\n");
}


// `render` does not modify the table, so it can be called repeatedly.
void test_renderTwice()
{
  TextTable tt;
  tt.m_columnTitles = {"a"};
  tt.m_cells = Cells{{ TextTableCell(Integer(1)) }};
  std::string first = tt.render();
  EXPECT_EQ(tt.render(), first);
  EXPECT_EQ(first, "a\n-\n1\n");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_text_table()
{
  test_cellInteger();
  test_cellString();
  test_cellCopyAndMove();

  test_docExample();
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
