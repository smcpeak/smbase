// gdvalue-span-fwd.h
// Forward decls for `gdvalue-span.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SPAN_FWD_H
#define SMBASE_GDVALUE_SPAN_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-span-fwd.h"        // smbase::Span

namespace smbase {
  template <typename T>
  gdv::GDValue toGDValue(smbase::Span<T> const &v);
}

#endif // SMBASE_GDVALUE_SPAN_FWD_H
