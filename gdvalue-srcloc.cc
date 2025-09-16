// gdvalue-srcloc.cc
// Code for `gdvalue-srcloc` module.

#include "gdvalue-srcloc.h"            // this module

#include "smbase/chained-cond.h"       // smbase::cc::z_lt_le
#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, CMEMB
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassertPrecondition

#include <iostream>                    // std::ostream
#include <string>                      // std::string

using namespace smbase;


OPEN_NAMESPACE(gdv)


// Return `value`, unless it is greater than `saturated`, in which case
// return the latter.
static std::uint32_t possiblySaturated(
  std::size_t value,
  std::uint32_t saturated)
{
  if (value > saturated) {
    return saturated;
  }
  else {
    return value;
  }
}


GDValueSourceLocation::GDValueSourceLocation(
  std::size_t line,
  std::size_t column)
:
  m_line(possiblySaturated(line, c_saturatedLineValue)),
  m_column(possiblySaturated(column, c_saturatedColumnValue))
{
  xassertPrecondition(line > 0);
  xassertPrecondition(column > 0);

  selfCheck();
}


GDValueSourceLocation::GDValueSourceLocation(
  GDValueSourceLocation const &obj)
:
  DMEMB(m_line),
  DMEMB(m_column)
{
  selfCheck();
}


GDValueSourceLocation &GDValueSourceLocation::operator=(GDValueSourceLocation const &obj)
{
  if (this != &obj) {
    CMEMB(m_line);
    CMEMB(m_column);
    selfCheck();
  }
  return *this;
}


void GDValueSourceLocation::selfCheck() const
{
  xassert(cc::z_lt_le(m_line, c_saturatedLineValue));
  xassert(cc::z_lt_le(m_column, c_saturatedColumnValue));
}


bool GDValueSourceLocation::lineIsSaturated() const
{
  return m_line == c_saturatedLineValue;
}


bool GDValueSourceLocation::columnIsSaturated() const
{
  return m_column == c_saturatedColumnValue;
}


int GDValueSourceLocation::compareTo(GDValueSourceLocation const &b) const
{
  using smbase::compare;

  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_line);
  RET_IF_COMPARE_MEMBERS(m_column);
  return 0;
}


void GDValueSourceLocation::write(std::ostream &os) const
{
  os << m_line << ':' << m_column;
}


std::string GDValueSourceLocation::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(gdv)


// EOF
