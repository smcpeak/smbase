// gdvalue-fwd.h
// Forwards for `gdvalue` module.

// This file is in the public domain.

#ifndef SMBASE_GDVALUE_FWD_H
#define SMBASE_GDVALUE_FWD_H

// IWYU pragma: begin_exports
#include "gdv-containers-fwd.h"        // gdv::GDV{Sequence,Tupe,Set,Map,OrderedMap} [n]
#include "gdvalue-itself-fwd.h"        // gdv::GDValue [n]
#include "gdvalue-kind-fwd.h"          // gdv::GDValueKind [n]
// IWYU pragma: end_exports

namespace gdv {

  template <typename CONTAINER>
  class GDVTaggedContainer;

  using GDVTaggedSequence   = GDVTaggedContainer<GDVSequence>;
  using GDVTaggedTuple      = GDVTaggedContainer<GDVTuple>;
  using GDVTaggedSet        = GDVTaggedContainer<GDVSet>;
  using GDVTaggedMap        = GDVTaggedContainer<GDVMap>;
  using GDVTaggedOrderedMap = GDVTaggedContainer<GDVOrderedMap>;

  class GDVStringIterableC;
  class GDVStringIterable;
  class GDVSequenceIterableC;
  class GDVSequenceIterable;
  class GDVTupleIterableC;
  class GDVTupleIterable;
  class GDVSetIterableC;
  class GDVSetIterable;
  class GDVMapIterableC;
  class GDVMapIterable;
  class GDVOrderedMapIterableC;
  class GDVOrderedMapIterable;

} // namespace gdv

#endif // SMBASE_GDVALUE_FWD_H
