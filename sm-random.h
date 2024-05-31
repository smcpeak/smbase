// sm-random.h
// Utilities related to random number generation.

// This file is in the public domain.

#ifndef SMBASE_SM_RANDOM_H
#define SMBASE_SM_RANDOM_H

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstdlib>                     // std::rand


OPEN_NAMESPACE(smbase)


/* Return a random number in [0,n-1].

   This isn't particularly good, it's just simple.
*/
inline int sm_random(int n)
{
  return std::rand() % n;
}


/* Return a random value of type `PRIM`, approximately uniformly
   distributed.

   This is sort of a placeholder until I can decipher the RNG facilities
   in libc++.
*/
template <typename PRIM>
PRIM sm_randomPrim()
{
  PRIM v = sm_random(256);

  // This test is needed to avoid Clang complaining about the shift.
  if (sizeof(PRIM) > 1) {
    for (int i=1; i < (int)sizeof(PRIM); ++i) {
      v <<= 8;
      v |= sm_random(256);
    }
  }

  return v;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_RANDOM_H
