// sm-compare.h
// StrongOrdering enum.

// This header has some features that are intended to work similiarly to
// those in <compare>, but without requiring C++20.

#ifndef SMBASE_SM_COMPARE_H
#define SMBASE_SM_COMPARE_H


// This holds the result of performing a three-way comparison, similar
// to 'strcmp' or C++20 spaceship '<=>' operator.
//
// This is like C++20 std::strong_ordering.
enum class StrongOrdering : int {
  less = -1,
  equal = 0,
  greater = +1,
};


// Allow comparing StrongOrdering to integers.
#define MAKE_STRONG_ORDERING_RELOP(op)                       \
  inline bool operator op (StrongOrdering v, int u) noexcept \
    { return static_cast<int>(v) op u; }
MAKE_STRONG_ORDERING_RELOP( == )
MAKE_STRONG_ORDERING_RELOP( != )
MAKE_STRONG_ORDERING_RELOP( <  )
MAKE_STRONG_ORDERING_RELOP( >  )
MAKE_STRONG_ORDERING_RELOP( <= )
MAKE_STRONG_ORDERING_RELOP( >= )
#undef MAKE_STRONG_ORDERING_RELOP


// Generic comparison of objects that have operator< and operator==.
//
// This is like C++20 std::strong_order, although I don't do any of the
// sophisticated floating-point stuff.
template <class T>
StrongOrdering strongOrder(T a, T b)
{
  return a <  b? StrongOrdering::less    :
         a == b? StrongOrdering::equal   :
                 StrongOrdering::greater ;
}


// Comparison of strings like strcmp.
StrongOrdering strongOrder(char const *a, char const *b);


#endif // SMBASE_SM_COMPARE_H
