// gdv-containers-fwd.h
// Forward decls for the GDV container classes.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDV_CONTAINERS_FWD_H
#define SMBASE_GDV_CONTAINERS_FWD_H

// IWYU pragma: begin_exports
#include "smbase/gdv-ordered-map-fwd.h"          // gdv::GDVOrderedMap
#include "smbase/gdvtuple-fwd.h"                 // gdv::GDVTuple
// IWYU pragma: end_exports

#include "smbase/gdvalue-itself-fwd.h"           // gdv::GDValue
#include "smbase/std-map-fwd.h"                  // stdfwd::map
#include "smbase/std-set-fwd.h"                  // stdfwd::set
#include "smbase/std-vector-fwd.h"               // stdfwd::vector


namespace gdv {

  // GDValue(GDVK_SEQUENCE) holds this.
  using GDVSequence = stdfwd::vector<GDValue>;

  // GDValue(GDVK_SET) holds this.
  using GDVSet = stdfwd::set<GDValue>;

  // GDValue(GDVK_MAP) holds this.
  using GDVMap = stdfwd::map<GDValue, GDValue>;

}


// Expand `macro` once for each kind of GDV container.
//
// I do not use this macro in every possible place because token pasting
// makes it hard to find things with ordinary text search, and some
// things are important enough that I want to ensure they are easy to
// find.
//
// TODO: Rename to `GDV_FOR_EACH_CONTAINER_TYPE`.
#define FOR_EACH_GDV_CONTAINER(macro)        \
  macro(SEQUENCE,    Sequence,   sequence  ) \
  macro(TUPLE,       Tuple,      tuple     ) \
  macro(SET,         Set,        set       ) \
  macro(MAP,         Map,        map       ) \
  macro(ORDERED_MAP, OrderedMap, orderedMap)


#endif // SMBASE_GDV_CONTAINERS_FWD_H
