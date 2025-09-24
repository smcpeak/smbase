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
  m_fileAndLine(0),
  m_column(0)
{}


GDValueKindSourceLocation::GDValueKindSourceLocation(
  GDValueKind kind,
  GDValueSourceLocation loc)
:
  m_kind(static_cast<unsigned>(kind)),
  m_fileAndLine(loc.fileAndLineNumber()),
  m_column(loc.column())
{
  selfCheck();
}


GDValueKindSourceLocation::GDValueKindSourceLocation(
  GDValueKindSourceLocation const &obj)
:
  DMEMB(m_kind),
  DMEMB(m_fileAndLine),
  DMEMB(m_column)
{}


GDValueKindSourceLocation &GDValueKindSourceLocation::operator=(
  GDValueKindSourceLocation const &obj)
{
  if (this != &obj) {
    CMEMB(m_kind);
    CMEMB(m_fileAndLine);
    CMEMB(m_column);
  }
  return *this;
}


void GDValueKindSourceLocation::selfCheck() const
{
  xassert(m_kind < NUM_GDVALUE_KINDS);
  xassert((m_fileAndLine==0) == (m_column==0));
}


int GDValueKindSourceLocation::compareTo(GDValueKindSourceLocation const &b) const
{
  using smbase::compare;

  auto const &a = *this;

  RET_IF_COMPARE_MEMBERS(m_kind);

  // Since absent is represented by 0, which is less than any present
  // line number, this will ensure that absent compares as less than any
  // present location.
  RET_IF_COMPARE_MEMBERS(m_fileAndLine);

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
  return m_fileAndLine != 0;
}


GDValueSourceLocation GDValueKindSourceLocation::sourceLocation() const
{
  xassertPrecondition(hasSourceLocation());

  return GDValueSourceLocation(
    GDValueSourceLocation::FILE_AND_LINE,
    m_fileAndLine,
    m_column);
}


std::optional<GDValueSourceLocation>
GDValueKindSourceLocation::sourceLocationOpt() const
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
  m_fileAndLine = 0;
  m_column = 0;
  selfCheck();
}


void GDValueKindSourceLocation::setSourceLocation(
  GDValueSourceLocation loc)
{
  m_fileAndLine = loc.fileAndLineNumber();
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
