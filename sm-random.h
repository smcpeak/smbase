// sm-random.h
// Utilities related to random number generation.

// This file is in the public domain.

#ifndef SMBASE_SM_RANDOM_H
#define SMBASE_SM_RANDOM_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-sized-int.h"       // SM_FOREACH_SIZED_INT


OPEN_NAMESPACE(smbase)


/* Return a random number in [0,n-1].

   The generated sequence is the same for every program invocation.

   Requires n > 0.
*/
int sm_random(int n);


/* Return a random value of type `PRIM`, uniformly distributed across
   its entire range, including negatives if `PRIM` is a signed type.

   The generated sequence is the same for every program invocation.

   Only a specific set of template instantiations are provided.
*/
template <typename PRIM>
PRIM sm_randomPrim();


#define DECLARE_RANDOM_PRIM(type) \
  extern template type sm_randomPrim<type>();

SM_FOREACH_SIZED_INT(DECLARE_RANDOM_PRIM)

#undef DECLARE_RANDOM_PRIM


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_RANDOM_H
