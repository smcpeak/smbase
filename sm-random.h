// sm-random.h
// Utilities related to random number generation.

// This file is in the public domain.

#ifndef SMBASE_SM_RANDOM_H
#define SMBASE_SM_RANDOM_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-sized-int.h"       // SM_FOREACH_SIZED_INT
#include "smbase/std-string-fwd.h"     // std::string [n]


OPEN_NAMESPACE(smbase)


// Function pointer that can be set to intercept `sm_random` for testing
// purposes.  Initially nullptr, meaning `sm_random` operates normally.
extern int (*sm_random_intercept)(int n);


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


// Return a random string of length up to `n-1`, where each character
// has a 1/20 chance of being a newline if `withNL`.  The non-newline
// characters are all the same, with the exact character in ['A','Z'].
std::string randomString(int n, bool withNL);

// Variants with `withNL` as true and false, respectively.
std::string randomStringWithNL(int n);
std::string randomStringNoNL(int n);


// Facilitate making a weighted random choice, especially in the context
// of randomized testing.
//
// This uses `sm_random`, so the sequence of choices is fixed with each
// program invocation.
class RandomChoice {
public:      // data
  // Size of the uniform range.
  //
  // Requires: m_rangeSize > 0
  int const m_rangeSize;

  // We've checked for all numbers below this value.
  //
  // Invariant: 0 <= m_checkLimit <= m_rangeSize
  int m_checkLimit;

  // Selected element in [0, m_rangeSize-1].
  int const m_choice;

public:
  // Initialize by choosing a value in [0, rangeSize-1].
  RandomChoice(int rangeSize);

  // Assert invariants.
  void selfCheck() const;

  // Check whether the choice lands within the next `n` numbers.  That
  // is, the probability of `check(n)` is proportional to `n`.  The sum
  // of all `n` passed to `check` must not exceed `m_rangeSize`.
  bool check(int n);

  // True if the choice has not been in any checked range.
  bool remains() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_RANDOM_H
