// gdvalue-vector-fwd.h
// Forwards for `gdvalue-vector.h`.

#ifndef SMBASE_GDVALUE_VECTOR_FWD_H
#define SMBASE_GDVALUE_VECTOR_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-vector-fwd.h"     // std::vector


OPEN_NAMESPACE(gdv)


template <typename T, typename A>
GDValue toGDValue(std::vector<T,A> const &v);


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_VECTOR_FWD_H
