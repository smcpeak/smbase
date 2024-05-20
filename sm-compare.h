// sm-compare.h
// StrongOrdering enum, similar to standard <compare>, but without
// requiring C++20.


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


// Return the StrongOrdering value corresponding to the sign of 'n'.
inline StrongOrdering strongOrderFromInt(int n)
{
  return n <  0? StrongOrdering::less    :
         n == 0? StrongOrdering::equal   :
                 StrongOrdering::greater ;
}


// Define a 'strongOrder' function from a 'compareTo' method.
#define DEFINE_STRONG_ORDER_FROM_COMPARE_TO(TYPE)                 \
  inline StrongOrdering strongOrder(TYPE const &a, TYPE const &b) \
    { return a.compareTo(b); }


// Define a single relational operator from a 'strongOrder' function.
#define DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, op)         \
  inline bool operator op (TYPE const &a, TYPE const &b) \
    { return strongOrder(a,b) op 0; }


// Define a set of relational operators for a given type, assuming the
// 'strongOrder' function can be applied.
#define DEFINE_RELOPS_FROM_STRONG_ORDER(TYPE) \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, == )   \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, != )   \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, <  )   \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, >  )   \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, <= )   \
  DEFINE_RELOP_FROM_STRONG_ORDER(TYPE, >= )


// Define 'strongOrder' and relational operators from 'compareTo'.
#define DEFINE_RELOPS_FROM_COMPARE_TO(TYPE) \
  DEFINE_STRONG_ORDER_FROM_COMPARE_TO(TYPE) \
  DEFINE_RELOPS_FROM_STRONG_ORDER(TYPE)


// Compare field 'memb' with that in 'obj', returning if the result is
// not equal.  This is meant to be used as part of a sequence of member
// comparisons to implement a lexicographical order.
#define COMPARE_MEMB(memb)                            \
  {                                                   \
    StrongOrdering ord = strongOrder(memb, obj.memb); \
    if (ord != StrongOrdering::equal) {               \
      return ord;                                     \
    }                                                 \
  }


#endif // SMBASE_SM_COMPARE_H
