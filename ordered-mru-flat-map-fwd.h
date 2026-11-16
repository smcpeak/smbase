// ordered-mru-flat-map-fwd.h
// Forward decls for `ordered-mru-flat-map.h`.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_MRU_FLAT_MAP_FWD_H
#define SMBASE_ORDERED_MRU_FLAT_MAP_FWD_H

#include "smbase/std-functional-fwd.h" // std::less [f]

namespace smbase {
  template <typename KEY,
            typename VALUE,
            typename COMPARE = std::less<KEY>>
  class OrderedMRUFlatMap;
}

#endif // SMBASE_ORDERED_MRU_FLAT_MAP_FWD_H
