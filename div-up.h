// div-up.h
// div_up function template.

// This file is in the public domain.

// This `div_up` function template was in typ.h for a long time.  It is
// not used in smbase but I think it is used in one of its clients.  It
// does not really fit anywhere else, so I put it into its own file.

#ifndef SMBASE_DIV_UP_H
#define SMBASE_DIV_UP_H


// Division with rounding towards +inf (when operands are positive).
template <class T>
inline T div_up(T const &x, T const &y)
{
  return (x + y - 1) / y;
}


// Round `x` up to the next multiple of `y`.
template <class T>
inline T round_up(T const &x, T const &y)
{
  return div_up(x,y) * y;
}


#endif // SMBASE_DIV_UP_H
