// gdvalue-optional-fwd.h
// Forward decls for `gdvalue-optional.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_OPTIONAL_FWD_H
#define SMBASE_GDVALUE_OPTIONAL_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/gdvalue-parser-fwd.h" // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::optional


OPEN_NAMESPACE(gdv)


template <typename T>
inline GDValue toGDValue(std::optional<T> const &p);

template <typename T>
struct GDVPTo<std::optional<T>>;


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_OPTIONAL_FWD_H
