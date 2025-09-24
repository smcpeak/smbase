// gdvalue-kind-srcloc.h
// `GDValueKindSourceLocation`, a combined kind and source location.
// This is a private implementation detail of `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_KIND_SRCLOC_H
#define SMBASE_GDVALUE_KIND_SRCLOC_H

#include "gdvalue-kind-srcloc-fwd.h"   // fwds for this module

#include "gdvalue-kind.h"              // gdv::GDValueKind
#include "gdvalue-srcloc.h"            // gdv::GDValueSourceLocation

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::optional [n]
#include "smbase/std-string-fwd.h"     // std::string [n]

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::uint32_t
#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(gdv)


// Justifies using 4 bits for the kind.
static_assert(NUM_GDVALUE_KINDS <= 16);


/* This class is only meant to be used internally by `GDValue`.  It
   combines a `GDValueKind` with a `GDValueSourceLocation` into a
   single 64-bit object.

   The interface mirrors the corresponding methods on `GDValue` and
   `GDValueSourceLocation`.
*/
class GDValueKindSourceLocation {
public:      // constants
  // Saturation constants.
  static inline std::uint32_t c_saturatedFileAndLineValue =
    GDValueSourceLocation::c_saturatedFileAndLineValue;
  static inline std::uint32_t c_saturatedColumnValue =
    GDValueSourceLocation::c_saturatedColumnValue;

private:     // data
  // The kind.
  //
  // Note: This can't be given the type `GDValueKind` because the only
  // portable types for a bitfield are `unsigned int` and `signed int`.
  unsigned int m_kind : 4;

  // Either 0, meaning there is no location information, or the
  // file+line global index.  It could be `c_saturatedFileAndLineValue`.
  unsigned int m_fileAndLine : 28;

  // 1-based byte column number within its containing line.  It could be
  // `c_saturatedColumnValue`.
  //
  // Invariant: (m_fileAndLine==0) == (m_column==0)
  std::uint32_t m_column;

public:      // methods
  // Create with an absent source location.
  explicit GDValueKindSourceLocation(GDValueKind kind);

  // Create with a specified location.
  explicit GDValueKindSourceLocation(
    GDValueKind kind, GDValueSourceLocation loc);

  GDValueKindSourceLocation(GDValueKindSourceLocation const &obj);
  GDValueKindSourceLocation &operator=(GDValueKindSourceLocation const &obj);

  // Assert invariants.
  void selfCheck() const;

  // Comparison of this class is lexicographic: kind, loc, where an
  // absent location is less than any present location.
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(GDValueKindSourceLocation);

  // Write as "<kind> at noloc" or "<kind> at <loc>", where <kind> is
  // `toString(kind())` and <loc> is `sourceLocation().asString()`.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(
    std::ostream &os, GDValueKindSourceLocation const &obj)
  {
    obj.write(os); return os;
  }

  // Return what `write` writes.
  std::string asString() const;


  // ------------------------------ kind -------------------------------
  GDValueKind getKind() const
    { return static_cast<GDValueKind>(m_kind); }

  // Set the kind without affecting the location.
  void setKind(GDValueKind kind);


  // ---------------------------- location -----------------------------
  // True if this object has source location information.
  bool hasSourceLocation() const;

  // Get the location.
  //
  // Requires: hasSourceLocation()
  GDValueSourceLocation sourceLocation() const;

  // Get the location if we have one.
  std::optional<GDValueSourceLocation> sourceLocationOpt() const;

  // Remove a source location if we have one.
  void clearSourceLocation();

  // Set the location to `loc`.
  void setSourceLocation(GDValueSourceLocation loc);

  // Set it or clear it depending on `locOpt`.
  void setSourceLocationOpt(std::optional<GDValueSourceLocation> locOpt);
};


// Type size should be 64 bits.
static_assert(sizeof(GDValueKindSourceLocation) == 8);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_KIND_SRCLOC_H
