// gdvalue-either-fwd.h
// Forward decls for `gdvalue-either.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_EITHER_FWD_H
#define SMBASE_GDVALUE_EITHER_FWD_H

#include "smbase/either-fwd.h"         // smbase::Either
#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


template <typename LEFT, typename RIGHT>
GDValue toGDValue(smbase::Either<LEFT, RIGHT> const &e);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_EITHER_FWD_H
