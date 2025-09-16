// gdvalue-klb.h
// `GDValueKindLineByte`, a combined kind, line, and byte offset.  This
// is a private implementation detail of `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_KLB_H
#define SMBASE_GDVALUE_KLB_H

#include "gdvalue-klb-fwd.h"           // fwds for this module

#include "gdvalue-kind.h"              // GDValueKind

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-string-fwd.h"     // std::string [n]

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::uint32_t
#include <iosfwd>                      // std::ostream [n]
#include <limits>                      // std::numeric_limits


OPEN_NAMESPACE(gdv)


/* This class is only meant to be used internally by `GDValue`.  It
   encodes the `GDValueKind` and an optional input file location as a
   24-bit line number and 32-bit byte offset within the line.  The total
   size is 64 bits so it nicely fits into a `GDValue` alongside the
   data pointer union that class uses.

   This does not include information about which file the value came
   from.  The assumption is the client can keep track of that.
*/
class GDValueKindLineByte {
public:      // constants
  // The value of a "saturated" line number, meaning the true value is
  // at least this large, but the exact value has been lost.
  static inline std::uint32_t c_saturatedLineValue = ((1u << 24) - 1);

  // The value of a saturated byte offset.
  static inline std::uint32_t c_saturatedByteOffsetValue =
    std::numeric_limits<std::uint32_t>::max();

private:     // data
  // The kind.
  //
  // Note: This can't be given the type `GDValueKind` because the only
  // portable types for a bitfield are `unsigned int` and `signed int`.
  unsigned int m_kind : 8;

  // Either 0, meaning there is no location information, or the 1-based
  // line number.  It could be `c_saturatedLineValue`.
  unsigned int m_line : 24;

  // 0-based byte offset within its containing line.  It could be
  // `c_saturatedByteOffsetValue`.
  std::uint32_t m_byteOffset;

public:      // methods
  // Create with an absent source location.
  explicit GDValueKindLineByte(GDValueKind kind);

  // Create with a specified location.  If `line` or `byteOffset` is too
  // large for their respective storage, the saturated value is stored
  // instead.
  //
  // It is legal to pass 0 for `line`, meaning no location information,
  // but in that case `byteOffset` must also be 0.
  explicit GDValueKindLineByte(
    GDValueKind kind, std::size_t line, std::size_t byteOffset);

  // Assert invariants.
  void selfCheck() const;

  GDValueKind kind() const
    { return static_cast<GDValueKind>(m_kind); }

  // True if this object has source location information.
  bool hasLocation() const;

  // Extract the 1-based source location line number.
  //
  // Requires: hasLocation()
  std::size_t line() const;

  // True if the line value is saturated, such that it is merely a lower
  // bound on the true value.
  //
  // Requires: hasLocation()
  bool lineIsSaturated() const;

  // Extract the 0-based source location intraline byte offset.
  //
  // Requires: hasLocation()
  std::size_t byteOffset() const;

  // True if the byte offset is saturated.
  //
  // Requires: hasLocation()
  bool byteOffsetIsSaturated() const;

  // Comparison of this class is lexicographic: kind, line, byte.
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(GDValueKindLineByte);

  // Write as "(<kind> <line> <byteOffset>)", where <kind> is
  // `toString(kind())`.  This form is used whether or not this object
  // is considered to have a location.  This is meant primarily for
  // diagnostic purposes; this entire class is a private implementation
  // detail of `GDValue`.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(
    std::ostream &os, GDValueKindLineByte const &obj)
  {
    obj.write(os); return os;
  }

  // Return what `write` writes.
  std::string asString() const;
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_KLB_H
