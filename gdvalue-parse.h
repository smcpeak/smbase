// gdvalue-parse.h
// Routines to "parse" GDValues into more structured data.

// The functions in this module throw `XFormat`, defined in
// smbase/exc.h.

// The functions here are intended to never fail an assertion inside
// `GDValue`; they should do all necessary precondition checking.

#ifndef SMBASE_GDVALUE_PARSE_H
#define SMBASE_GDVALUE_PARSE_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/gdvalue-types.h"      // gdv::GDVIndex
#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_NAMESPACE
#include "smbase/std-memory-fwd.h"     // std::unique_ptr
#include "smbase/std-string-fwd.h"     // std::string

#include <type_traits>                 // std::{enable_if_t, is_final, is_constructible}


OPEN_NAMESPACE(gdv)


// ------------------- Stand-alone parsing functions -------------------
// Throw if `v` does not have kind `kind`.
void checkGDValueKind(GDValue const &v, GDValueKind kind);

// Throw if `v` is not a symbol.
void checkIsSymbol(GDValue const &v);

// Throw if `v` is not a small integer.
void checkIsSmallInteger(GDValue const &v);

// Throw if `v` is not a string.
void checkIsString(GDValue const &v);

// Return `v.stringGet()`; throw if it is not a string.
GDVString stringGet_parse(GDValue const &v);

// Throw if `v` is not a sequence.
void checkIsSequence(GDValue const &v);

// Throw if `v` is not a tuple.
void checkIsTuple(GDValue const &v);

// Throw if `v` is not a tuple, or `index` is out of range.
void checkTupleIndex(GDValue const &v, GDVIndex index);

// Throw if `v` is not a set or tagged set.
void checkIsSet(GDValue const &v);

// Throw if `v` is not a map or tagged map.
void checkIsMap(GDValue const &v);

// Throw if `v` is not a tagged map.
void checkIsTaggedMap(GDValue const &v);

// Throw unless `v` is a possibly-ordered map, possibly tagged.
void checkIsPOMap(GDValue const &v);

// Throw if `v` is not a tagged container.
void checkIsTaggedContainer(GDValue const &v);

// Throw if `v` is not a tagged container with tag `symName`.
void checkContainerTag(GDValue const &v, char const *symName);

// Throw if `v` is not a tagged map with symbol `symName`.
void checkTaggedMapTag(GDValue const &v, char const *symName);

// Throw unless `v` is a tagged ordered map with symbol `symName`.
void checkTaggedOrderedMapTag(GDValue const &v, char const *symName);

// Return `v.tupleGetValueAt(index)`, except throw if there is a
// problem.
GDValue tupleGetValueAt_parse(GDValue const &v, GDVIndex index);

// Return `v.mapGetSym(symName)`, except throw if there is a problem.
GDValue mapGetSym_parse(GDValue const &v, char const *symName);

// Return `v.mapGetSym(symName)`.  If `v` is a map, but does not have
// `symName` mapped, return `GDValue()` (null).  If it is not a map,
// then throw.
GDValue mapGetSym_parseOpt(GDValue const &v, char const *symName);

// Return `v.mapGetValueAt(str)`; throw if problem.
GDValue mapGetValueAtStr_parse(GDValue const &v, char const *str);

// Return `v.mapGetValueAt(str)`, i.e., `str` is a string (not the name
// of a symbol).  If `v` is a map but `str` is not mapped, return a null
// `GDValue`.  If it is not a map, throw.
//
// Regarding naming: The name that would be parallel with
// "mapGetSym_parseOpt" is "mapGetStr_parseOpt", but for both, I think
// the type (symbol or string) could be confused with the value
// *returned*.  So I'm breaking the parallelism for added clarity here.
// I might rename the other one at some point.
//
GDValue mapGetValueAtStr_parseOpt(GDValue const &v, char const *str);


// ------------------------------- GDVTo -------------------------------
// Declaration of a class template that is meant to be specialized to
// create a set of functions to convert from GDValue to various other
// types.  This is intended to convert from the obvious kind of GDValue
// that would naturally be used for serialization, rather than doing
// ad-hoc coercions.
//
// Specializations are expected contain one static method called `f`
// with signature:
//
//   static T f(GDValue const &v);
//
// The use of a class template rather than a function template is
// necessitated because we need to partially specialize this to handle
// things like ASTList<>, but partial specialization of function
// templates is not allowed.
//
// Hmmm, what was the problem with overloading?  I can't remember, and
// may have been mistaken.
//
template <typename T, typename Enable = void>
struct GDVTo {};


template <>
struct GDVTo<bool> {
  // Requires that `v` be the symbol `true` or `false`.
  static bool f(GDValue const &v);
};


template <>
struct GDVTo<int> {
  // Requires that `v` be a small integer.
  static int f(GDValue const &v);
};

// Convert from a string.
template <>
struct GDVTo<std::string> {
  // Requires that `v` be a string.
  static std::string f(GDValue const &v);
};

// If `T` is final and can be constructed from `GDValue`, we should be
// able to safely create it directly.
template <typename T>
struct GDVTo<T,
             std::enable_if_t<
               std::is_final<T>::value &&
               std::is_constructible<T, GDValue>::value>
            > {
  static T f(GDValue const &v)
  {
    return T(v);
  }
};


// Syntactic convenience for calling the above.
template <typename T>
T gdvTo(GDValue const &v)
{
  return GDVTo<T>::f(v);
}


// If `v` is null, then return a default-constructed `T`.  Otherwise
// convert it normally.
//
// This is defined in `gdvalue-parse-ops.h` since it requires a
// definition for `GDValue`.
template <typename T>
inline T gdvOptTo(GDValue const &v);


// This is similar to `gdvTo`, except it returns a newly allocated
// object.  This is particularly useful when `T` is a superclass, and
// the contents of `v` must be inspected to determine which subclass to
// create.
//
// Specializations should contain a method with signature:
//
//   static T *f(GDValue const &v);
//
template <typename T, typename Enable = void>
struct GDVToNew {};


// If `T` is final, we should be able to safely use `new` directly.
template <typename T>
struct GDVToNew<T, std::enable_if_t<std::is_final<T>::value>> {
  static T *f(GDValue const &v)
  {
    return new T(v);
  }
};


// Syntactic convenience for calling the above.
template <typename T>
T *gdvToNew(GDValue const &v)
{
  return GDVToNew<T>::f(v);
}


// ---------------------- Member de/serialization ----------------------
// If `name` begins with "m_", return `name+2`, thus stripping the
// prefix.  Otherwise return it unchanged.
char const *stripMemberPrefix(char const *name);

// Write `<memb>` to a field of GDValue `m` that has the same name
// except without the "m_" prefix (if any).
#define GDV_WRITE_MEMBER(memb) \
  m.mapSetSym(gdv::stripMemberPrefix(#memb), gdv::toGDValue(memb)) /* user ; */

// Initialize `<memb>` from an optional field `<memb>` of GDValue `m`
// that has the same name except without the "m_" prefix.
#define GDV_READ_MEMBER(memb) \
  memb(gdv::gdvOptTo<decltype(memb)>(gdv::mapGetSym_parseOpt(m, gdv::stripMemberPrefix(#memb)))) /* user , */

// Same, but the key is a string rather than a symbol.  The suffix "_SK"
// means "string key".
#define GDV_WRITE_MEMBER_SK(memb) \
  m.mapSetValueAt(gdv::stripMemberPrefix(#memb), gdv::toGDValue(memb)) /* user , */

#define GDV_READ_MEMBER_SK(memb) \
  memb(gdv::gdvTo<decltype(memb)>(gdv::mapGetValueAtStr_parseOpt(m, gdv::stripMemberPrefix(#memb)))) /* user , */


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_PARSE_H
