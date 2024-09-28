// file-line-col.cc
// Code for file-line-col.h.

#include "file-line-col.h"             // this module

#include <utility>                     // std::move


// ------------------------------ LineCol ------------------------------
LineCol::LineCol(int line, int column) noexcept
  : m_line(line),
    m_column(column)
{}


void LineCol::incrementForChar(int c)
{
  if (c == '\n') {
    ++m_line;
    m_column = 1;
  }
  else {
    ++m_column;
  }
}


void LineCol::decrementColumn()
{
  if (m_column > 0) {
    --m_column;
  }
}


void LineCol::decrementForChar(int c)
{
  if (c == '\n') {
    // We put a newline back after seeing a symbol at the end of a
    // line.  Decrement the line number and clear the column, expecting
    // to restore them momentarily.
    --m_line;
    m_column = 0;
  }
  else {
    decrementColumn();
  }
}


// ---------------------------- FileLineCol ----------------------------
FileLineCol::FileLineCol(std::optional<std::string> fileName,
                         int line,
                         int column) noexcept
  : m_fileName(std::move(fileName)),
    m_lc(line, column)
{}


FileLineCol::~FileLineCol()
{}


// EOF
