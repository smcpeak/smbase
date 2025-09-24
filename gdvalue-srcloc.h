// gdvalue-srcloc.h
// `GDValueSourceLocation`, a source location for a `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SRCLOC_H
#define SMBASE_GDVALUE_SRCLOC_H

#include "gdvalue-srcloc-fwd.h"                  // fwds for this module

#include "smbase/compare-util-iface.h"           // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/gdvalue-srcloc-mgr-fwd.h"       // GDValueSourceLocationManager [n]
#include "smbase/ordered-map-fwd.h"              // smbase::OrderedMap
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"             // std::optional [n]
#include "smbase/std-vector-fwd.h"               // stdfwd::vector [n]

#include <cstddef>                               // std::size_t
#include <cstdint>                               // std::uint32_t
#include <iosfwd>                                // std::ostream [n]
#include <limits>                                // std::numeric_limits
#include <memory>                                // std::unique_ptr


OPEN_NAMESPACE(gdv)


/* A source location for a `GDValue`, as a 28-bit combined file and line
   number, and a 32-bit column number.

   Why this distribution of bits?  If the GDVN/JSON uses newlines and
   indentation then lines require more bits.  If not, then only the
   column needs a lot of bits.  This distribution tries to balance those
   two scenarios.  (It would be possible to use a "floating" divider,
   but I think that's overkill for now.)

   Not all `GDValue`s have them.  `GDValueReader` populates the
   locations as it reads, whereas `GDValue`s created directly do not
   have locations.

   The representation range is limited by what `GDValueKindLineColumn`
   can carry.  Values beyond that range "saturate" to the maximum value.
*/
class GDValueSourceLocation {
public:      // types
  // An index into the global file table, or 0 to mean "no file".
  using FileIndex = std::uint32_t;

  // A combined file+line global index.
  using FileAndLineNumber = std::uint32_t;

  // 1-based line number.
  using LineNumber = std::uint32_t;

  // A 1-based column number, in bytes, within its line.
  using ColumnNumber = std::uint32_t;

public:      // constants
  // The value of a "saturated" file+line number, meaning the true value
  // is at least this large, but the exact value has been lost.
  static inline FileAndLineNumber c_saturatedFileAndLineValue =
    ((1u << 28) - 1);

  // The value of a saturated byte offset.
  static inline ColumnNumber c_saturatedColumnValue =
    std::numeric_limits<std::uint32_t>::max();

  // The file index that represents the lack of a file name.
  static inline FileIndex c_nullFileIndex = 0;

private:     // class data
  // Global indexed file names.
  //
  // Invariant: Index 0 is reserved, and maps to the empty string, which
  // is used for locations that do not have associated files.
  static std::unique_ptr<GDValueSourceLocationManager> s_srclocMgr;

private:     // instance data
  // Either 0, for no info, or a file+line global index.
  //
  // Invariant: 0 < m_fileAndLine <= c_saturatedFileAndLineValue
  FileAndLineNumber m_fileAndLine;

  // 1-based byte column number within its line.
  //
  // Invariant: 0 < m_column <= c_saturatedColumnValue
  ColumnNumber m_column;

private:     // methods
  // Get the managar, creating it first if necessary.
  static GDValueSourceLocationManager *srclocMgr();

public:      // methods
  // Construct the given location without an associated file name.  If
  // `line` or `column` is too large, the corresponding field will
  // saturate.  This accepts `size_t` because that is a reasonably
  // natural type to work with in a reader class, and I want to handle
  // the saturation in just one place, namely here.
  //
  // Requires: line > 0 && column > 0
  GDValueSourceLocation(LineNumber line, ColumnNumber column);

  // Location with file index.  If `fileIndex==0`, it means the location
  // is not in any file, e.g., it came from a string literal.
  //
  // Requires: line > 0 && column > 0
  GDValueSourceLocation(
    FileIndex fileIndex,
    LineNumber line,
    ColumnNumber column);

  // Build with a previously encoded file+line.  For internal use.
  enum FileAndLineTag { FILE_AND_LINE };
  GDValueSourceLocation(
    FileAndLineTag,
    FileAndLineNumber fileAndLine,
    ColumnNumber column);

  GDValueSourceLocation(GDValueSourceLocation const &obj);
  GDValueSourceLocation &operator=(GDValueSourceLocation const &obj);

  // Assert invariants.
  void selfCheck() const;

  // Comparison of this class is lexicographic: file, line, byte.
  //
  // Files are ordered by numeric *index*, not their string values.
  // Absent compares as less than any present value.
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(GDValueSourceLocation);

  // ---------------------------- Line/col -----------------------------
  LineNumber line() const;
  ColumnNumber column() const { return m_column; }

  // True if either value is saturated.
  bool lineIsSaturated() const;
  bool columnIsSaturated() const;

  // --------------------------- File index ----------------------------
  // True if the file index is non-zero.
  bool hasFileIndex() const;

  // Get the file index.  This can be 0 to mean "no file".
  FileIndex fileIndex() const;

  // True if file+line info is saturated.
  bool fileIndexIsSaturated() const;

  // Combined index; only meant for internal use.
  FileAndLineNumber fileAndLineNumber() const;

  // ---------------------------- File name ----------------------------
  // Assert class data invariants.
  static void globalSelfCheck();

  // Get manager, read-only.
  static GDValueSourceLocationManager const *srclocMgrC();

  // Reset the map so no index is mapped other than 0.
  static void resetFileNameToIndex();

  // Number of mapped files, and hence valid file indices.
  static FileIndex numFileIndices();

  // Retrieve the index for `fname`, adding it to the table if needed.
  // If `fname.empty()`, returns 0.
  static FileIndex fileIndexOfName(std::string const &fname);

  // Get the name associated with `index`.  If `index==0`, returns the
  // empty string.
  //
  // Requires: 0 <= index < numFiles()
  static std::string fileNameOfIndex(FileIndex index);

  // Get the associated file name, which could be empty.
  std::string fileName() const;

  // -------------------------- Serialization --------------------------
  // Write as "<fileName()>:<line>:<column>" or "<line>:<column>".
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(
    std::ostream &os, GDValueSourceLocation const &obj)
  {
    obj.write(os); return os;
  }

  // Return what `write` writes.
  std::string asString() const;
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_SRCLOC_H
