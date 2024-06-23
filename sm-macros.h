// sm-macros.h            see license.txt for copyright and terms of use
// A bunch of useful macros.

#ifndef SMBASE_SM_MACROS_H
#define SMBASE_SM_MACROS_H


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


// Member copy ("duplicate") in constructor initializer list.
#define DMEMB(var) var(obj.var)

// Member move in move constructor.  Caller must `#include <utility>`.
#define MDMEMB(var) var(std::move(obj.var))

// Member copy in copy assignment operator.
#define CMEMB(var) (var = obj.var)

// Member move in move assignment operator.
#define MCMEMB(var) (var = std::move(obj.var))

// Member comparison in operator ==.
#define EMEMB(var) (var == obj.var)


// Within a method that is writing the fields of an object, this will
// write one such field
#define WRITE_MEMBER(var) os << " " #var ":" << var /* user ; */


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


/* Get the number of entries in the array `tbl`.

   This has a cast to `int` because:

   * The value is always small enough to fit, as it is simply a count of
     the number of entries in an initializer literally present in the
     source code.

   * I often use signed integers as indices so I can use negative values
     either as "invalid" values or to allow counting backwards and using
     a test like `i >= 0` for loop termination.  If the size is unsigned
     then that at a minimum causes compiler warnings, and could lead to
     incorrect computation for something like `i < TABLESIZE(...)` if
     `i` is negative.
*/
#define TABLESIZE(tbl) ((int)(sizeof(tbl)/sizeof((tbl)[0])))


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
#ifdef __cplusplus
template <class T>
inline void pretendUsedFn(T const &) {}
#define PRETEND_USED(arg) pretendUsedFn(arg) /* user ; */
#endif // __cplusplus


// For use with a function call when the return value is ignored, and I
// do not want a compiler warning about that.  Historically this was
// done by casting to 'void', but GCC-4.6+ no longer recognizes that
// idiom.
#define IGNORE_RESULT(expr) ((void)!(expr))


#if defined(__GNUC__) || defined(__clang__)
  // Appended function declarations (not definitions!) to indicate they
  // do not return control to their caller; e.g.:
  //
  //   void exit(int code) NORETURN;
  //
  #define NORETURN __attribute__((noreturn))

  // Declare that a variable may be unused, e.g.:
  //
  //   int UNUSED some_var = 5;
  //
  #define UNUSED __attribute__((unused))

  // Declare that a function is deprecated, and provide a string to
  // explain why or what to use instead:
  //
  //   void oldFunc() DEPRECATED("Use `newFunc` instead.")
  //
  // The GCC macro does not actually do anything with the reason string,
  // but this provides a standard place to put it, and in the future
  // perhaps there would be a way for the compiler to use it.
  //
  #define DEPRECATED(reasonString) __attribute__((deprecated))

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


// 2024-05-24: In the past, there was in this file a class called
// `Restorer` and a macro called `RESTORER`.  These did not belong in
// this file, so were removed.  The file `save-restore.h` has
// `SetRestore` and `SET_RESTORE` that should be used instead.


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


// Place in a class definition to inhibit the auto-generated copy
// operations.
#define NO_OBJECT_COPIES(name)              \
  name(name&) = delete;                     \
  void operator=(name&) = delete /*user ;*/


// In the past, I had "#define override virtual" here, intended as
// an annotation for which I would later write a checker.  But C++11
// 'override' has made that unnecessary.


/* Open and close namespaces without interfering with indentation (if
   the editor does not treat `namespace` braces specially).  This also
   makes it a little easier to find and (if needed) automatically
   process these enclosures, especially for the closing brace.

   There is nothing to enforce that `CLOSE_NAMESPACE` names the same
   namespace as `OPEN_NAMESPACE`, but that would be a pretty easy check
   to add to some ad-hoc tool at some point.

   See smbase-namespace.txt for information about the `smbase` namespace
   specfically.
*/
#define OPEN_NAMESPACE(name) namespace name {
#define CLOSE_NAMESPACE(name) } /* name */

#define OPEN_ANONYMOUS_NAMESPACE namespace {
#define CLOSE_ANONYMOUS_NAMESPACE } /* anon */


/* For `name` that is declared in the `smbase` namespace, export it into
   the global namespace (assuming that is where the macro is invoked)
   unless `SMBASE_NO_GLOBAL_ALIASES` suppresses that.  The idea is a
   client could set that symbol to turn off the global aliases, which
   are primarily meant for compatibility with older code.
*/
#ifdef SMBASE_NO_GLOBAL_ALIASES
  #define SMBASE_GLOBAL_ALIAS(name) /*nothing*/
#else
  #define SMBASE_GLOBAL_ALIAS(name) using smbase::name;
#endif


// My recollection is there is a way to do what this macro does without
// using variadic macros.  (I think Boost PP has a way.)  For now I'll
// just assume there is, and hence it would be possible to redefine this
// macro if needed.
#define SMBASE_PP_UNWRAP_PARENS(...) __VA_ARGS__

// Define a 'toString' method for an enumeration.  Use like this:
//
//   DEFINE_ENUMERATION_TO_STRING_OR(
//     DocumentProcessStatus,
//     NUM_DOCUMENT_PROCESS_STATUSES,
//     (
//       "DPS_NONE",
//       "DPS_RUNNING",
//       "DPS_FINISHED"
//     ),
//     "DPS_invalid"
//   )
//
#define DEFINE_ENUMERATION_TO_STRING_OR(Enumeration, NUM_VALUES, nameList, unknown) \
  char const *toString(Enumeration value)                                           \
  {                                                                                 \
    RETURN_ENUMERATION_STRING_OR(Enumeration, NUM_VALUES, nameList, value, unknown) \
  }

// The core of the enum-to-string logic, exposed separately so I can use
// it to define functions not called 'toString()'.
#define RETURN_ENUMERATION_STRING_OR(Enumeration, NUM_VALUES, nameList, value, unknown) \
  static char const * const names[] =                                                   \
    { SMBASE_PP_UNWRAP_PARENS nameList };                                               \
  ASSERT_TABLESIZE(names, (NUM_VALUES));                                                \
  if ((unsigned)value < TABLESIZE(names)) {                                             \
    return names[value];                                                                \
  }                                                                                     \
  else {                                                                                \
    return unknown;                                                                     \
  }

// Compatibility.
#define DEFINE_ENUMERATION_TO_STRING(Enumeration, NUM_VALUES, nameList) \
  DEFINE_ENUMERATION_TO_STRING_OR(Enumeration, NUM_VALUES, nameList, "unknown")

// Compatibility.
#define RETURN_ENUMERATION_STRING(Enumeration, NUM_VALUES, nameList, value) \
  RETURN_ENUMERATION_STRING_OR(Enumeration, NUM_VALUES, nameList, value, "unknown")


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


// When defining a static class method outside the class body, I put
// this where the `static` keyword ought to go but C++ does not allow
// it.  This is another one I hope to eventually enforce.
#define STATICDEF /*static*/


/* Function attribute to indicate that it acts like `printf`.

   `formatArgIndex` is the 1-based index of the argument that contains
   the format string.  `firstCheckArgIndex` is the index of the first
   argument to be checked for compatibility with the format string, with
   all subsequent arguments also being checked.

   This goes after the declarator in a prototype, for example:

     std::string stringf(char const *format, ...)
       SM_PRINTF_ANNOTATION(1, 2);
*/
#if defined(__GNUC__) || defined (__CLANG__)
  #define SM_PRINTF_ANNOTATION(formatArgIndex, firstCheckArgIndex) \
    __attribute__((format (printf, formatArgIndex, firstCheckArgIndex)))
#else
  #define SM_PRINTF_ANNOTATION(formatArgIndex, firstCheckArgIndex) \
    /*nothing*/
#endif


// These provide a concise way to loop on an integer range.
#define smbase_loopi(end) for(int i=0; i<(int)(end); i++)
#define smbase_loopj(end) for(int j=0; j<(int)(end); j++)
#define smbase_loopk(end) for(int k=0; k<(int)(end); k++)


// `ENABLE_SELFCHECK` can be set to 0 (disable) or 1 (enable).  If not
// explicitly set, it is set to 0 when `NDEBUG`.
#ifndef ENABLE_SELFCHECK
  #ifdef NDEBUG
    #define ENABLE_SELFCHECK 0
  #else
    #define ENABLE_SELFCHECK 1
  #endif
#endif

// The `SELFCHECK()` macro runs the `selfCheck()` method unless it is
// disabled for speed reasons.
#if ENABLE_SELFCHECK != 0
  #define SELFCHECK() selfCheck()
#else
  #define SELFCHECK() ((void)0)
#endif


// Compatiblity macro originally created to deal with compilers that
// did not have the `explicit` keyword.  Obsolete.
#define EXPLICIT explicit


#endif // SMBASE_SM_MACROS_H
