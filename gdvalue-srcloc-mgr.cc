// gdvalue-srcloc-mgr.cc
// Code for `gdvalue-srcloc-mgr` module.

#include "gdvalue-srcloc-mgr.h"        // this module

#include "smbase/chained-cond.h"       // smbase::cc::z_le_lt
#include "smbase/div-up.h"             // round_up
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/ordered-set.h"        // smbase::OrderedSet method impls
#include "smbase/sm-intcmp.h"          // smbase::intcmp_equal, etc.
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-test.h"            // PVAL (TEMPORARY)
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/xassert.h"            // xassertPrecondition

#include <string>                      // std::string

using namespace gdv;
using namespace smbase;


INIT_TRACE("gdvalue-srcloc-mgr");


OPEN_NAMESPACE(gdv)


GDValueSourceLocationManager::~GDValueSourceLocationManager()
{
  TRACE1("dtor: " << (void*)this);
}


GDValueSourceLocationManager::GDValueSourceLocationManager()
:
  m_fileNames(),
  m_asManager()
{
  TRACE1("ctor: " << (void*)this);
}


void GDValueSourceLocationManager::selfCheck() const
{
  localSelfCheck();

  m_fileNames.selfCheck();
  m_asManager.selfCheck();
}


void GDValueSourceLocationManager::localSelfCheck() const
{
  if (!intcmp_equal(m_asManager.numLocalSpaces(), m_fileNames.size())) {
    PVAL(m_asManager.numLocalSpaces());
    PVAL(m_fileNames.size());
  }
  xassert(intcmp_equal(m_asManager.numLocalSpaces(), m_fileNames.size()));
}


// ------------------------------ Queries ------------------------------
bool GDValueSourceLocationManager::validFileIndex(FileIndex index) const
{
  return cc::z_le_lt(index, m_fileNames.size());
}


std::string GDValueSourceLocationManager::fileNameForIndex(
  FileIndex index) const
{
  return m_fileNames.atC(index);
}


bool GDValueSourceLocationManager::validEncodedFileAndLine(
  EncodedFileAndLine encoded) const
{
  return cc::z_le_lt<EncodedFileAndLine>(
    encoded, m_asManager.globalSpaceSize());
}


auto GDValueSourceLocationManager::decodeFileAndLine(
  EncodedFileAndLine encoded) const
  -> std::pair<FileIndex, LineNumber>
{
  return m_asManager.globalToLocal(encoded);
}


auto GDValueSourceLocationManager::getEncodedFileAndLine(
  FileIndex fileIndex, LineNumber lineNumber) const
  -> EncodedFileAndLine
{
  xassertPrecondition(validFileIndex(fileIndex));

  return m_asManager.localToGlobal(fileIndex, lineNumber);
}


// --------------------------- Modifications ---------------------------
auto GDValueSourceLocationManager::fileIndexForName(
  std::string const &fname)
  -> FileIndex
{
  FileIndex ret = m_fileNames.insert(fname);

  if (intcmp_greater_equal(ret, m_asManager.numLocalSpaces())) {
    // This is a new file name.
    xassert(intcmp_equal(ret, m_asManager.numLocalSpaces()));

    // Allocate a space to go with it.
    auto vasid = m_asManager.allocateLocalSpace();
    xassert(intcmp_equal(vasid, ret));

    TRACE1_GDVN_EXPRS("fileIndexForName: allocated", fname, ret);

    localSelfCheck();
  }

  return ret;
}


/*static*/ auto GDValueSourceLocationManager::sizeToAccomodateLine(
  LineNumber const curSize,
  LineNumber const lineNumber)
  -> LineNumber
{
  // Crudely cap the line number to ~1 billion so the arithmetic below
  // cannot overflow.
  xassertPrecondition(lineNumber < 0x40000000);

  // Starting with 0x100 lines, double the amount of space up to
  // 0x10000, at which point we use the smallest multiple of 0x10000
  // that is larger than `lineNumber`.
  LineNumber newSize = curSize;
  while (!( lineNumber < newSize )) {
    if (newSize == 0) {
      newSize = 0x100;
    }
    else if (newSize >= 0x10000) {
      newSize = round_up<LineNumber>(lineNumber+1, 0x10000);
    }
    else {
      newSize *= 2;
    }
  }

  TRACE1_GDVN_EXPRS("sizeToAccomodateLine",
    curSize,
    lineNumber,
    newSize);

  return newSize;
}


auto GDValueSourceLocationManager::encodeFileAndLine(
  FileIndex const fileIndex,
  LineNumber const lineNumber)
  -> EncodedFileAndLine
{
  xassertPrecondition(validFileIndex(fileIndex));

  LineNumber const curSize = m_asManager.localSpaceSize(fileIndex);

  if (!( lineNumber < curSize )) {
    // Need to allocate more space.  Choose the new size.
    LineNumber const newSize =
      sizeToAccomodateLine(curSize, lineNumber);

    xassert(lineNumber < newSize);

    m_asManager.extendLocalSpace(fileIndex, newSize - curSize);

    TRACE1_GDVN_EXPRS("encodeFileAndLine: extended",
      fileIndex,
      fileNameForIndex(fileIndex),
      lineNumber,
      curSize,
      newSize);
  }

  return m_asManager.localToGlobal(fileIndex, lineNumber);
}


CLOSE_NAMESPACE(gdv)


// EOF
