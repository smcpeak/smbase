// text-table-test.cc
// Tests for `text-table` module.

#include "smbase/text-table.h"         // module under test

#include "smbase/sm-integer.h"         // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <string>                      // std::string
#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Build the table shown in the `render` documentation.
void test_docExample()
{
  TextTable tt;

  tt.m_columnTitles = std::vector<std::string>{
    "col1", "col2", "col3",
  };

  tt.m_cells = std::vector<std::vector<TextTableCell>>{
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


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_text_table()
{
  test_docExample();
}


// EOF
