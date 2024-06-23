// compare-util-iface.h
// Interface for `compare-util.h`.

#ifndef SMBASE_COMPARE_UTIL_IFACE_H
#define SMBASE_COMPARE_UTIL_IFACE_H


// Return -1 if a<b, +1 if a>b, and 0 otherwise.
template <class NUM>
int compare(NUM const &a, NUM const &b);


// Compare 'a' to 'b' and return if they are unequal.
#define RET_IF_COMPARE(a, b)         \
  if (int ret = compare((a), (b))) { \
    return ret;                      \
  }


// Compare member 'memb' from objects 'a' and 'b' (assumed to be in
// scope), returning the comparison result.
#define COMPARE_MEMBERS(memb) compare(a.memb, b.memb)

// Compare two pointer values first for pointer equality, then as a
// deep comparison of contents.
#define DEEP_COMPARE_PTR_MEMBERS(memb) \
  ((a.memb == b.memb)? 0 : compare(*(a.memb), *(b.memb)))


// If two members are not equal, return the comparison result.  This is
// meant to be used as part of a comparison chain.
#define RET_IF_COMPARE_MEMBERS(memb) RET_IF_COMPARE(a.memb, b.memb)


/* Compare a base class subobjects of objects 'a' and 'b'.

   The cast is needed because this is meant to be used from within the
   definition of a 'compare' function that operates on a superclass, so
   without the cast, this would just be the function calling itself in
   an infinite loop.
*/
#define RET_IF_COMPARE_SUBOBJS(BaseType)           \
  RET_IF_COMPARE(static_cast<BaseType const &>(a), \
                 static_cast<BaseType const &>(b))


// If 'a' and 'b' compare equal, return 0.  This is meant for cases
// where they are pointers, so the equality test is quick, whereas a
// content check might be slow.
#define RET_ZERO_IF_EQUAL(a, b) \
  if ((a) == (b)) {             \
    return 0;                   \
  }


// Check two members 'memb' for equal in a fast-path check.
#define RET_ZERO_IF_EQUAL_MEMB(memb) RET_ZERO_IF_EQUAL(a.memb, b.memb)


// Define a single friend relational operator in terms of `compare`.
#define DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, op) \
  friend bool operator op (Class const &a, Class const &b) \
    { return compare(a,b) op 0; }


// Declare a set of friend comparison operators, *excluding* the
// equality operators, assuming that a 'compare' function exists.
#define DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_OPERATORS(Class) \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, < )             \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, <=)             \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, > )             \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, >=)


// Declare a set of friend comparison operators, assuming that a
// 'compare' function exists.
#define DEFINE_FRIEND_RELATIONAL_OPERATORS(Class)        \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, ==)       \
  DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, !=)       \
  DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_OPERATORS(Class)


#endif // SMBASE_COMPARE_UTIL_IFACE_H
