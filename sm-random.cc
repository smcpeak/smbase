// sm-random.cc
// Code for `sm-random` module.

#include "sm-random.h"                 // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-sized-int.h"       // SM_FOREACH_SIZED_INT
#include "smbase/xassert.h"            // xassertPrecondition

#include <limits>                      // std::numeric_limits
#include <random>                      // std::{mt19937, uniform_int_distribution}


OPEN_NAMESPACE(smbase)


int sm_random(int n)
{
  xassertPrecondition(n > 0);

  // Generates a fixed sequence.
  static std::mt19937 rng;

  // Return a uniformly distributed value.
  std::uniform_int_distribution<int> dist(0, n - 1);
  return dist(rng);
}


template <typename PRIM>
PRIM sm_randomPrim()
{
  std::uniform_int_distribution<PRIM> dist(
    std::numeric_limits<PRIM>::min(),
    std::numeric_limits<PRIM>::max());

  if constexpr (sizeof(PRIM) > 4) {
    static std::mt19937_64 rng;
    return dist(rng);
  }
  else {
    static std::mt19937 rng;
    return dist(rng);
  }
}


#define DEFINE_RANDOM_PRIM(type) \
  template type sm_randomPrim<type>();

SM_FOREACH_SIZED_INT(DEFINE_RANDOM_PRIM)

#undef DEFINE_RANDOM_PRIM


CLOSE_NAMESPACE(smbase)


// EOF
