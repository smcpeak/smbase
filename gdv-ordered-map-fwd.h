// gdv-ordered-map-fwd.h
// Forward decls for `gdv-ordered-map.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDV_ORDERED_MAP_FWD_H
#define SMBASE_GDV_ORDERED_MAP_FWD_H

#include "smbase/gdvalue-itself-fwd.h" // gdv::GDValue [n]
#include "smbase/ordered-map-fwd.h"    // smbase::OrderedMap [n]


namespace gdv {

  using GDVOrderedMap = smbase::OrderedMap<GDValue, GDValue>;

} // namespace gdv


#endif // SMBASE_GDV_ORDERED_MAP_FWD_H
