// sm-macros.h            see license.txt for copyright and terms of use
// grab-bag of useful macros, stashed here to avoid mucking up
//   other modules with more focus; there's no clear rhyme or
//   reason for why some stuff is here and some in typ.h
// (no configuration stuff here!)

#ifndef SMBASE_SM_MACROS_H
#define SMBASE_SM_MACROS_H

#include "typ.h"        // bool


// Concatenate tokens.  Unlike plain '##', this works for __LINE__.  It
// is the same as BOOST_PP_CAT.
#define SMBASE_PP_CAT(a,b) SMBASE_PP_CAT2(a,b)
#define SMBASE_PP_CAT2(a,b) a##b


// complement of ==
#define NOTEQUAL_OPERATOR(T)             \
  bool operator != (T const &obj) const  \
    { return !operator==(obj); }

// toss this into a class that already has == and < defined, to
// round out the set of relational operators (assumes a total
// order, i.e.  a < b  <=>  b < a)
#define RELATIONAL_OPERATORS(T)                    \
  NOTEQUAL_OPERATOR(T)                             \
  bool operator <= (T const &obj) const            \
    { return !obj.operator<(*this); }              \
  bool operator > (T const &obj) const             \
    { return obj.operator<(*this); }               \
  bool operator >= (T const &obj) const            \
    { return !operator<(obj); }


// member copy in constructor initializer list
#define DMEMB(var) var(obj.var)

// member copy in operator =
#define CMEMB(var) (var = obj.var)

// member comparison in operator ==
#define EMEMB(var) (var == obj.var)


// standard insert operator
// (note that you can put 'virtual' in front of the macro call if desired)
#define INSERT_OSTREAM(T)                                \
  void insertOstream(ostream &os) const;                 \
  friend ostream& operator<< (ostream &os, T const &obj) \
    { obj.insertOstream(os); return os; }


// usual declarations for a data object (as opposed to control object)
#define DATA_OBJ_DECL(T)                \
  T();                                  \
  T(T const &obj);                      \
  ~T();                                 \
  T& operator= (T const &obj);          \
  bool operator== (T const &obj) const; \
  NOTEQUAL_OPERATOR(T)                  \
  INSERTOSTREAM(T)


// copy this to the .cc file for implementation of DATA_OBJ_DECL
#if 0
T::T()
{}

T::T(T const &obj)
  : DMEMB(),
    DMEMB(),
    DMEMB()
{}

T::~T()
{}

T& T::operator= (T const &obj)
{
  if (this != &obj) {
    CMEMB();
  }
  return *this;
}

bool T::operator== (T const &obj) const
{
  return
    EMEMB() &&
    EMEMB();
}

void T::insertOstream(ostream &os) const
{}
#endif // 0


#if __cplusplus >= 201103L
  // I think the C++11 requirement of a message string is dumb, but I
  // do not want to require C++17 yet, so I use "".
  #define STATIC_ASSERT(cond) static_assert((cond), "")
#else
  // assert something at compile time (must use this inside a function);
  // works because compilers won't let us declare negative-length arrays
  // (the expression below works with egcs-1.1.2, gcc-2.x, gcc-3.x)
  //
  // 2022-05-26: I've run into a case where this implementation does not
  // work.  Specifically, in sm-rc-ptr.h, when trying to check
  // std::is_convertible, the compiler allows this even when the
  // condition is false.  I think there's some problem due to the
  // template context, although this is the first I've seen of such an
  // issue.   I'm switching to the C++11 standard version, when
  // available, rather than dive into the guts of GCC to figure out the
  // problem.
  #define STATIC_ASSERT(cond) \
    { (void)((int (*)(char failed_static_assertion[(cond)?1:-1]))0); }
#endif

// assert that a table is an expected size; the idea is to make sure
// that static data in some table gets updated when a corresponding
// symbolic constant is changed
#define ASSERT_TABLESIZE(table, size) \
  STATIC_ASSERT(TABLESIZE(table) == (size))


// for silencing variable-not-used warnings
template <class T>
inline void pretendUsedFn(T const &) {}
#define PRETEND_USED(arg) pretendUsedFn(arg) /* user ; */


// For use with a function call when the return value is ignored, and I
// do not want a compiler warning about that.  Historically this was
// done by casting to 'void', but GCC-4.6+ no longer recognizes that
// idiom.
#define IGNORE_RESULT(expr) ((void)!(expr))


// appended to function declarations to indicate they do not
// return control to their caller; e.g.:
//   void exit(int code) NORETURN;
#ifdef __GNUC__
  #define NORETURN __attribute__((noreturn))

  // Declare that a variable may be unused, e.g.:
  //   int UNUSED some_var = 5;
  #define UNUSED __attribute__((unused))
#else
  // just let the warnings roll if we can't suppress them
  #define NORETURN
  #define UNUSED
#endif


// This macro is meant to be used within a class definition.
//
// Given a subtype 'destType', declare four functions that perform
// checked downcasts to that type, checked with an 'isXXX' method.
// The methods that end in 'C' are the const versions.  The methods
// that begin with 'as' assert that the cast is accurate, while the
// methods that begin with 'if' return NULL if it is not.
//
// The 'as/isXXXC' methods are 'virtual' in order to allow a subclass to
// pretend to be a different subclass.  I do this in Elsa's TypedefType.
#define DOWNCAST_FN(destType)                                                   \
  virtual destType const *as##destType##C() const;                              \
  destType *as##destType() { return const_cast<destType*>(as##destType##C()); } \
  virtual destType const *if##destType##C() const;                              \
  destType *if##destType() { return const_cast<destType*>(if##destType##C()); }

// These declarations can be used in a subclass that is pretending to
// be another one by only overriding 'as/isXXXC'.
#define OVERRIDE_DOWNCAST_FN(destType)              \
  destType const *as##destType##C() const override; \
  destType const *if##destType##C() const override;

// This macro is used in the implementation file of a class that uses
// DOWNCAST_FN in its definition.
#define DOWNCAST_IMPL(inClass, destType)            \
  destType const *inClass::as##destType##C() const  \
  {                                                 \
    xassert(is##destType());                        \
    return static_cast<destType const*>(this);      \
  }                                                 \
  destType const *inClass::if##destType##C() const  \
  {                                                 \
    if (!is##destType()) {                          \
      return NULL;                                  \
    }                                               \
    return static_cast<destType const*>(this);      \
  }


// keep track of a count and a high water mark
#define INC_HIGH_WATER(count, highWater)  \
  count++;                                \
  if (count > highWater) {                \
    highWater = count;                    \
  }


// egcs has the annoying "feature" that it warns
// about switches on enums where not all cases are
// covered .... what is this, f-ing ML??
#define INCL_SWITCH \
  default: break; /*silence warning*/


// for a class that maintains allocated-node stats
#define ALLOC_STATS_DECLARE                     \
  static int numAllocd;                         \
  static int maxAllocd;                         \
  static void printAllocStats(bool anyway);

// these would go in a .cc file, whereas above goes in .h file
#define ALLOC_STATS_DEFINE(classname)                      \
  int classname::numAllocd = 0;                            \
  int classname::maxAllocd = 0;                            \
  STATICDEF void classname::printAllocStats(bool anyway)   \
  {                                                        \
    if (anyway || numAllocd != 0) {                        \
      cout << #classname << " nodes: " << numAllocd        \
           << ", max  nodes: " << maxAllocd                \
           << endl;                                        \
    }                                                      \
  }

#define ALLOC_STATS_IN_CTOR                     \
  INC_HIGH_WATER(numAllocd, maxAllocd);

#define ALLOC_STATS_IN_DTOR                     \
  numAllocd--;


// ----------- automatic data value restorer -------------
// used when a value is to be set to one thing now, but restored
// to its original value on return (even when the return is by
// an exception being thrown)
template <class T>
class Restorer {
  T &variable;
  T prevValue;

public:
  Restorer(T &var, T newValue)
    : variable(var),
      prevValue(var)
  {
    variable = newValue;
  }

  // this one does not set it to a new value, just remembers the current
  Restorer(T &var)
    : variable(var),
      prevValue(var)
  {}

  ~Restorer()
  {
    variable = prevValue;
  }
};


// Declare a restorer for 'variable', of 'type'.
#define RESTORER(type, variable, value) \
  Restorer< type > SMBASE_PP_CAT(restorer,__LINE__)((variable), (value)) /* user ; */


// declare a bunch of a set-like operators for enum types
#define ENUM_BITWISE_AND(Type)                  \
  inline Type operator& (Type f1, Type f2)      \
    { return (Type)((int)f1 & (int)f2); }       \
  inline Type& operator&= (Type &f1, Type f2)   \
    { return f1 = f1 & f2; }

#define ENUM_BITWISE_OR(Type)                   \
  inline Type operator| (Type f1, Type f2)      \
    { return (Type)((int)f1 | (int)f2); }       \
  inline Type& operator|= (Type &f1, Type f2)   \
    { return f1 = f1 | f2; }

#define ENUM_BITWISE_XOR(Type)                  \
  inline Type operator^ (Type f1, Type f2)      \
    { return (Type)((int)f1 ^ (int)f2); }       \
  inline Type& operator^= (Type &f1, Type f2)   \
    { return f1 = f1 ^ f2; }

#define ENUM_BITWISE_NOT(Type, ALL)             \
  inline Type operator~ (Type f)                \
    { return (Type)((~(int)f) & ALL); }

#define ENUM_BITWISE_OPS(Type, ALL)             \
  ENUM_BITWISE_AND(Type)                        \
  ENUM_BITWISE_OR(Type)                         \
  ENUM_BITWISE_XOR(Type)                        \
  ENUM_BITWISE_NOT(Type, ALL)


// Iterate over the elements of 'Enumeration', assuming that the first
// element has code 0.
#define FOR_EACH_ENUM_ELEMENT(Enumeration, NUM_ELTS, iter) \
  for (Enumeration iter = static_cast<Enumeration>(0);     \
       iter < NUM_ELTS;                                    \
       iter = static_cast<Enumeration>(iter+1))


// macro to conditionalize something on NDEBUG; I typically use this
// to hide the declaration of a variable whose value is only used by
// debugging trace statements (and thus provokes warnings about unused
// variables if NDEBUG is set)
#ifdef NDEBUG
  #define IFDEBUG(stuff)
#else
  #define IFDEBUG(stuff) stuff
#endif


// put at the top of a class for which the default copy ctor
// and operator= are not desired; then don't define these functions
#define NO_OBJECT_COPIES(name)   \
  private:                       \
    name(name&);                 \
    void operator=(name&) /*user ;*/


// In the past, I had "#define override virtual" here, intended as
// an annotation for which I would later write a checker.  But C++11
// 'override' has made that unnecessary.


// Open and close namespaces w/o interfering with indentation.
#define OPEN_NAMESPACE(name) namespace name {
#define CLOSE_NAMESPACE(name) } /* name */

#define OPEN_ANONYMOUS_NAMESPACE namespace {
#define CLOSE_ANONYMOUS_NAMESPACE } /* anon */


// My recollection is there is a way to do what this macro does without
// using variadic macros.  (I think Boost PP has a way.)  For now I'll
// just assume there is, and hence it would be possible to redefine this
// macro if needed.
#define SMBASE_PP_UNWRAP_PARENS(...) __VA_ARGS__

// Define a 'toString' method for an enumeration.  Use like this:
//
//   DEFINE_ENUMERATION_TO_STRING(
//     DocumentProcessStatus,
//     NUM_DOCUMENT_PROCESS_STATUSES,
//     (
//       "DPS_NONE",
//       "DPS_RUNNING",
//       "DPS_FINISHED"
//     )
//   )
//
#define DEFINE_ENUMERATION_TO_STRING(Enumeration, NUM_VALUES, nameList) \
  char const *toString(Enumeration value)                               \
  {                                                                     \
    RETURN_ENUMERATION_STRING(Enumeration, NUM_VALUES, nameList, value) \
  }

// The core of the enum-to-string logic, exposed separately so I can use
// it to define functions not called 'toString()'.
#define RETURN_ENUMERATION_STRING(Enumeration, NUM_VALUES, nameList, value) \
  static char const * const names[] =                                       \
    { SMBASE_PP_UNWRAP_PARENS nameList };                                   \
  ASSERT_TABLESIZE(names, (NUM_VALUES));                                    \
  if ((unsigned)value < TABLESIZE(names)) {                                 \
    return names[value];                                                    \
  }                                                                         \
  else {                                                                    \
    return "unknown";                                                       \
  }


// Given an array with known size, return a one-past-the-end pointer
// that can serve as the 'end' location for STL algorithms.
#define ARRAY_ENDPTR(array) ((array) + TABLESIZE(array))


// This is meant to be used like a type qualifier to document that a
// pointer can be null.  Examples:
//
//   int * NULLABLE intptr = ...;
//   int * NULLABLE func() { ... }
//   void takesPtr(int * NULLABLE param);
//
// Right now it's just documentation, but maybe someday it will be
// enforced automatically.
//
#define NULLABLE /**/


#endif // SMBASE_SM_MACROS_H
