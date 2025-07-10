// gdvalue-set-fwd.h
// Forwards for `gdvalue-set.h`.

#ifndef SMBASE_GDVALUE_SET_FWD_H
#define SMBASE_GDVALUE_SET_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-set-fwd.h"        // std::set


OPEN_NAMESPACE(gdv)


template <typename V, typename C, typename A>
GDValue toGDValue(std::set<V,C,A> const &s);


CLOSE_NAMESPACE(gdv)



#endif // SMBASE_GDVALUE_SET_FWD_H
