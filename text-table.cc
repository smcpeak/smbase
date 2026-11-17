// text-table.cc
// Code for `text-table` module.

#include "text-table.h"                // this module

#include "smbase/sm-integer.h"         // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, MDMEMB, CMEMB, MCMEMB
#include "smbase/xassert.h"            // xassertPrecondition

#include <algorithm>                   // std::max
#include <cstddef>                     // std::size_t
#include <string>                      // std::string
#include <utility>                     // std::move
#include <variant>                     // std::get
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// --------------------------- TextTableCell ---------------------------
TextTableCell::~TextTableCell()
{}


TextTableCell::TextTableCell(TextTableCell const &obj)
  : DMEMB(m_kind),
    DMEMB(m_data)
{}


TextTableCell::TextTableCell(TextTableCell &&obj)
  : MDMEMB(m_kind),
    MDMEMB(m_data)
{}


TextTableCell::TextTableCell(Integer &&i)
  : m_kind(TTCK_INTEGER),
    m_data(std::move(i))
{}


TextTableCell::TextTableCell(std::string &&s)
  : m_kind(TTCK_STRING),
    m_data(std::move(s))
{}


// In both assignment operators, `m_data` is assigned first so that, if
// it throws, `m_kind` has not yet been changed to disagree with it.

TextTableCell &TextTableCell::operator=(TextTableCell const &obj)
{
  if (this != &obj) {
    CMEMB(m_data);
    CMEMB(m_kind);
  }
  return *this;
}


TextTableCell &TextTableCell::operator=(TextTableCell &&obj)
{
  if (this != &obj) {
    MCMEMB(m_data);
    MCMEMB(m_kind);
  }
  return *this;
}


Integer const &TextTableCell::getInteger() const
{
  xassertPrecondition(m_kind == TTCK_INTEGER);
  return std::get<Integer>(m_data);
}


std::string const &TextTableCell::getString() const
{
  xassertPrecondition(m_kind == TTCK_STRING);
  return std::get<std::string>(m_data);
}


// ----------------------------- TextTable -----------------------------
TextTable::~TextTable()
{}


TextTable::TextTable()
  : m_columnTitles(),
    m_cells()
{}


OPEN_ANONYMOUS_NAMESPACE


// Cell text and alignment.
struct RenderedCell {
  // Text for the cell.
  std::string m_text;

  // True to align on the right side of the column, false for left.
  bool m_rightAlign = false;
};


// One line of output, before padding.  It may have fewer elements
// than there are columns.
using RenderedLine = std::vector<RenderedCell>;


RenderedCell renderCell(TextTableCell const &cell)
{
  switch (cell.getKind()) {
    case TTCK_INTEGER:
      return RenderedCell{cell.getInteger().toString(), true /*right*/};

    case TTCK_STRING:
      return RenderedCell{cell.getString(), false /*right*/};

    default:
      break;
  }

  xfailure("invalid TextTableCellKind");
  return RenderedCell{};             // Not reached.
}


// Append to `dest` the padded form of `line`, using `columnWidths`,
// then a newline.  Padding that would be at the end of the line is
// omitted, but any trailing spaces within the cell `m_text` are kept.
void appendLine(
  std::string &dest,
  RenderedLine const &line,
  std::vector<std::size_t> const &columnWidths)
{
  std::size_t const lineStart = dest.size();

  // Position just past the last non-empty cell text appended so far.
  // This position does not include padding added after left-aligned
  // text.  This mechanism allows all trailing padding to be removed at
  // the end of the line (via `resize`) while retaining any spaces that
  // came from `m_text`.
  std::size_t contentEnd = lineStart;

  for (std::size_t c=0; c < columnWidths.size(); ++c) {
    if (c > 0) {
      // Spacing between columns.
      dest += "  ";
    }

    // A missing cell is treated like an empty string.
    static RenderedCell const emptyCell;
    RenderedCell const &cell = (c < line.size()? line[c] : emptyCell);

    xassert(cell.m_text.size() <= columnWidths[c]);
    std::string const padding(columnWidths[c] - cell.m_text.size(), ' ');

    if (cell.m_rightAlign) {
      dest += padding;
      dest += cell.m_text;
      if (!cell.m_text.empty()) {
        contentEnd = dest.size();
      }
    }
    else {
      dest += cell.m_text;
      if (!cell.m_text.empty()) {
        contentEnd = dest.size();
      }
      dest += padding;
    }
  }

  // Remove trailing padding and separators.
  dest.resize(contentEnd);
  dest += '\n';
}


CLOSE_ANONYMOUS_NAMESPACE


std::string TextTable::render() const
{
  // Convert everything to text first, so each `Integer` is converted
  // only once, and the title and separator lines can be handled the
  // same way as the data.
  RenderedLine titleLine;
  for (std::string const &title : m_columnTitles) {
    titleLine.push_back(RenderedCell{title, false /*right*/});
  }

  std::vector<RenderedLine> dataLines;
  for (std::vector<TextTableCell> const &row : m_cells) {
    RenderedLine &line = dataLines.emplace_back();
    for (TextTableCell const &cell : row) {
      line.push_back(renderCell(cell));
    }
  }

  // Compute the width of each column.  The number of columns is the
  // largest number of cells in any line, including the titles.
  std::vector<std::size_t> columnWidths;
  auto accountForLine = [&columnWidths](RenderedLine const &line) -> void {
    if (columnWidths.size() < line.size()) {
      columnWidths.resize(line.size(), 0);
    }
    for (std::size_t c=0; c < line.size(); ++c) {
      columnWidths[c] = std::max(columnWidths[c], line[c].m_text.size());
    }
  };
  accountForLine(titleLine);
  for (RenderedLine const &line : dataLines) {
    accountForLine(line);
  }

  // Separator line, which fills every column with dashes.
  RenderedLine separatorLine;
  for (std::size_t width : columnWidths) {
    separatorLine.push_back(RenderedCell{std::string(width, '-'), false});
  }

  // Emit the lines.
  std::string ret;
  appendLine(ret, titleLine, columnWidths);
  appendLine(ret, separatorLine, columnWidths);
  for (RenderedLine const &line : dataLines) {
    appendLine(ret, line, columnWidths);
  }
  return ret;
}


CLOSE_NAMESPACE(smbase)


// EOF
