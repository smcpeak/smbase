// gdvalue-klb.cc
// Code for `gdvalue-klb` module.

#include "gdvalue-klb.h"               // this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassertPrecondition

#include <iostream>                    // std::ostream
#include <string>                      // std::string


OPEN_NAMESPACE(gdv)


// Type size should be 64 bits.
static_assert(sizeof(GDValueKindLineByte) == 8);

// The `kind` must fit into 8 bits.
static_assert(NUM_GDVALUE_KINDS <= 256);


GDValueKindLineByte::GDValueKindLineByte(GDValueKind kind)
  : GDValueKindLineByte(kind, 0, 0)
{}


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


GDValueKindLineByte::GDValueKindLineByte(
  GDValueKind kind, std::size_t line, std::size_t byteOffset)
:
  m_kind(static_cast<unsigned>(kind)),
  m_line(possiblySaturated(line, c_saturatedLineValue)),
  m_byteOffset(possiblySaturated(byteOffset, c_saturatedByteOffsetValue))
{
  xassertPrecondition(kind < NUM_GDVALUE_KINDS);

  if (line == 0) {
    xassertPrecondition(byteOffset == 0);
  }

  selfCheck();
}


void GDValueKindLineByte::selfCheck() const
{
  xassert(m_kind < NUM_GDVALUE_KINDS);

  if (m_line == 0) {
    xassert(m_byteOffset == 0);
  }
}


bool GDValueKindLineByte::hasLocation() const
{
  return m_line != 0;
}


std::size_t GDValueKindLineByte::line() const
{
  xassertPrecondition(hasLocation());

  return m_line;
}


bool GDValueKindLineByte::lineIsSaturated() const
{
  xassertPrecondition(hasLocation());

  return m_line == c_saturatedLineValue;
}


std::size_t GDValueKindLineByte::byteOffset() const
{
  xassertPrecondition(hasLocation());

  return m_byteOffset;
}


bool GDValueKindLineByte::byteOffsetIsSaturated() const
{
  xassertPrecondition(hasLocation());

  return m_byteOffset == c_saturatedByteOffsetValue;
}


int GDValueKindLineByte::compareTo(GDValueKindLineByte const &b) const
{
  using smbase::compare;

  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_kind);
  RET_IF_COMPARE_MEMBERS(m_line);
  RET_IF_COMPARE_MEMBERS(m_byteOffset);
  return 0;
}


void GDValueKindLineByte::write(std::ostream &os) const
{
  os << "(" << toString(kind())
     << " " << m_line
     << " " << m_byteOffset
     << ")";
}


std::string GDValueKindLineByte::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(gdv)


// EOF
