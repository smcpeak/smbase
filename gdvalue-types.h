// gdvalue-types.h
// Support types for `GDValue`.

// This header is separate so it can be included without getting all of
// gdvalue.h.

#ifndef SMBASE_GDVALUE_TYPES_H
#define SMBASE_GDVALUE_TYPES_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/ordered-map-fwd.h"    // smbase::OrderedMap
#include "smbase/sm-integer-fwd.h"     // smbase::Integer
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-map-fwd.h"        // stdfwd::map
#include "smbase/std-set-fwd.h"        // stdfwd::set
#include "smbase/std-string-fwd.h"     // std::string
#include "smbase/std-vector-fwd.h"     // stdfwd::vector
#include "smbase/std-utility-fwd.h"    // std::pair

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::int64_t


OPEN_NAMESPACE(gdv)


// Count of elements.
using GDVSize = std::size_t;

// Index for vectors.
using GDVIndex = std::size_t;

// GDValue(GDVK_INTEGER) holds this.
using GDVInteger = smbase::Integer;

// Stored when the kind is GDVK_SMALL_INTEGER.
using GDVSmallInteger = std::int64_t;

// GDValue(GDVK_STRING) holds this.  It is a UTF-8 encoding of the
// sequence of Unicode code points the string represents.
using GDVString = std::string;

//using GDVOctetSequence = std::vector<unsigned char>;

// GDValue(GDVK_SEQUENCE) holds this.
using GDVSequence = stdfwd::vector<GDValue>;

// `GDVTuple` is defined in `gdvtuple.h`.

// GDValue(GDVK_SET) holds this.
using GDVSet = stdfwd::set<GDValue>;

// GDValue(GDVK_MAP) holds this.
using GDVMap = stdfwd::map<GDValue, GDValue>;

// GDValue(GDVK_ORDERED_MAP) holds this.
using GDVOrderedMap = smbase::OrderedMap<GDValue, GDValue>;

// The entry type for GDVMap and GDVOrderedMap.
using GDVMapEntry = std::pair<GDValue const, GDValue>;


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_TYPES_H
