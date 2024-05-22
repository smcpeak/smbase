// file-line-col.cc
// Code for file-line-col.h.

#include "file-line-col.h"             // this module



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
    // We should never be putting back a newline, and we cannot recover
    // the old column number.  Just use column 0 for this case.
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
  : m_fileName(fileName),
    m_lc(line, column)
{}


FileLineCol::~FileLineCol()
{}


// EOF
