// gdvalue-map-fwd.h
// Forwards for `gdvalue-map.h`.

#ifndef SMBASE_GDVALUE_MAP_FWD_H
#define SMBASE_GDVALUE_MAP_FWD_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-map-fwd.h"        // std::map


OPEN_NAMESPACE(gdv)


template <typename K, typename V, typename C, typename A>
GDValue toGDValue(std::map<K,V,C,A> const &m);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_MAP_FWD_H
