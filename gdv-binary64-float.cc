// gdv-binary64-float.cc
// Code for `gdv-binary64-float` module.

#include "gdv-binary64-float.h"        // this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS, smbase::compare
#include "smbase/exc.h"                // xformatsb
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert

#include <charconv>                    // std::from_chars
#include <cmath>                       // std::{isfinite, sign_bit, fpclassify}
#include <iostream>                    // std::ostream
#include <limits>                      // std::numeric_limits
#include <sstream>                     // std::ostringstream
#include <string_view>                 // std::string_view
#include <system_error>                // std::{errc, [make_]error_code}

using namespace smbase;


OPEN_NAMESPACE(gdv)


GDVBinary64Float::GDVBinary64Float()
  : m_value()
{
  selfCheck();
}


// ---- create-tuple-class: definitions for GDVBinary64Float
/*AUTO_CTC*/ GDVBinary64Float::GDVBinary64Float(
/*AUTO_CTC*/   double const &value)
/*AUTO_CTC*/   : IMEMBFP(value)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ GDVBinary64Float::GDVBinary64Float(
/*AUTO_CTC*/   double &&value)
/*AUTO_CTC*/   : IMEMBMFP(value)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ GDVBinary64Float::GDVBinary64Float(GDVBinary64Float const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_value)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ GDVBinary64Float::GDVBinary64Float(GDVBinary64Float &&obj) noexcept
/*AUTO_CTC*/   : MDMEMB(m_value)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ GDVBinary64Float &GDVBinary64Float::operator=(GDVBinary64Float const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_value);
/*AUTO_CTC*/     selfCheck();
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ GDVBinary64Float &GDVBinary64Float::operator=(GDVBinary64Float &&obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     MCMEMB(m_value);
/*AUTO_CTC*/     selfCheck();
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::string GDVBinary64Float::toString() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   std::ostringstream oss;
/*AUTO_CTC*/   write(oss);
/*AUTO_CTC*/   return oss.str();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::ostream &operator<<(std::ostream &os, GDVBinary64Float const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   obj.write(os);
/*AUTO_CTC*/   return os;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


void GDVBinary64Float::selfCheck() const
{
  xassert(std::isfinite(m_value));
}


void GDVBinary64Float::setValue(double value)
{
  *this = GDVBinary64Float(value);
}


/* Compare `a` and `b` such that:

     * All distinct values compare as unequal.  In particular, -0 < +0.

     * All equivalent values compare as equal.  In particular, NaN ==
       NaN.

     * Otherwise, all finite values compare the same as numerically.
*/
int compareDoublesRepresentationally(double a, double b)
{
  using smbase::compare;

  if (std::isfinite(a) && std::isfinite(b)) {
    // First compare the sign bits to ensure -0 < +0.  I swap the order
    // of the arguments here because I want negative < positive, which
    // after calling `signbit` means true < false, but in C++, false <
    // true.
    RET_IF_COMPARE(std::signbit(b), std::signbit(a));

    // Now just compare them normally.
    return compare(a, b);
  }

  else {
    // I'll put the finite values before non-finite values.
    RET_IF_COMPARE(std::isfinite(b), std::isfinite(a));

    // Now we're only dealing with non-finite.  Use the FP
    // classification to separate NaN from Infinity.  Note that the
    // order among `fpclassify` values is implementation-defined.
    RET_IF_COMPARE(std::fpclassify(a), std::fpclassify(b));

    if (std::isinf(a)) {
      // Both are infinite, use ordinary comparison.
      return compare(a, b);
    }
    else {
      // I will say that NaNs compare equal here since my goal is
      // representational rather than numerical equality.
      return 0;
    }
  }
}


int compare(GDVBinary64Float const &a, GDVBinary64Float const &b)
{
  return compareDoublesRepresentationally(a.m_value, b.m_value);
}


void GDVBinary64Float::write(std::ostream &os) const
{
  // Based on https://stackoverflow.com/a/34556738 .
  std::ostringstream oss;
  oss.precision(std::numeric_limits<double>::max_digits10);
  oss << m_value;

  // Ensure the result has a decimal or exponent so it will be reliably
  // recognized as floating-point.
  std::string s = oss.str();
  if (s.find_first_of("eE.") == std::string::npos) {
    s += ".0";
  }

  os << s;
}


/*static*/ GDVBinary64Float GDVBinary64Float::parseString(
  std::string_view view)
{
  // This code is based on the example at:
  // https://en.cppreference.com/w/cpp/utility/from_chars.html
  double value = 0;
  auto [ptr, ec] = std::from_chars(
    view.data(),
    view.data() + view.size(),
    value /*OUT*/);

  try {
    if (ec == std::errc()) {
      if (ptr == view.data() + view.size()) {
        // Successful conversion.
        if (std::isfinite(value)) {
          return GDVBinary64Float(value);
        }
        else {
          // This happens for "NaN", "Infinity", and a few variations.
          xformat("Non-finite value");
        }
      }
      else {
        xformatsb("offset " << (ptr - view.data()) <<
                  ": invalid character");
      }
    }
    else {
      std::error_code code = std::make_error_code(ec);
      xformatsb("offset " << (ptr - view.data()) <<
                ": " << code.message());
    }
  }
  catch (XFormat &x) {
    x.prependContext(stringb(
      "Parsing " << doubleQuote(view) << " as float"));
    throw x;
  }

  // Not reached.
  return GDVBinary64Float();
}


CLOSE_NAMESPACE(gdv)


// EOF
