// compare-util-iface.h
// Interface for `compare-util.h`.

#ifndef SMBASE_COMPARE_UTIL_IFACE_H
#define SMBASE_COMPARE_UTIL_IFACE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


// Although `compare` and `compareSequences` are in `smbase`, the
// intention is that additional overloads will be put into other
// namespaces.  Consequently, the macros below do not use `smbase::` as
// a qualifier when invoking `compare`.  Instead, client code should use
// a `using` declaration or directive to make `smbase::compare` visible
// if desired.
OPEN_NAMESPACE(smbase)


// ----------------------- Comparison functions ------------------------
// Return -1 if a<b, +1 if a>b, and 0 otherwise.
template <class NUM>
inline int compare(NUM const &a, NUM const &b);


// Compare two sequence containers lexicographically.
template <class CONTAINER>
inline int compareSequences(CONTAINER const &a, CONTAINER const &b);


// --------------- Macros to use in comparison functions ---------------
// Return the value of `expr` if it is nonzero.
#define RET_IF_NONZERO(expr) \
  if (int ret = (expr)) {    \
    return ret;              \
  }


// Compare 'a' to 'b' and return if they are unequal.
#define RET_IF_COMPARE(a, b) RET_IF_NONZERO(compare((a), (b)))


// Compare member 'memb' from objects 'a' and 'b' (assumed to be in
// scope), returning the comparison result.
#define COMPARE_MEMBERS(memb) compare(a.memb, b.memb)

// Compare two pointer values first for pointer equality, then as a
// deep comparison of contents.
#define DEEP_COMPARE_PTR_MEMBERS(memb) \
  ((a.memb == b.memb)? 0 : compare(*(a.memb), *(b.memb)))


// If two members are not equal, return the comparison result.  This is
// meant to be used as part of a comparison chain.
#define RET_IF_COMPARE_MEMBERS(memb) \
  RET_IF_NONZERO(COMPARE_MEMBERS(memb))

#define RET_IF_DEEP_COMPARE_PTR_MEMBERS(memb) \
  RET_IF_NONZERO(DEEP_COMPARE_PTR_MEMBERS(memb))


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


// -------------------- Define relational operators --------------------
// Define a single friend relational operator in terms of `compare`.
//
// Mark it "maybe_unused" because I typically generate all six for
// uniformity without necessarily using all (or any) of them.
#define DEFINE_ONE_FRIEND_RELATIONAL_OPERATOR(Class, op)   \
  [[maybe_unused]]                                         \
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


/* Declare a `compareTo` method that must be implemented elsewhere.
   Then, define a friend `compare` in terms of it, and friend relational
   operators in terms of that.

   Why not just define `compare` instead?  Well, it turns out that does
   not work for private (nested) classes because friends of private
   classes cannot be defined outside their class since the access
   control rules prevent even naming them, which is a prerequisite to
   defining such a friend function.  (I think this is a bug in the
   design of C++.)  So, in order to allow the comparison function to be
   defined outside its class body, it has to be a method, not a friend.
*/
#define DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(Class) \
  int compareTo(Class const &b) const;                  \
  friend int compare(Class const &a, Class const &b)    \
    { return a.compareTo(b); }                          \
  DEFINE_FRIEND_RELATIONAL_OPERATORS(Class)


// ---------------- Heterogeneous comparison operators -----------------
// These macros define relational operators that compare two different
// types.  They are meant to be used within the body of `Class`, for
// the purpose of comparing it to `Other`.  They generate operators that
// work in either order, all in terms of a single `compareTo` method,
// that must be defined elsewhere by the user.

// Define one operator that compares this `Class` to `Other` in either
// direction.
#define DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, op) \
  [[maybe_unused]]                                                       \
  friend bool operator op (Class const &a, Other const &b)               \
    { return compare(a,b) op 0; }                                        \
  [[maybe_unused]]                                                       \
  friend bool operator op (Other const &a, Class const &b)               \
    { return compare(a,b) op 0; }


// Declare a set of friend comparison-to-other operators, *excluding*
// the equality operators, assuming that two suitable 'compare'
// functions exist.
#define DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_TO_OTHER_OPERATORS(Class, Other) \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, < )             \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, <=)             \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, > )             \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, >=)


// Declare a set of friend comparison-to-other operators, assuming that
// 'compare' exists.
#define DEFINE_FRIEND_RELATIONAL_TO_OTHER_OPERATORS(Class, Other)        \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, ==)       \
  DEFINE_ONE_FRIEND_RELATIONAL_TO_OTHER_OPERATOR(Class, Other, !=)       \
  DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_TO_OTHER_OPERATORS(Class, Other)


/* Declare a `compareTo` method, to compare to an `Other` type, that
   must be implemented elsewhere.  Then, define friend `compare` methods
   terms of it, and friend relational operators in terms of that.
*/
#define DECLARE_COMPARETO_AND_DEFINE_RELATIONALS_TO_OTHER(Class, Other) \
  int compareTo(Other const &b) const;                                  \
  [[maybe_unused]]                                                      \
  friend int compare(Class const &a, Other const &b)                    \
    { return a.compareTo(b); }                                          \
  [[maybe_unused]]                                                      \
  friend int compare(Other const &a, Class const &b)                    \
    { return -(b.compareTo(a)); }                                       \
  DEFINE_FRIEND_RELATIONAL_TO_OTHER_OPERATORS(Class, Other)


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_COMPARE_UTIL_IFACE_H
