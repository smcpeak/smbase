// gdvalue-parser.h
// `GDValueParser` class.

// See license.txt for copyright and terms of use.

// This is similar to `gdvalue-parse`.  I'm trying a different approach,
// and might entirely replace that module with this one.
//
// The main idea here is that, instead of passing around references to
// `GDValue`, we wrap that in a `GDValueParser` that keeps track of the
// access path that got to the current value.  That way if there is a
// problem we can report the location.

#ifndef SMBASE_GDVALUE_PARSER_H
#define SMBASE_GDVALUE_PARSER_H

#include "smbase/gdvalue-parser-fwd.h"           // fwds for this file

#include "smbase/exc.h"                          // smbase::XBase
#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue
#include "smbase/gdvalue-kind.h"                 // gdv::GDValueKind
#include "smbase/gdvalue-types.h"                // gdv::GDVIndex
#include "smbase/gdvsymbol-fwd.h"                // gdv::GDVSymbol
#include "smbase/gdvtuple-fwd.h"                 // gdv::GDVTuple
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NORETURN

#include "smbase/std-optional-fwd.h"             // std::optional
#include "smbase/std-string-view-fwd.h"          // std::string_view

#include <string>                                // std::string
#include <type_traits>                           // std::{enable_if_t, is_final, is_constructible}
#include <vector>                                // std::vector


OPEN_NAMESPACE(gdv)


// A single step that traverses from a parent `GDValue` container to one
// of its children.  The exact interpretation depends on what kind of
// container the parent is.
class GDVNavStep {
private:     // types
  // Select which union element is active.
  enum StepKind {
    SK_INDEX,
    SK_KEY,
    SK_VALUE,
  };

public:      // types
  // Tags to allow selecting the desired constructor overload.
  enum StepIsIndex { STEP_IS_INDEX };
  enum StepIsKey   { STEP_IS_KEY };
  enum StepIsValue { STEP_IS_VALUE };

private:     // data
  StepKind m_kind;

  union {
    // If `m_kind==SK_INDEX`, this provides the numerical index into a
    // sequence or tuple.
    GDVIndex m_index;

    // If `m_kind==SK_KEY`, this points at either a value stored in a
    // set, or a key stored in a map, and the navigation step goes to
    // that stored key value.
    GDValue const *m_key;

    // If `m_kind==SK_VALUE`, the pointer's meaning is the same as in
    // the `m_key` case, but the navigation step goes to the stored
    // *value* of a map.
    GDValue const *m_value;

    // Of course, it is not necessary to split `m_key` and `m_value`
    // since they have the same type, but I do so for uniformity.
  };

private:     // methods
  // Copy the active union member from `obj`.
  void copyUnionMember(GDVNavStep const &obj);

public:      // methods
  explicit GDVNavStep(StepIsIndex, GDVIndex index);
  explicit GDVNavStep(StepIsKey, GDValue const *key);
  explicit GDVNavStep(StepIsValue, GDValue const *value);

  GDVNavStep(GDVNavStep const &obj);

  // I currently have no need of this.  There is an implementation in
  // the .cc file but it is commented out due to being untested.
  GDVNavStep &operator=(GDVNavStep const &obj) = delete;

  // For the index case, return "[n]".  For the value case, return the
  // value as GDVN, preceded by ".".
  std::string asString() const;

  // Apply this navigation step to `parent` to get a child.
  //
  // This function asserts the step is incompatible with the parent
  // value kind or the index/value is not valid.  It does not throw
  // a recoverable exception because a mismatch here is due to a bug in
  // the parsing code, not an unexpected GDValue.
  //
  GDValue const *getSpecifiedChild(GDValue const *parent) const;
};


// An instance of this class has a reference to a particular `GDValue`
// to be parsed, as well as a navigation path from a top-level `GDValue`
// that was the starting point of the parsing effort.  The purpose is to
// be able to specify where in a GDV structure an error occurred when
// there is a problem.
//
// This does not use things like line/col because a GDV does not have to
// have come from GDVN.  Among other things, it could have come from
// JSON instead.
//
// Methods in this class throw `XGDValueError`.
//
class GDValueParser {
public:      // class data
  // When true, every constructor calls `selfCheck()`.  (There are no
  // non-const member functions, so checking during construction is
  // sufficient.)
  //
  // This is meant for use during unit testing, as it has signficant
  // performance cost.  Default is false.
  static bool s_selfCheckCtors;

private:     // instance data
  // The entire `GDValue` we are parsing.  This object, and all if its
  // children, must not be changed while the parser object is active.
  //
  // This (and `m_value`) is set to null by `clearParserPointers`, but
  // that is a mostly invalid state, so I do not annotate these with
  // `NULLABLE`; all methods aside from the destructor should assume
  // these pointers are valid.
  //
  GDValue const *m_topLevel;

  // The value to be parsed by the code receiving this parser.  It is
  // somewhere inside `m_topLevel`.
  GDValue const *m_value;

  // The navigation path from `m_topLevel` to `m_value`.  If this is
  // empty, then both are references to the same object.
  std::vector<GDVNavStep> m_path;

public:      // methods
  ~GDValueParser();

  GDValueParser(GDValueParser const &obj);
  GDValueParser(GDValueParser      &&obj);

  // Start a new parser.
  explicit GDValueParser(GDValue const &topLevel);

  // Make a parser by navigating from `parent` by `step`.
  explicit GDValueParser(GDValueParser const &parent, GDVNavStep step);

  // Read-only access to members.
  GDValue const &getTopLevel() const { return *m_topLevel; }
  GDValue const &getValue() const { return *m_value; }
  std::vector<GDVNavStep> const &getPath() const { return m_path; }

  // Stringify the value as GDVN.
  std::string valueGDVN() const;

  // Render the path as a string.
  std::string pathString() const;

  // Assert that the path is accurate.
  void selfCheck() const;

  // The object to which this parser refers is about to be modified, so
  // nullify all of our pointers.  If anything is done with this parser
  // object (other than destroying it), we will crash; but that is
  // better than use after free.
  void clearParserPointers();

  // Queries on the current value, with same semantics as the same-named
  // methods on `GDValue`.
  GDValueKind getKind() const;
  char const *getKindName() const;
  char const *getKindCommonName() const;
  GDValueKind getSuperKind() const;
  bool isSymbol() const;
  bool isInteger() const;
  bool isSmallInteger() const;
  bool isString() const;
  bool isSequence() const;
  bool isTaggedSequence() const;
  bool isTuple() const;
  bool isTaggedTuple() const;
  bool isSet() const;
  bool isTaggedSet() const;
  bool isMap() const;
  bool isTaggedMap() const;
  bool isOrderedMap() const;
  bool isTaggedOrderedMap() const;
  bool isPOMap() const;
  bool isTaggedPOMap() const;
  bool isContainer() const;
  bool isTaggedContainer() const;
  bool isOrderedContainer() const;
  bool isUnorderedContainer() const;

  // Throw `XGDValueError` with this parser as context.
  void throwError(std::string &&msg) const NORETURN;

  // Throw `XGDValueError` if the current value does not have kind
  // `kind`.
  void checkKind(GDValueKind kind) const;

  // ---- Symbol ----
  void checkIsSymbol() const;
  bool isNull() const;
  bool isBool() const;
  bool boolGet() const;
  GDVSymbol symbolGet() const;
  std::string_view symbolGetName() const;

  // ---- Integer ----
  void checkIsInteger() const;
  GDVInteger integerGet() const;
  bool integerIsNegative() const;
  GDVInteger const &largeIntegerGet() const;

  // ---- SmallInteger ----
  void checkIsSmallInteger() const;
  GDVSmallInteger smallIntegerGet() const;

  // ---- String ----
  void checkIsString() const;
  GDVString const &stringGet() const;

  // ---- Container ----
  void checkIsContainer() const;
  GDVSize containerSize() const;
  bool containerIsEmpty() const;

  // ---- Sequence ----
  void checkIsSequence() const;
  GDVSequence const &sequenceGet() const;
  GDValueParser sequenceGetValueAt(GDVIndex index) const;

  // ---- Tuple ----
  void checkIsTuple() const;
  GDVTuple const &tupleGet() const;
  GDValueParser tupleGetValueAt(GDVIndex index) const;

  // ---- Set ----
  void checkIsSet() const;
  GDVSet const &setGet() const;
  bool setContains(GDValue const &elt) const;
  GDValueParser setGetValue(GDValue const &elt) const;

  // ---- Map ----
  // False for an ordered map.  Use `checkIsPOMap()` to allow both.
  void checkIsMap() const;

  // Does not work for ordered map.
  GDVMap const &mapGet() const;

  // The rest of these *do* work for ordered maps.
  bool mapContains(GDValue const &key) const;
  GDValueParser mapGetKeyAt(GDValue const &key) const;
  GDValueParser mapGetValueAt(GDValue const &key) const;
  bool mapContainsSym(char const *symName) const;
  GDValueParser mapGetValueAtSym(char const *symName) const;

  // Use a string as a key.
  GDValueParser mapGetValueAtStr(char const *str) const;

  // Return `nullopt` if the key is not mapped.
  std::optional<GDValueParser> mapGetValueAtOpt(GDValue const &key) const;
  std::optional<GDValueParser> mapGetValueAtSymOpt(char const *symName) const;
  std::optional<GDValueParser> mapGetValueAtStrOpt(char const *str) const;

  // ---- OrderedMap ----
  void checkIsOrderedMap() const;
  void checkIsPOMap() const;
  GDVOrderedMap const &orderedMapGet() const;
  bool orderedMapContains(GDValue const &key) const;
  GDValueParser orderedMapGetKeyAt(GDValue const &key) const;
  GDValueParser orderedMapGetValueAt(GDValue const &key) const;
  bool orderedMapContainsSym(char const *symName) const;
  GDValueParser orderedMapGetValueAtSym(char const *symName) const;

  // ---- TaggedContainer ----
  void checkIsTaggedContainer() const;
  GDVSymbol taggedContainerGetTag() const;
  std::string_view taggedContainerGetTagName() const;

  // Check that the tag is a symbol with `symName`.
  void checkContainerTag(char const *symName) const;

  // ---- Tagged Map ----
  void checkIsTaggedMap() const;
  void checkTaggedMapTag(char const *symName) const;

  // ---- Tagged OrderedMap ----
  void checkIsTaggedOrderedMap() const;
  void checkTaggedOrderedMapTag(char const *symName) const;
};


// Thrown when a `GDValue` differs from what was expected.
class XGDValueError : public smbase::XBase {
public:      // data
  // Note: It is not possible to carry the `GDValueParser` here, nor any
  // of its elements, because they all point into the toplevel `GDValue`
  // being parsed, but that object's lifetime may end before this
  // exception is caught.

  // GDV navigation path to the offending object.
  std::string m_path;

  // The conflict between what was expected and what was actually found
  // in the primary value in `m_parser`.
  std::string m_message;

public:      // methods
  ~XGDValueError();

  explicit XGDValueError(
    std::string &&path,
    std::string &&message);

  XGDValueError(XGDValueError const &obj);

  // This combines information in `m_parser` with `m_conflict`.
  virtual std::string getConflict() const override;
};


// ------------------------------ GDVPTo -------------------------------
// Declaration of a class template that is meant to be specialized to
// create a set of functions to convert from GDValueParser to various
// other types.  This is intended to convert from the obvious kind of
// GDValue that would naturally be used for serialization, rather than
// doing ad-hoc coercions.
//
// Specializations are expected contain one static method called `f`
// with signature:
//
//   static T f(GDValueParser const &p);
//
// The use of a class template rather than a function template is
// necessitated because we need to partially specialize this to handle
// things like ASTList<>, but partial specialization of function
// templates is not allowed.  (Ordinary overloading does not work
// because I need to be able to supply explict template arguments to
// select the conversion I want.)
//
template <typename T, typename Enable /*= void*/>
struct GDVPTo {};


template <>
struct GDVPTo<bool> {
  // Requires that `p` be the symbol `true` or `false`.
  static bool f(GDValueParser const &p);
};


template <>
struct GDVPTo<int> {
  // Requires that `p` be a small integer.
  static int f(GDValueParser const &p);
};


template <>
struct GDVPTo<std::string> {
  // Requires that `v` be a string.
  static std::string f(GDValueParser const &p);
};


// If `T` is final and can be constructed from `GDValueParser`, we
// should be able to safely create it directly.  The requirement for
// `final` is to avoid constructing a superclass when the value
// specifies to make a subclass.
template <typename T>
struct GDVPTo<T,
              std::enable_if_t<
                std::is_final<T>::value &&
                std::is_constructible<T, GDValueParser>::value>
             > {
  static T f(GDValueParser const &p)
  {
    return T(p);
  }
};



// Syntactic convenience for calling the above.
template <typename T>
T gdvpTo(GDValueParser const &p)
{
  return GDVPTo<T>::f(p);
}


// If `p` is nullopt, then return a default-constructed `T`.  Otherwise
// convert it normally.
//
// Defined in `gdvalue-parser-ops.h`.
template <typename T>
inline T gdvpOptTo(std::optional<GDValueParser> const &p);


// This is similar to `gdvpTo`, except it returns a newly allocated
// object.  This is particularly useful when `T` is a superclass, and
// the contents of `p` must be inspected to determine which subclass to
// create.
//
// Specializations should contain a method with signature:
//
//   static T *f(GDValueParser const &p);
//
template <typename T, typename Enable /*= void*/>
struct GDVPToNew {};


// If `T` is final, we should be able to safely use `new` directly.
template <typename T>
struct GDVPToNew<T, std::enable_if_t<std::is_final<T>::value>> {
  static T *f(GDValueParser const &p)
  {
    return new T(p);
  }
};


// Syntactic convenience for calling the above.
template <typename T>
T *gdvpToNew(GDValueParser const &p)
{
  return GDVPToNew<T>::f(p);
}


// ---------------------- Member deserialization -----------------------
// Initialize `<memb>` from a field `<memb>` of GDValueParser `p` that
// has the same name except without the "m_" prefix.
#define GDVP_READ_MEMBER_SYM(memb) \
  memb(gdv::gdvpTo<decltype(memb)>(p.mapGetValueAtSym(gdv::stripMemberPrefix(#memb)))) /* user , */

// Same, but using a string as a key.
#define GDVP_READ_MEMBER_STR(memb) \
  memb(gdv::gdvpTo<decltype(memb)>(p.mapGetValueAtStr(gdv::stripMemberPrefix(#memb)))) /* user , */


// Initialize `<memb>` from an optional field `<memb>` of GDValueParser
// `p` that has the same name except without the "m_" prefix.
#define GDVP_READ_OPT_MEMBER_SYM(memb) \
  memb(gdv::gdvpOptTo<decltype(memb)>(p.mapGetValueAtSymOpt(gdv::stripMemberPrefix(#memb)))) /* user , */

// This one uses a string key.
#define GDVP_READ_OPT_MEMBER_STR(memb) \
  memb(gdv::gdvpOptTo<decltype(memb)>(p.mapGetValueAtStrOpt(gdv::stripMemberPrefix(#memb)))) /* user , */


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_PARSER_H
