// most-sig-bit.h
// `mostSignificantBit` function.

// See license.txt for copyright and terms of use.

// This module could be expanded and renamed to have more low-level bit
// tests if I find use for them in the future.

#ifndef SMBASE_MOST_SIG_BIT_H
#define SMBASE_MOST_SIG_BIT_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <cstdint>                     // std::uint64_t


OPEN_NAMESPACE(smbase)


// Return the bit index of the most significant bit of `n` that is 1.
// For example, MSB(1) = 0, MSB(2) = MSB(3) = 1, MSB(4) = 2, etc.
//
// Requires: n > 0
int mostSignificantBit(std::uint64_t n);

// Same, but without compiler built-ins.  This is exposed for the
// benefit of the unit test.
int mostSignificantBit_fallback(std::uint64_t n);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_MOST_SIG_BIT_H
