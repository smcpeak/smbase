// gdvalue-srcloc.h
// `GDValueSourceLocation`, a source location for a `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SRCLOC_H
#define SMBASE_GDVALUE_SRCLOC_H

#include "gdvalue-srcloc-fwd.h"        // fwds for this module

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::uint32_t
#include <iosfwd>                      // std::ostream [n]
#include <limits>                      // std::numeric_limits


OPEN_NAMESPACE(gdv)


/* A source location for a `GDValue`, as a 24-bit line number and 32-bit
   column number.  This does not include information about which file
   the value came from.  The assumption is the client can keep track of
   that.

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
public:      // constants
  // The value of a "saturated" line number, meaning the true value is
  // at least this large, but the exact value has been lost.
  static inline std::uint32_t c_saturatedLineValue = ((1u << 24) - 1);

  // The value of a saturated byte offset.
  static inline std::uint32_t c_saturatedColumnValue =
    std::numeric_limits<std::uint32_t>::max();

private:     // data
  // 1-based line number.
  //
  // Invariant: 0 < m_line <= c_saturatedLineValue
  std::uint32_t m_line;

  // 1-based byte column number within its line.
  //
  // Invariant: 0 < m_column <= c_saturatedColumnValue
  std::uint32_t m_column;

public:      // methods
  // Construct the given location.  If `line` or `column` is too large,
  // the corresponding field will saturate.  This accepts `size_t`
  // because that is a reasonably natural type to work with in a reader
  // class, and I want to handle the saturation in just one place,
  // namely here.
  //
  // Requires: line > 0 && column > 0
  GDValueSourceLocation(std::size_t line, std::size_t column);

  GDValueSourceLocation(GDValueSourceLocation const &obj);
  GDValueSourceLocation &operator=(GDValueSourceLocation const &obj);

  // Assert invariants.
  void selfCheck() const;

  // Although the ctor accepts `size_t`, it is no secret that the
  // representation for each only has 32 bits, so that is what these
  // return.
  std::uint32_t line() const { return m_line; }
  std::uint32_t column() const { return m_column; }

  // True if either value is saturated.
  bool lineIsSaturated() const;
  bool columnIsSaturated() const;

  // Comparison of this class is lexicographic: line, byte.
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(GDValueSourceLocation);

  // Write as "<line>:<column>".
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
