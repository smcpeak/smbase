// gdvalue-unique-ptr-fwd.h
// Forwards for `gdvalue-unique-ptr.h`.

#ifndef GDVALUE_UNIQUE_PTR_FWD_H
#define GDVALUE_UNIQUE_PTR_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-memory-fwd.h"     // std::unique_ptr


OPEN_NAMESPACE(gdv)


template <typename T, typename D>
GDValue toGDValue(std::unique_ptr<T,D> const &p);


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_UNIQUE_PTR_FWD_H
