// gdvalue-srcloc-mgr.h
// `GDValueSourceLocationManager`, which encodes and decodes
// `GDValueSourceLocation`s.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SRCLOC_MGR_H
#define SMBASE_GDVALUE_SRCLOC_MGR_H

#include "gdvalue-srcloc-mgr-fwd.h"              // fwds for this module

#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue [n]
#include "smbase/ordered-set-iface.h"            // smbase::OrderedSet
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/std-string-fwd.h"               // std::string_view [n]
#include "smbase/std-utility-fwd.h"              // std::pair [n]
#include "smbase/virtual-address-space.h"        // smbase::VirtualASManager

#include <cstdint>                               // std::uint32_t


OPEN_NAMESPACE(gdv)


/* As a first step, this implements a map between a 24-bit number and
   the corresponding file name and line number.

   TODO: Expand the scope to handle column numbers and byte offsets too.
*/
class GDValueSourceLocationManager {
public:      // types
  // An index for a known file name.
  using FileIndex = std::uint32_t;

  // A line number.  This class is agnostic as to whether the first line
  // number is 0 or 1; even if only line 1 is initially encoded, space
  // to encode a line 0 is always reserved.
  using LineNumber = std::uint32_t;

  // A number that encodes a file index and line number.
  using EncodedFileAndLine = std::uint32_t;

private:     // data
  // Map between file names and indices.
  smbase::OrderedSet<std::string, FileIndex> m_fileNames;

  // 2D to 1D encoder.
  //
  // Invariant: m_asManager.numLocalSpaces() == m_fileNames.size()
  smbase::VirtualASManager m_asManager;

private:     // methods
  // Given that `curSize` is inadequate to contain `lineNumber`, select
  // a new, larger size that can.
  static LineNumber sizeToAccomodateLine(
    LineNumber const curSize,
    LineNumber const lineNumber);

public:      // methods
  ~GDValueSourceLocationManager();

  // Empty map.
  GDValueSourceLocationManager();

  // Assert invariants.
  void selfCheck() const;

  // Bounded subset of `selfCheck`.
  void localSelfCheck() const;

  // ----------------------------- Queries -----------------------------
  // True if `index` was previously returned by `fileIndexForName`.
  bool validFileIndex(FileIndex index) const;

  // Get the name associated with `index`.
  //
  // Requires: validFileIndex(index)
  std::string fileNameForIndex(FileIndex index) const;

  // True if `encoded` can be decoded, which is true of everything that
  // has been directly encoded, and potentially also of some other
  // values, but that should not be relied upon.
  bool validEncodedFileAndLine(EncodedFileAndLine encoded) const;

  // Decode a previously encoded pair.
  //
  // Requires: validEncodedFileAndLine(encoded)
  std::pair<FileIndex, LineNumber> decodeFileAndLine(
    EncodedFileAndLine encoded) const;

  // Get the encoding for the given pair, which either has previously
  // been encoded, or (less reliably) other nearby locations have been
  // encoded such that this one is valid too.
  //
  // Requires: validFileIndex(index)
  EncodedFileAndLine getEncodedFileAndLine(
    FileIndex fileIndex, LineNumber lineNumber) const;

  // -------------------------- Modifications --------------------------
  // Get the index for `fname`, allocating a new one if needed.  The
  // name is simply treated as an opaque string; this class does not
  // access the file system.
  FileIndex fileIndexForName(std::string const &fname);

  // Encode the given pair.
  //
  // Requires: validFileIndex(index)
  EncodedFileAndLine encodeFileAndLine(
    FileIndex fileIndex, LineNumber lineNumber);
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_SRCLOC_MGR_H
