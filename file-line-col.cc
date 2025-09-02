// file-line-col.cc
// Code for file-line-col.h.

#include "file-line-col.h"             // this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS, smbase::compare
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/stringb.h"            // stringb

#include <iostream>                    // std::ostream
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


FileLineCol::FileLineCol(std::optional<std::string> fileName,
                         int line,
                         int column,
                         std::size_t byteOffset) noexcept
  : m_fileName(std::move(fileName)),
    m_lc(line, column, byteOffset)
{}


FileLineCol::~FileLineCol()
{}


void FileLineCol::selfCheck() const
{
  m_lc.selfCheck();
}


int FileLineCol::compareTo(FileLineCol const &b) const
{
  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_fileName);
  RET_IF_COMPARE_MEMBERS(m_lc);
  return 0;
}


void FileLineCol::write(std::ostream &os) const
{
  if (m_fileName) {
    os << *m_fileName << ": ";
  }

  os << m_lc;
}


std::string FileLineCol::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(smbase)


// EOF
