// gdvalue-list-fwd.h
// Forward decls for `gdvalue-list.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_LIST_FWD_H
#define SMBASE_GDVALUE_LIST_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-list-fwd.h"       // std::list


OPEN_NAMESPACE(gdv)


template <typename T, typename A>
GDValue toGDValue(std::list<T,A> const &lst);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_LIST_FWD_H
