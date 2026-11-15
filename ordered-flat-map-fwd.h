// ordered-flat-map-fwd.h
// Forward decls for `ordered-flat-map.h`.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_FLAT_MAP_FWD_H
#define SMBASE_ORDERED_FLAT_MAP_FWD_H

#include "smbase/std-functional-fwd.h" // std::less

namespace smbase {
  template <typename KEY,
            typename VALUE,
            typename COMPARE = std::less<KEY>>
  class OrderedFlatMap;
}

#endif // SMBASE_ORDERED_FLAT_MAP_FWD_H
