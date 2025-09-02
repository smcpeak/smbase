// line-col.cc
// Code for `line-col` module.

#include "line-col.h"                  // this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS, smbase::compare
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert

#include <iostream>                    // std::ostream

using namespace smbase;


LineCol::LineCol(int line, int column, std::size_t byteOffset) noexcept
  : m_line(line),
    m_column(column),
    m_byteOffset(byteOffset)
{}


void LineCol::selfCheck() const
{
  xassert(m_line >= 1);
  xassert(m_column >= 0);
  xassert(m_byteOffset >= 0);
}


int LineCol::compareTo(LineCol const &b) const
{
  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_line);
  RET_IF_COMPARE_MEMBERS(m_column);
  RET_IF_COMPARE_MEMBERS(m_byteOffset);
  return 0;
}


void LineCol::write(std::ostream &os) const
{
  os << m_line << ':' << m_column;
}


std::string LineCol::asString() const
{
  return stringb(*this);
}


void LineCol::incrementForChar(int c)
{
  if (c == '\n') {
    ++m_line;
    m_column = 1;
  }
  else {
    ++m_column;
  }

  ++m_byteOffset;
}


void LineCol::decrementColumn()
{
  if (m_column > 0) {
    --m_column;
  }

  --m_byteOffset;
}


void LineCol::decrementForChar(int c)
{
  if (c == '\n') {
    // We put a newline back after seeing a symbol at the end of a
    // line.  Decrement the line number and clear the column, expecting
    // to restore them momentarily.
    --m_line;
    m_column = 0;
    --m_byteOffset;
  }
  else {
    decrementColumn();
  }
}


// EOF
