// gdvalue-tuple-fwd.h
// Forward decls for `gdvalue-tuple.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_TUPLE_FWD_H
#define SMBASE_GDVALUE_TUPLE_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/gdvalue-parser-fwd.h" // gdv::GDVPTo [n]
#include "smbase/std-tuple-fwd.h"      // std::tuple [n]


namespace gdv {

  template <typename... Elements>
  GDValue toGDValue(std::tuple<Elements...> const &t);

  template <typename... Elements>
  struct GDVPTo<std::tuple<Elements...>>;

}


#endif // SMBASE_GDVALUE_TUPLE_FWD_H
