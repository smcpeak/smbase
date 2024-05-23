// gdvalue.h
// General Data Value: integer, sequence, map, etc.

/* The basic idea is to represent general-purpose data, made up of a
   few common primitives and containers, for the purpose of interchange
   between systems.  The data model is loosely based on JSON, but with
   a number of fixes.  The text serialization format is inspired by both
   JSON and s-expressions.  See gdvalue-design.txt for more information.
*/

#ifndef SMBASE_GDVALUE_H
#define SMBASE_GDVALUE_H

#include "gdvalue-fwd.h"               // fwds for this module

// this dir
#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "gdvalue-write-options.h"     // gdv::GDValueWriteOptions
#include "gdvsymbol.h"                 // gdv::GDVSymbol

// libc++
#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::int64_t
#include <iosfwd>                      // std::ostream
#include <map>                         // std::map
#include <set>                         // std::set
#include <string>                      // std::string
#include <vector>                      // std::vector
#include <utility>                     // std::pair


namespace gdv {


// Count of elements.
using GDVSize = std::size_t;

// Index for vectors.
using GDVIndex = std::size_t;

// The type that a GDValue(GDVK_BOOL) holds.
using GDVBool = bool;

// GDValue(GDVK_INTEGER) holds this.  It should eventually be an
// arbitrary-precision integer.
using GDVInteger = std::int64_t;

// GDValue(GDVK_STRING) holds this.  It is a UTF-8 encoding of the
// sequence of Unicode code points the string represents.
using GDVString = std::string;

//using GDVOctetSequence = std::vector<unsigned char>;

// GDValue(GDVK_SEQUENCE) holds this.
using GDVSequence = std::vector<GDValue>;

// GDValue(GDVK_SET) holds this.
using GDVSet = std::set<GDValue>;

// GDValue(GDVK_MAP) holds this.
using GDVMap = std::map<GDValue, GDValue>;

// The entry type for GDVMap.
using GDVMapEntry = std::pair<GDValue const, GDValue>;


// Possible kinds of GDValues.
enum GDValueKind : int {
  // The "null" value, the only inhabitant of the "null" type.
  GDVK_NULL,

  // bool: True or false.
  GDVK_BOOL,

  // integer: int64_t for now, but arbitrary precision in the future
  GDVK_INTEGER,

  // symbol: An identifier-like string that acts as a name of something
  // defined elsewhere.
  GDVK_SYMBOL,

  // string: Sequence of Unicode characters encoded as UTF-8.
  GDVK_STRING,

  // sequence: Ordered sequence of values.
  GDVK_SEQUENCE,

  // set: Unordered set of (unique) values.
  GDVK_SET,

  // map: Set of (key, value) pairs that are indexed by key.
  GDVK_MAP,

  NUM_GDVALUE_KINDS
};

// Return a string like "GDVK_NULL", or "GDVK_invalid" if 'gdvk' is
// invalid.
char const *toString(GDValueKind gdvk);


// A General Data Value is a disjoint union of several different types
// of data, enumerated as 'GDValueKind'.
class GDValue {
private:     // data
  // Tag indicating which kind of value is represented.
  GDValueKind m_kind;

  // Representation of the value.
  union GDValueUnion {
    GDVBool      m_bool;
    GDVInteger   m_int64;

    // Non-owning pointer to a NUL-terminated string stored in
    // 'GDVSymbol::s_stringTable'.
    char const  *m_symbolName;

    // These are all owner pointers.
    GDVString   *m_string;
    GDVSequence *m_sequence;
    GDVSet      *m_set;
    GDVMap      *m_map;

    GDValueUnion() : m_int64(0) {}
  } m_value;

public:      // static data
  // Expose some method counts for testing purposes.
  static unsigned s_ct_ctorDefault;
  static unsigned s_ct_dtor;
  static unsigned s_ct_ctorCopy;
  static unsigned s_ct_ctorMove;
  static unsigned s_ct_assignCopy;
  static unsigned s_ct_assignMove;
  static unsigned s_ct_valueKindCtor;
  static unsigned s_ct_boolCtor;
  static unsigned s_ct_integerCtor;
  static unsigned s_ct_symbolCtor;
  static unsigned s_ct_stringCtorCopy;
  static unsigned s_ct_stringCtorMove;
  static unsigned s_ct_stringSetCopy;
  static unsigned s_ct_stringSetMove;
  static unsigned s_ct_sequenceCtorCopy;
  static unsigned s_ct_sequenceCtorMove;
  static unsigned s_ct_sequenceSetCopy;
  static unsigned s_ct_sequenceSetMove;
  static unsigned s_ct_setCtorCopy;
  static unsigned s_ct_setCtorMove;
  static unsigned s_ct_setSetCopy;
  static unsigned s_ct_setSetMove;
  static unsigned s_ct_mapCtorCopy;
  static unsigned s_ct_mapCtorMove;
  static unsigned s_ct_mapSetCopy;
  static unsigned s_ct_mapSetMove;

private:     // methods
  // Reset the object to 'null' without deallocating anything that
  // 'm_value' might point at.  This is done as part of certain
  // low-level transfers.
  void resetWithoutDeallocating() noexcept;

  // Clear this object, then take the data in 'obj', leaving 'obj' as
  // the null value.
  void clearSelfAndSwapWith(GDValue &obj) noexcept;

public:      // methods
  // Make a null value.
  GDValue() noexcept;

  ~GDValue();

  GDValue(GDValue const &obj);
  GDValue(GDValue      &&obj);

  GDValue &operator=(GDValue const &obj);
  GDValue &operator=(GDValue      &&obj);


  // Make an empty/zero value of 'kind'.
  explicit GDValue(GDValueKind kind);


  GDValueKind getKind() const { return m_kind; }
  bool isNull()     const { return getKind() == GDVK_NULL;     }
  bool isBool()     const { return getKind() == GDVK_BOOL;     }
  bool isInteger()  const { return getKind() == GDVK_INTEGER;  }
  bool isSymbol()   const { return getKind() == GDVK_SYMBOL;   }
  bool isString()   const { return getKind() == GDVK_STRING;   }
  bool isSequence() const { return getKind() == GDVK_SEQUENCE; }
  bool isSet()      const { return getKind() == GDVK_SET;      }
  bool isMap()      const { return getKind() == GDVK_MAP;      }


  // Return <0 if a<b, 0 if a==b, and >0 otherwise.
  friend int compare(GDValue const &a, GDValue const &b);

  // Define operator==, etc.
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDValue)

  // Return the sum of all of the 's_ct_XXXCtorXXX' counts.
  static unsigned countConstructorCalls();


  // Number of contained values.  Result depends on kind of value:
  //   - null: 0
  //   - bool, int, symbol, string: 1
  //   - sequence, set: number of elements
  //   - map: number of entries
  GDVSize size() const;

  // True if 'size()' is zero.
  bool empty() const;

  // Reset to null.
  void clear();

  // Exchange values with 'obj'.
  void swap(GDValue &obj) noexcept;


  // ---- Write as text ----
  // Write as text to 'os'.  By default this does not use any
  // indentation.
  void write(std::ostream &os,
             GDValueWriteOptions options = GDValueWriteOptions()) const;

  friend std::ostream &operator<<(std::ostream &os, GDValue const &v)
    { v.write(os); return os; }

  // Use 'write' to create a string.
  std::string asString(
    GDValueWriteOptions options = GDValueWriteOptions()) const;

  // Enable indentation in the write options, then write to 'os', then
  // write a final newline.
  void writeLines(std::ostream &os,
                  GDValueWriteOptions options = GDValueWriteOptions()) const;

  // Capture what 'writeLines' would write as a string.
  std::string asLinesString(
    GDValueWriteOptions options = GDValueWriteOptions()) const;

  // Write the value to 'fileName', terminated by a final newline.
  // Throw an exception if the file cannot be written.
  void writeToFile(std::string const &fileName,
                   GDValueWriteOptions options = GDValueWriteOptions()) const;


  // ---- Read as text ----
  // Read the next value from 'is'.  It must read enough to determine
  // that the value is complete, and will block if it is not.  It will
  // leave the input stream at the character after the last in the
  // value, typically using istream::putback to do that.
  //
  // If there is no value before EOF, this returns nullopt.
  //
  // If a syntax error is encountered, throws 'GDValueReaderException'
  // (declared in gdvalue-reader-exception.h).
  //
  static std::optional<GDValue> readNextValue(std::istream &is);

  // Read a single serialized value from 'is', throwing an exception if
  // there is not exactly one value before EOF or it is malformed.
  static GDValue readFromStream(std::istream &is);

  // Read the single serialized value in 'str', throwing an exception if
  // there is not exactly one value orit is malformed.
  static GDValue readFromString(std::string const &str);

  // Read the single value stored in 'fileName', throwing an exception
  // if it cannot be opened, there is not exactly one value, or is
  // malformed.
  static GDValue readFromFile(std::string const &fileName);


  // ---- Boolean ----
  // A constructor that just accepts 'bool' creates too many ambiguities
  // so we use a discriminator tag.
  enum BoolTagType { BoolTag };
  GDValue(BoolTagType, bool b);

  void boolSet(bool b);

  bool boolGet() const;


  // ---- Integer ----
  // The GDValue ctors are safe to use implicitly because they are
  // merely passive containers for data that preserve the information
  // passed as arguments.  Furthermore, making them explicit (which I
  // initially did) *greatly* expands the verbosity and difficulty of
  // reading initializers for complex values.
  /*implicit*/ GDValue(GDVInteger i);

  void integerSet(GDVInteger i);

  GDVInteger integerGet() const;


  // ---- Symbol ----
  /*implicit*/ GDValue(GDVSymbol sym);

  void symbolSet(GDVSymbol sym);

  GDVSymbol symbolGet() const;


  // ---- String ----
  /*implicit*/ GDValue(GDVString const &str);
  /*implicit*/ GDValue(GDVString      &&str);

  // Accept string literals.  But doing so in the obvious way causes
  // ambiguity with the constructor that accepts GDVInteger with an
  // argument of "0".  In order to prefer the GDVInteger interpretation,
  // make the ctor that accepts a pointer be a template, but delete the
  // general form.
  template <typename T>
  /*implicit*/ GDValue(T const *str) = delete;

  // Then define the actual constructor I want as a specialization.
  //
  // Hmmm, it seems that GCC does not allow this.
  // https://stackoverflow.com/questions/49707184/explicit-specialization-in-non-namespace-scope-does-not-compile-in-gcc
  //
  // Moving the declaration to namespace scope...
  //template <>
  ///*implicit*/ GDValue(char const *str);

  void stringSet(GDVString const &str);
  void stringSet(GDVString      &&str);

  GDVString const &stringGet()        const;

  // I don't know if I want the 'getMutable' functions to be part of the
  // API.  For the moment they should be treated as an implementation
  // helper and a stop-gap in case an important operation is missing.
  GDVString       &stringGetMutable()      ;

  // Declare the iterators for a particular kind of GDValue.
  #define DECLARE_GDV_KIND_ITERATORS(GDVKindName, kindName)    \
    /* Explicitly const begin/end. */                          \
    GDVKindName::const_iterator kindName##CBegin() const;      \
    GDVKindName::const_iterator kindName##CEnd() const;        \
                                                               \
    /* begin/end const overloads. */                           \
    GDVKindName::const_iterator kindName##Begin() const        \
      { return                  kindName##CBegin(); }          \
    GDVKindName::const_iterator kindName##End() const          \
      { return                  kindName##CEnd(); }            \
                                                               \
    /* begin/end non-const overloads. */                       \
    GDVKindName::iterator       kindName##Begin();             \
    GDVKindName::iterator       kindName##End();               \
                                                               \
    /* Objects for use in range-based 'for' loops. */          \
    inline GDVKindName##IterableC kindName##IterableC() const; \
    inline GDVKindName##IterableC kindName##Iterable()  const; \
    inline GDVKindName##Iterable  kindName##Iterable()       ;

  // Declare string[C]{Begin,End} and stringIterable[C].
  DECLARE_GDV_KIND_ITERATORS(GDVString, string)


  // ---- Sequence ----
  /*implicit*/ GDValue(GDVSequence const &seq);
  /*implicit*/ GDValue(GDVSequence      &&seq);

  void sequenceSet(GDVSequence const &seq);
  void sequenceSet(GDVSequence      &&seq);

  GDVSequence const &sequenceGet()        const;
  GDVSequence       &sequenceGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVSequence, sequence)

  void sequenceAppend(GDValue value);

  // Discard extra elements or pad with nulls to match the size.
  void sequenceResize(GDVSize newSize);

  void sequenceSetValueAt(GDVIndex index, GDValue const &value);
  void sequenceSetValueAt(GDVIndex index, GDValue      &&value);

  GDValue const &sequenceGetValueAt(GDVIndex index) const;
  GDValue       &sequenceGetValueAt(GDVIndex index)      ;

  void sequenceClear();


  // ---- Set ---
  /*implicit*/ GDValue(GDVSet const &set);
  /*implicit*/ GDValue(GDVSet      &&set);

  void setSet(GDVSet const &set);
  void setSet(GDVSet      &&set);

  GDVSet const &setGet()        const;
  GDVSet       &setGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVSet, set)

  bool setContains(GDValue const &elt) const;

  // True if the element was inserted, false if it was already there.
  bool setInsert(GDValue const &elt);
  bool setInsert(GDValue      &&elt);

  // True if the element was removed, false if it was not there.
  bool setRemove(GDValue const &elt);

  void setClear();


  // ---- Map ----
  /*implicit*/ GDValue(GDVMap const &map);
  /*implicit*/ GDValue(GDVMap      &&map);

  void mapSet(GDVMap const &map);
  void mapSet(GDVMap      &&map);

  GDVMap const &mapGet()        const;
  GDVMap       &mapGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVMap, map)

  bool mapContains(GDValue const &key) const;

  // Requires that the key be mapped.
  GDValue const &mapGetValueAt(GDValue const &key) const;
  GDValue       &mapGetValueAt(GDValue const &key)      ;

  void mapSetValueAt(GDValue const &key, GDValue const &value);
  void mapSetValueAt(GDValue      &&key, GDValue      &&value);

  bool mapRemoveKey(GDValue const &key);

  void mapClear();


  #undef DECLARE_GDV_KIND_ITERATORS
};


// Declare the ctor specialization that GCC does not like to have inside
// the class body.
template <>
/*implicit*/ GDValue::GDValue(char const *str);


#define DEFINE_GDV_KIND_ITERABLE(GDVKindName, kindName)              \
  /* Helper for use with const range-based 'for' loops. */           \
  class GDVKindName##IterableC {                                     \
  public:      /* data */                                            \
    /* The value to iterate over as a GDVKindName. */                \
    GDValue const &m_value;                                          \
                                                                     \
  public:      /* methods */                                         \
    explicit GDVKindName##IterableC(GDValue const &value)            \
      : m_value(value)                                               \
    {}                                                               \
                                                                     \
    GDVKindName::const_iterator begin() const                        \
      { return m_value.kindName##CBegin(); }                         \
    GDVKindName::const_iterator end() const                          \
      { return m_value.kindName##CEnd(); }                           \
  };                                                                 \
                                                                     \
                                                                     \
  /* Helper for use with non-const range-based 'for' loops. */       \
  class GDVKindName##Iterable {                                      \
  public:      /* data */                                            \
    /* The value to iterate over as a GDVKindName. */                \
    GDValue &m_value;                                                \
                                                                     \
  public:      /* methods */                                         \
    explicit GDVKindName##Iterable(GDValue &value)                   \
      : m_value(value)                                               \
    {}                                                               \
                                                                     \
    GDVKindName::iterator begin() const                              \
      { return m_value.kindName##Begin(); }                          \
    GDVKindName::iterator end() const                                \
      { return m_value.kindName##End(); }                            \
  };                                                                 \
                                                                     \
                                                                     \
  inline GDVKindName##IterableC GDValue::kindName##IterableC() const \
  {                                                                  \
    return GDVKindName##IterableC(*this);                            \
  }                                                                  \
                                                                     \
  inline GDVKindName##IterableC GDValue::kindName##Iterable() const  \
  {                                                                  \
    return GDVKindName##IterableC(*this);                            \
  }                                                                  \
                                                                     \
  inline GDVKindName##Iterable GDValue::kindName##Iterable()         \
  {                                                                  \
    return GDVKindName##Iterable(*this);                             \
  }


// Define classes GDVStringIterable[C] and methods
// GDValue::stringIterable[C].
DEFINE_GDV_KIND_ITERABLE(GDVString, string)

DEFINE_GDV_KIND_ITERABLE(GDVSequence, sequence)

DEFINE_GDV_KIND_ITERABLE(GDVSet, set)


#undef DEFINE_GDV_KIND_ITERABLE


} // namespace gdv


#endif // SMBASE_GDVALUE_H
