// gdvalue-srcloc.h
// `GDValueSourceLocation`, a source location for a `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SRCLOC_H
#define SMBASE_GDVALUE_SRCLOC_H

#include "gdvalue-srcloc-fwd.h"        // fwds for this module

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/ordered-map-fwd.h"    // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::optional [n]
#include "smbase/std-vector-fwd.h"     // stdfwd::vector [n]

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::uint32_t
#include <iosfwd>                      // std::ostream [n]
#include <limits>                      // std::numeric_limits
#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(gdv)


/* A source location for a `GDValue`, as a 20-bit line number, a 32-bit
   column number, and an optional 8-bit file index.

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
  // An index into the global file table.
  using FileIndex = std::uint32_t;

  using FileIndexOpt = std::optional<FileIndex>;

  // Map from file name to `FileIndex`, and also the inverse in the form
  // of the extrinsic order.
  //
  // Invariant: Index 0 is reserved, and maps to the empty string.
  //
  // Invariant: The index to which every string is mapped is its index
  // in the extrinsic order.
  using FileNameToIndexMap = smbase::OrderedMap<std::string, FileIndex>;

public:      // constants
  // Saturated file index.
  static inline FileIndex c_saturatedFileIndexValue =
    ((1u << 8) - 1);

  // The value of a "saturated" line number, meaning the true value is
  // at least this large, but the exact value has been lost.
  static inline std::uint32_t c_saturatedLineValue =
    ((1u << 20) - 1);

  // The value of a saturated byte offset.
  static inline std::uint32_t c_saturatedColumnValue =
    std::numeric_limits<std::uint32_t>::max();

private:     // class data
  // Global indexed file names.
  static std::unique_ptr<FileNameToIndexMap> s_fileNameToIndex;

private:     // instance data
  // Either 0, for no file info, or the positive index for the source
  // file name.
  //
  // Invariant: 0 <= m_fileIndex <= c_saturatedFileValue
  FileIndex m_fileIndex;

  // 1-based line number.
  //
  // Invariant: 0 < m_line <= c_saturatedLineValue
  std::uint32_t m_line;

  // 1-based byte column number within its line.
  //
  // Invariant: 0 < m_column <= c_saturatedColumnValue
  std::uint32_t m_column;

private:     // methods
  // Get the current map, creating it first if necessary.
  static FileNameToIndexMap *fileNameToIndex();

public:      // methods
  // Construct the given location.  If `line` or `column` is too large,
  // the corresponding field will saturate.  This accepts `size_t`
  // because that is a reasonably natural type to work with in a reader
  // class, and I want to handle the saturation in just one place,
  // namely here.
  //
  // Requires: line > 0 && column > 0
  GDValueSourceLocation(std::size_t line, std::size_t column);

  // Location with file index.
  //
  // Requires: if fileIndexOpt, *fileIndexOpt > 0
  // Requires: line > 0 && column > 0
  GDValueSourceLocation(
    FileIndexOpt fileIndexOpt,
    std::size_t line,
    std::size_t column);

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
  // Although the ctor accepts `size_t`, in part to enable saturation to
  // be done inside the ctor, it is no secret that the representation
  // for each only has 32 bits (or less), so that is what these return.
  std::uint32_t line() const { return m_line; }
  std::uint32_t column() const { return m_column; }

  // True if either value is saturated.
  bool lineIsSaturated() const;
  bool columnIsSaturated() const;

  // --------------------------- File index ----------------------------
  // File index, or 0 for no info.  This is a low-level function meant
  // for use by `GDValueKindSourceLocation`.
  FileIndex fileIndexOrZero() const;

  // True if we have file info.
  bool hasFileIndex() const;

  // Requires: hasFileIndex()
  FileIndex fileIndex() const;

  FileIndexOpt fileIndexOpt() const;

  // True if we have a file index and it is saturated.
  bool fileIndexIsSaturated() const;

  // ---------------------------- File name ----------------------------
  // Assert class data invariants.
  static void globalSelfCheck();

  // Get current map.
  static FileNameToIndexMap const *fileNameToIndexC();

  // Reset the map so no index is mapped.
  static void resetFileNameToIndex();

  // Retrieve the index for `fname`, adding it to the table if needed.
  //
  // Requires: !fname.empty()
  static FileIndex fileIndexOfName(std::string const &fname);

  // If `fname` is nullopt, return nullopt.  Otherwise map it to an
  // index and return that.  This is somewhat inefficient.
  static FileIndexOpt fileIndexOfNameOpt(
    std::optional<std::string> const &fnameOpt);

  // Get the name associated with `index`, if any.
  //
  // Requires: index > 0
  static std::optional<std::string> fileNameOptOfIndex(FileIndex index);

  // If this object has an unsaturated file index, and it is mapped to a
  // name, return that name.
  std::optional<std::string> fileNameOpt() const;

  /* If this object has no file index:

       nullopt

     If this has a saturated file index:

       "(Saturated FileIndex)"

     If this has an unmapped file index:

       "(FileIndex <n>)"

     Otherwse, return the file name.
  */
  std::optional<std::string> fileNameOrExplanationOpt() const;

  // -------------------------- Serialization --------------------------
  // Write as "<fileNameOrExplanationOpt().value()>:<line>:<column>" or
  // "<line>:<column>".
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
