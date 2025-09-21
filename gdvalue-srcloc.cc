// gdvalue-srcloc.cc
// Code for `gdvalue-srcloc` module.

#include "gdvalue-srcloc.h"            // this module

#include "smbase/chained-cond.h"       // smbase::cc::{z_lt_le,z_le_le}
#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
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
  std::size_t line,
  std::size_t column)
:
  GDValueSourceLocation(std::nullopt, line, column)
{}


GDValueSourceLocation::GDValueSourceLocation(
  FileIndexOpt fileIndexOpt,
  std::size_t line,
  std::size_t column)
:
  m_fileIndex(fileIndexOpt?
    possiblySaturated(*fileIndexOpt, c_saturatedFileIndexValue) :
    0),
  m_line(possiblySaturated(line, c_saturatedLineValue)),
  m_column(possiblySaturated(column, c_saturatedColumnValue))
{
  if (fileIndexOpt) {
    xassertPrecondition(*fileIndexOpt > 0);
  }
  xassertPrecondition(line > 0);
  xassertPrecondition(column > 0);

  selfCheck();
}


GDValueSourceLocation::GDValueSourceLocation(
  GDValueSourceLocation const &obj)
:
  DMEMB(m_fileIndex),
  DMEMB(m_line),
  DMEMB(m_column)
{
  selfCheck();
}


GDValueSourceLocation &GDValueSourceLocation::operator=(GDValueSourceLocation const &obj)
{
  if (this != &obj) {
    CMEMB(m_fileIndex);
    CMEMB(m_line);
    CMEMB(m_column);
    selfCheck();
  }
  return *this;
}


void GDValueSourceLocation::selfCheck() const
{
  xassert(cc::z_le_le(m_fileIndex, c_saturatedFileIndexValue));
  xassert(cc::z_lt_le(m_line, c_saturatedLineValue));
  xassert(cc::z_lt_le(m_column, c_saturatedColumnValue));
}


int GDValueSourceLocation::compareTo(GDValueSourceLocation const &b) const
{
  using smbase::compare;

  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_fileIndex);
  RET_IF_COMPARE_MEMBERS(m_line);
  RET_IF_COMPARE_MEMBERS(m_column);
  return 0;
}


// ----------------------------- Line/col ------------------------------
bool GDValueSourceLocation::lineIsSaturated() const
{
  return m_line == c_saturatedLineValue;
}


bool GDValueSourceLocation::columnIsSaturated() const
{
  return m_column == c_saturatedColumnValue;
}


// ---------------------------- File index -----------------------------
GDValueSourceLocation::FileIndex
GDValueSourceLocation::fileIndexOrZero() const
{
  return m_fileIndex;
}


bool GDValueSourceLocation::hasFileIndex() const
{
  return m_fileIndex > 0;
}


GDValueSourceLocation::FileIndex
GDValueSourceLocation::fileIndex() const
{
  xassertPrecondition(hasFileIndex());
  return m_fileIndex;
}


GDValueSourceLocation::FileIndexOpt
GDValueSourceLocation::fileIndexOpt() const
{
  if (hasFileIndex()) {
    return m_fileIndex;
  }
  else {
    return std::nullopt;
  }
}


bool GDValueSourceLocation::fileIndexIsSaturated() const
{
  return m_fileIndex == c_saturatedFileIndexValue;
}


// ----------------------------- File name -----------------------------
std::unique_ptr<GDValueSourceLocation::FileNameToIndexMap>
  GDValueSourceLocation::s_fileNameToIndex;


/*static*/ void GDValueSourceLocation::globalSelfCheck()
{
  if (!s_fileNameToIndex) {
    return;
  }

  xassert(s_fileNameToIndex->valueAtKey("") == 0);

  for (std::size_t i=0; i < s_fileNameToIndex->size(); ++i) {
    xassert(s_fileNameToIndex->entryAtIndex(i).second == i);
  }
}


/*static*/ GDValueSourceLocation::FileNameToIndexMap *
GDValueSourceLocation::fileNameToIndex()
{
  if (!s_fileNameToIndex) {
    s_fileNameToIndex.reset(new FileNameToIndexMap);
    s_fileNameToIndex->insert({"", 0});

    TRACE1("Created s_fileNameToIndex");
    globalSelfCheck();
  }

  return s_fileNameToIndex.get();
}


/*static*/ GDValueSourceLocation::FileNameToIndexMap const *
GDValueSourceLocation::fileNameToIndexC()
{
  return fileNameToIndex();
}


/*static*/ void GDValueSourceLocation::resetFileNameToIndex()
{
  s_fileNameToIndex.reset();

  TRACE1("Reset s_fileNameToIndex");
  globalSelfCheck();
}


/*static*/ GDValueSourceLocation::FileIndex
GDValueSourceLocation::fileIndexOfName(
  std::string const &fname)
{
  xassertPrecondition(!fname.empty());

  auto &map = *(fileNameToIndex());

  if (map.contains(fname)) {
    return map.valueAtKey(fname);
  }
  else {
    FileIndex nextIndex = map.size();

    bool inserted = map.insert({fname, nextIndex});
    xassert(inserted);

    auto const &entry = map.entryAtIndex(nextIndex);
    xassert(entry.first == fname);
    xassert(entry.second == nextIndex);

    TRACE1("mapped index " << nextIndex << " to file: " << fname);

    globalSelfCheck();

    return nextIndex;
  }
}


/*static*/ GDValueSourceLocation::FileIndexOpt
GDValueSourceLocation::fileIndexOfNameOpt(
  std::optional<std::string> const &fnameOpt)
{
  if (fnameOpt) {
    return fileIndexOfName(*fnameOpt);
  }
  else {
    return std::nullopt;
  }
}


/*static*/ std::optional<std::string>
GDValueSourceLocation::fileNameOptOfIndex(FileIndex index)
{
  xassertPrecondition(index > 0);

  auto const &map = *(fileNameToIndex());

  if (index < map.size()) {
    return map.entryAtIndex(index).first;
  }
  else {
    return std::nullopt;
  }
}


std::optional<std::string> GDValueSourceLocation::fileNameOpt() const
{
  if (hasFileIndex()) {
    return fileNameOptOfIndex(fileIndex());
  }
  else {
    return std::nullopt;
  }
}


std::optional<std::string>
GDValueSourceLocation::fileNameOrExplanationOpt() const
{
  if (!hasFileIndex()) {
    return std::nullopt;
  }

  if (fileIndexIsSaturated()) {
    return "(Saturated FileIndex)";
  }

  if (auto nameOpt = fileNameOptOfIndex(fileIndex())) {
    return *nameOpt;
  }
  else {
    return std::make_optional<std::string>(stringb(
      "(FileIndex " << fileIndex() << ")"));
  }
}


// --------------------------- Serialization ---------------------------
void GDValueSourceLocation::write(std::ostream &os) const
{
  if (auto nameOpt = fileNameOrExplanationOpt()) {
    os << *nameOpt << ':';
  }
  os << m_line << ':' << m_column;
}


std::string GDValueSourceLocation::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(gdv)


// EOF
