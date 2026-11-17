// text-table.h
// Emit/render a 2D table as plain text.

// This file is in the public domain.

#ifndef SMBASE_TEXT_TABLE_H
#define SMBASE_TEXT_TABLE_H

#include "text-table-fwd.h"            // fwds for this module

#include "smbase/sm-integer.h"         // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <string>                      // std::string
#include <variant>                     // std::variant
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Kinds of cells that can be included.
enum TextTableCellKind : int {
  // An arbitrary-precision integer, right-aligned.
  TTCK_INTEGER,

  // A string, left-aligned.
  TTCK_STRING,

  NUM_TEXT_TABLE_CELL_KINDS
};


// Data for one cell in the table.  Immutable.
class TextTableCell {
private:     // data
  // Kind of data stored in this cell.
  TextTableCellKind m_kind;

  // The data itself.
  //
  // Invariant: The active alternative must agree with `m_kind`.
  std::variant<
    Integer,
    std::string
  > m_data;

public:      // funcs
  ~TextTableCell();

  TextTableCell(TextTableCell const &obj);
  TextTableCell(TextTableCell      &&obj);

  explicit TextTableCell(Integer &&i);
  explicit TextTableCell(std::string &&s);

  TextTableCellKind getKind() const
    { return m_kind; }

  // Requires: getKind() == TTCK_INTEGER
  Integer const &getInteger() const;

  // Requires: getKind() == TTCK_STRING
  std::string const &getString() const;
};


// All table data.
//
// The data members are public for ease of access since there are no
// constraints or invariants.
//
class TextTable {
public:      // data
  // Column titles, which will be rendered left-aligned.
  std::vector<std::string> m_columnTitles;

  // Table data, organized first into rows then cells.
  std::vector<std::vector<TextTableCell>> m_cells;

public:      // funcs
  ~TextTable();

  // Make an empty table.
  TextTable();

  /*
    Return a string that has one (newline-terminated) line for the
    column titles, then a line of dashed separators, then one line for
    each row of data.  Each column has a consistent width throughout the
    table, and columns are separated by two spaces.

    If a row (including the column title row) is missing data for a
    cell, it is treated as if it had the empty string there.

    Lines do not end with padding spaces, although any trailing spaces
    that are part of a cell's own string data are retained.

    Example (overall table indentation is exposition only):

      col1        col2  col3
      ----------  ----  ----
      celldata11  d12   d13
      short row
      text         123   456
      more text      9    10

    This function currently assumes that every `char` counts as one unit
    of width (i.e., it does not handle UTF-8 text, nor tab characters).
  */
  std::string render() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_TEXT_TABLE_H
