// gdvalue-kind-srcloc.cc
// Code for `gdvalue-kind-srcloc` module.

#include "gdvalue-kind-srcloc.h"       // this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, CMEMB
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassertPrecondition

#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(gdv)


GDValueKindSourceLocation::GDValueKindSourceLocation(GDValueKind kind)
:
  m_kind(static_cast<unsigned>(kind)),
  m_fileIndex(0),
  m_line(0),
  m_column(0)
{}


GDValueKindSourceLocation::GDValueKindSourceLocation(
  GDValueKind kind,
  GDValueSourceLocation loc)
:
  m_kind(static_cast<unsigned>(kind)),
  m_fileIndex(loc.fileIndexOrZero()),
  m_line(loc.line()),
  m_column(loc.column())
{
  selfCheck();
}


GDValueKindSourceLocation::GDValueKindSourceLocation(
  GDValueKindSourceLocation const &obj)
:
  DMEMB(m_kind),
  DMEMB(m_fileIndex),
  DMEMB(m_line),
  DMEMB(m_column)
{}


GDValueKindSourceLocation &GDValueKindSourceLocation::operator=(
  GDValueKindSourceLocation const &obj)
{
  if (this != &obj) {
    CMEMB(m_kind);
    CMEMB(m_fileIndex);
    CMEMB(m_line);
    CMEMB(m_column);
  }
  return *this;
}


void GDValueKindSourceLocation::selfCheck() const
{
  xassert(m_kind < NUM_GDVALUE_KINDS);
  if (m_line == 0) {
    xassert(m_fileIndex == 0);
  }
  xassert((m_line==0) == (m_column==0));
}


int GDValueKindSourceLocation::compareTo(GDValueKindSourceLocation const &b) const
{
  using smbase::compare;

  auto const &a = *this;

  RET_IF_COMPARE_MEMBERS(m_kind);

  RET_IF_COMPARE_MEMBERS(m_fileIndex);

  // Since absent is represented by 0, which is less than any present
  // line number, this will ensure that absent compares as less than any
  // present location.
  RET_IF_COMPARE_MEMBERS(m_line);

  RET_IF_COMPARE_MEMBERS(m_column);

  return 0;
}


void GDValueKindSourceLocation::write(std::ostream &os) const
{
  os << toString(getKind()) << " at ";

  if (hasSourceLocation()) {
    os << sourceLocation();
  }
  else {
    os << "noloc";
  }
}


std::string GDValueKindSourceLocation::asString() const
{
  return stringb(*this);
}


// ------------------------------- kind --------------------------------
void GDValueKindSourceLocation::setKind(GDValueKind kind)
{
  m_kind = kind;
  selfCheck();
}


// ----------------------------- location ------------------------------
bool GDValueKindSourceLocation::hasSourceLocation() const
{
  return m_line != 0;
}


GDValueSourceLocation GDValueKindSourceLocation::sourceLocation() const
{
  xassertPrecondition(hasSourceLocation());

  return GDValueSourceLocation(
    m_fileIndex? std::make_optional(m_fileIndex) : std::nullopt,
    m_line,
    m_column);
}


std::optional<GDValueSourceLocation> GDValueKindSourceLocation::sourceLocationOpt() const
{
  if (hasSourceLocation()) {
    return sourceLocation();
  }
  else {
    return std::nullopt;
  }
}


void GDValueKindSourceLocation::clearSourceLocation()
{
  m_fileIndex = 0;
  m_line = 0;
  m_column = 0;
  selfCheck();
}


void GDValueKindSourceLocation::setSourceLocation(
  GDValueSourceLocation loc)
{
  m_fileIndex = loc.fileIndexOrZero();
  m_line = loc.line();
  m_column = loc.column();
  selfCheck();
}


void GDValueKindSourceLocation::setSourceLocationOpt(
  std::optional<GDValueSourceLocation> locOpt)
{
  if (locOpt) {
    setSourceLocation(*locOpt);
  }
  else {
    clearSourceLocation();
  }
  selfCheck();
}


CLOSE_NAMESPACE(gdv)


// EOF
