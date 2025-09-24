// gdvalue-srcloc.cc
// Code for `gdvalue-srcloc` module.

#include "gdvalue-srcloc.h"            // this module

#include "smbase/chained-cond.h"       // smbase::cc::{z_lt_le,z_le_le,z_le_lt}
#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
#include "smbase/gdvalue-srcloc-mgr.h" // GDValueSourceLocationManager
#include "smbase/ordered-map.h"        // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, CMEMB
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassertPrecondition

#include <iostream>                    // std::ostream
#include <optional>                    // std::{nullopt,optional}
#include <string>                      // std::string
#include <utility>                     // std::move

using namespace smbase;


INIT_TRACE("gdvalue-srcloc");


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
  LineNumber line,
  ColumnNumber column)
:
  GDValueSourceLocation(c_nullFileIndex, line, column)
{}


GDValueSourceLocation::GDValueSourceLocation(
  FileIndex fileIndex,
  LineNumber line,
  ColumnNumber column)
:
  m_fileAndLine(
    possiblySaturated(
      srclocMgr()->encodeFileAndLine(fileIndex, line),
      c_saturatedFileAndLineValue)),
  m_column(possiblySaturated(column, c_saturatedColumnValue))
{
  xassertPrecondition(line > 0);
  xassertPrecondition(column > 0);

  selfCheck();
}


GDValueSourceLocation::GDValueSourceLocation(
  FileAndLineTag,
  FileAndLineNumber fileAndLine,
  ColumnNumber column)
:
  IMEMBFP(fileAndLine),
  IMEMBFP(column)
{
  selfCheck();
}


GDValueSourceLocation::GDValueSourceLocation(
  GDValueSourceLocation const &obj)
:
  DMEMB(m_fileAndLine),
  DMEMB(m_column)
{
  selfCheck();
}


GDValueSourceLocation &GDValueSourceLocation::operator=(GDValueSourceLocation const &obj)
{
  if (this != &obj) {
    CMEMB(m_fileAndLine);
    CMEMB(m_column);
    selfCheck();
  }
  return *this;
}


void GDValueSourceLocation::selfCheck() const
{
  xassert(cc::z_le_le(m_fileAndLine, c_saturatedFileAndLineValue));
  xassert(cc::z_lt_le(m_column, c_saturatedColumnValue));
}


int GDValueSourceLocation::compareTo(GDValueSourceLocation const &b) const
{
  using smbase::compare;

  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_fileAndLine);
  RET_IF_COMPARE_MEMBERS(m_column);
  return 0;
}


// ----------------------------- Line/col ------------------------------
auto GDValueSourceLocation::line() const -> LineNumber
{
  return srclocMgrC()->decodeFileAndLine(m_fileAndLine).second;
}


bool GDValueSourceLocation::lineIsSaturated() const
{
  return m_fileAndLine == c_saturatedFileAndLineValue;
}


bool GDValueSourceLocation::columnIsSaturated() const
{
  return m_column == c_saturatedColumnValue;
}


// ---------------------------- File index -----------------------------
bool GDValueSourceLocation::hasFileIndex() const
{
  return fileIndex() != 0;
}


auto GDValueSourceLocation::fileIndex() const -> FileIndex
{
  return srclocMgrC()->decodeFileAndLine(m_fileAndLine).first;
}


bool GDValueSourceLocation::fileIndexIsSaturated() const
{
  return m_fileAndLine == c_saturatedFileAndLineValue;
}


auto GDValueSourceLocation::fileAndLineNumber() const
  -> FileAndLineNumber
{
  return m_fileAndLine;
}


// ----------------------------- File name -----------------------------
std::unique_ptr<GDValueSourceLocationManager>
  GDValueSourceLocation::s_srclocMgr;


/*static*/ void GDValueSourceLocation::globalSelfCheck()
{
  if (!s_srclocMgr) {
    return;
  }

  s_srclocMgr->selfCheck();
  xassert(s_srclocMgr->fileNameForIndex(0) == "");
}


/*static*/ GDValueSourceLocationManager *
GDValueSourceLocation::srclocMgr()
{
  if (!s_srclocMgr) {
    s_srclocMgr.reset(new GDValueSourceLocationManager);
    FileIndex fi = s_srclocMgr->fileIndexForName("");
    xassert(fi == c_nullFileIndex);

    TRACE1("Created s_srclocMgr");
    globalSelfCheck();
  }

  return s_srclocMgr.get();
}


/*static*/ GDValueSourceLocationManager const *
GDValueSourceLocation::srclocMgrC()
{
  return srclocMgr();
}


/*static*/ void GDValueSourceLocation::resetFileNameToIndex()
{
  s_srclocMgr.reset();

  TRACE1("Reset s_srclocMgr");
  globalSelfCheck();
}


/*static*/ auto GDValueSourceLocation::numFileIndices() -> FileIndex
{
  return srclocMgrC()->numFiles();
}


/*static*/ GDValueSourceLocation::FileIndex
GDValueSourceLocation::fileIndexOfName(
  std::string const &fname)
{
  return srclocMgr()->fileIndexForName(fname);
}


/*static*/ std::string GDValueSourceLocation::fileNameOfIndex(
  FileIndex index)
{
  xassertPrecondition(cc::z_le_lt(index, numFileIndices()));

  return srclocMgr()->fileNameForIndex(index);
}


std::string GDValueSourceLocation::fileName() const
{
  return srclocMgrC()->fileNameForIndex(fileIndex());
}


// --------------------------- Serialization ---------------------------
void GDValueSourceLocation::write(std::ostream &os) const
{
  if (auto name = fileName(); !name.empty()) {
    os << name << ':';
  }
  os << line() << ':' << column();
}


std::string GDValueSourceLocation::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(gdv)


// EOF
