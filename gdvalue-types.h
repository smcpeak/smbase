// gdvalue-types.h
// Support types for `GDValue`.

#ifndef SMBASE_GDVALUE_TYPES_H
#define SMBASE_GDVALUE_TYPES_H

#include "smbase/gdv-containers-fwd.h"           // gdv::GDV{Sequence,Set,Map} (for clients
#include "smbase/gdv-ordered-map-fwd.h"          // gdv::GDVOrderedMap (for clients)
#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue
#include "smbase/sm-integer-fwd.h"               // smbase::Integer
#include "smbase/std-map-fwd.h"                  // stdfwd::map
#include "smbase/std-set-fwd.h"                  // stdfwd::set
#include "smbase/std-string-fwd.h"               // std::string
#include "smbase/std-vector-fwd.h"               // stdfwd::vector
#include "smbase/std-utility-fwd.h"              // std::pair

#include <cstddef>                               // std::size_t
#include <cstdint>                               // std::int64_t


namespace gdv {

  // Count of elements.
  using GDVSize = std::size_t;

  // Index for vectors.
  using GDVIndex = std::size_t;

  // GDValue(GDVK_INTEGER) holds this.
  using GDVInteger = smbase::Integer;

  // Stored when the kind is GDVK_SMALL_INTEGER.
  using GDVSmallInteger = std::int64_t;

  // Note: GDVBinary64Float is in `gdv-binary64-float.h`.

  // GDValue(GDVK_STRING) holds this.  It is a UTF-8 encoding of the
  // sequence of Unicode code points the string represents.
  using GDVString = std::string;

  //using GDVOctetSequence = std::vector<unsigned char>;

  // `GDVTuple` is defined in `gdvtuple.h`.

  // `GDVOrderedMap` is declared in `gdv-ordered-map-fwd.h`.

  // The entry type for GDVMap and GDVOrderedMap.
  using GDVMapEntry = std::pair<GDValue const, GDValue>;

}


#endif // SMBASE_GDVALUE_TYPES_H
