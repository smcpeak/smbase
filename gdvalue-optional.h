// gdvalue-optional.h
// Conversion between `GDValue` and `std::optional`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_OPTIONAL_H
#define SMBASE_GDVALUE_OPTIONAL_H

#include "gdvalue-optional-fwd.h"      // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <optional>                    // std::optional


OPEN_NAMESPACE(gdv)


template <typename T>
GDValue toGDValue(std::optional<T> const &p)
{
  if (p.has_value()) {
    return toGDValue(p.value());
  }
  else {
    // We assume that null is sufficiently distinct.
    return GDValue();
  }
}


template <typename T>
struct GDVPTo<std::optional<T>> {
  static std::optional<T> f(GDValueParser const &p)
  {
    if (p.isNull()) {
      return std::nullopt;
    }
    else {
      return std::optional<T>(gdvpTo<T>(p));
    }
  }
};


CLOSE_NAMESPACE(gdv)



#endif // SMBASE_GDVALUE_OPTIONAL_H
