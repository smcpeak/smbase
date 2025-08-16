// gdv-binary64-float.h
// `GDVBinary64Float`, IEEE 754 binary64 floating point number.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDV_BINARY64_FLOAT_H
#define SMBASE_GDV_BINARY64_FLOAT_H

#include "smbase/compare-util-iface.h"           // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/std-string-fwd.h"               // std::string
#include "smbase/std-string-view-fwd.h"          // std::string

#include <iosfwd>                                // std::ostream

OPEN_NAMESPACE(gdv)


// This is the data stored in `GDValue` when the kind is
// `GDVK_BINARY64_FLOAT`.  This type represents any finite (not Nan or
// Infinity) IEEE 754 binary64 floating point number.
class GDVBinary64Float {
private:     // data
  // Assuming we are compiling on a machine where `double` meets the
  // stated requirements, this suffices.  On some other machine, the
  // intention is this class implementation (including data) would
  // change but the interface would remain the same.
  //
  // Invariant: std::isfinite(m_value)
  double m_value;

public:      // methods
  // Default-initialize to +0.
  GDVBinary64Float();

  /* Commentary on some of the generated methods:

     Construction from a `double` requires `std::isfinite(value)`.

     The primary ctors accept references, including an rvalue reference,
     to `double` simply because that is what `create-tuple-class.py`
     does uniformly.

     The +move option is here to ensure that this class has move ops for
     uniformity with other GDV data classes, and to prepare for a
     possible alternative implementation, even though they don't
     accomplish anything if `m_value` is `double`.

     The `write` method expresses the value using JSON/GDVN syntax with
     sufficient decimal digits (namely, up to 17) to ensure that a round
     trip back to binary64 will preserve all information.

     That is, for all `GDVBinary64Float v`:

       GDVBinary64Float::parseString(v.toString()) == v
  */

  // ---- create-tuple-class: declarations for GDVBinary64Float +selfCheck +write -writeDefn +move
  /*AUTO_CTC*/ explicit GDVBinary64Float(double const &value);
  /*AUTO_CTC*/ explicit GDVBinary64Float(double &&value);
  /*AUTO_CTC*/ GDVBinary64Float(GDVBinary64Float const &obj) noexcept;
  /*AUTO_CTC*/ GDVBinary64Float(GDVBinary64Float &&obj) noexcept;
  /*AUTO_CTC*/ void selfCheck() const;
  /*AUTO_CTC*/ GDVBinary64Float &operator=(GDVBinary64Float const &obj) noexcept;
  /*AUTO_CTC*/ GDVBinary64Float &operator=(GDVBinary64Float &&obj) noexcept;
  /*AUTO_CTC*/ // For +write:
  /*AUTO_CTC*/ std::string toString() const;
  /*AUTO_CTC*/ void write(std::ostream &os) const;
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, GDVBinary64Float const &obj);

  double getValue() const { return m_value; }
  void setValue(double value);

  /* Comparison of `GDVBinary64Float` is like comparing as floats except
     that -0 < +0 (whereas for floats they are equal).  That is because
     this class is meant primarily for data storage rather than
     arithmetical computation.
  */
  friend int compare(GDVBinary64Float const &a, GDVBinary64Float const &b);
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDVBinary64Float)

  // Parse the format that `write` creates, throwing `XFormat` (declared
  // in exc.h) on error.
  static GDVBinary64Float parseString(std::string_view view);
};


// Exposed for unit testing; see comments on implementation for details.
int compareDoublesRepresentationally(double a, double b);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDV_BINARY64_FLOAT_H
