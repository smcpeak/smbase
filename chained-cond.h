// chained-cond.h
// Chained relational conditionals.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_CHAINED_COND_H
#define SMBASE_CHAINED_COND_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)
OPEN_NAMESPACE(cc)                     // "Chained conditionals"


// This is an experiment to see whether I prefer this notation.
template <typename T>
inline bool le_lt(T const &a, T const &b, T const &c)
{
  return a <= b && b < c;
}


// Use zero (default-constructed `T`) as the lower bound.
template <typename T>
inline bool z_le_lt(T const &b, T const &c)
{
  return le_lt(T(), b, c);
}


template <typename T>
inline bool le_le(T const &a, T const &b, T const &c)
{
  return a <= b && b <= c;
}


CLOSE_NAMESPACE(cc)
CLOSE_NAMESPACE(smbase)


#endif // SMBASE_CHAINED_COND_H
