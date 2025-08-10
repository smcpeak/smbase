// gdvalue-span.h
// Conversion from `Span` to `GDValue`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SPAN_H
#define SMBASE_GDVALUE_SPAN_H

#include "gdvalue-span-fwd.h"          // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-span.h"            // smbase::Span


OPEN_NAMESPACE(smbase)


template <typename T>
gdv::GDValue toGDValue(smbase::Span<T> const &sp)
{
  // Partly as an experiment, I've chosen to put this `toGDValue` into
  // `smbase` since that is where `Span` is.  But when this code calls
  // `toGDValue` below, we need to still see the overloads in `gdv` for
  // things like `int`.
  using gdv::toGDValue;

  gdv::GDValue ret(gdv::GDVK_SEQUENCE);

  for (T const &t : sp) {
    ret.sequenceAppend(toGDValue(t));
  }

  return ret;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_GDVALUE_SPAN_H
