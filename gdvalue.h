// gdvalue.h
// General Data Value.

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
#include "gdvsymbol-fwd.h"             // gdv::GDVSymbol [n]

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

//using GDVOctetVector = std::vector<unsigned char>;

// GDValue(GDVK_VECTOR) holds this.
using GDVVector = std::vector<GDValue>;

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

  // vector: Ordered sequence of values.
  GDVK_VECTOR,

  // set: Unordered set of (unique) values.
  GDVK_SET,

  // map: Set of (key, value) pairs that are indexed by key.
  GDVK_MAP,

  NUM_GDVALUE_KINDS
};

// Return a string like "GDVK_NULL", or "GDVK_invalid" if 'gdvk' is
// invalid.
char const *toString(GDValueKind gdvk);


// 'GDVALUE_CORE_CTOR' is placed before a "core" constructor
// declaration, meaning one that creates a GDValue from the type of data
// it wraps.  I'm experimenting with making those non-explicit.
#if 0
  #define GDVALUE_CORE_CTOR explicit
#else
  #define GDVALUE_CORE_CTOR /*nothing*/
#endif


// A General Data Value is a disjoint union of several different types
// of data, enumerated as 'GDValueKind'.
class GDValue {
private:     // data
  // Tag indicating which kind of value is represented.
  GDValueKind m_kind;

  // Representation of the value.
  union GDValueUnion {
    GDVBool    m_bool;
    GDVInteger m_int64;

    // This is for now an owner pointer.
    GDVSymbol *m_symbol;

    // These are all owner pointers.
    GDVString *m_string;
    GDVVector *m_vector;
    GDVSet    *m_set;
    GDVMap    *m_map;

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
  static unsigned s_ct_vectorCtorCopy;
  static unsigned s_ct_vectorCtorMove;
  static unsigned s_ct_vectorSetCopy;
  static unsigned s_ct_vectorSetMove;
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
  bool isNull()    const { return getKind() == GDVK_NULL;    }
  bool isBool()    const { return getKind() == GDVK_BOOL;    }
  bool isInteger() const { return getKind() == GDVK_INTEGER; }
  bool isSymbol()  const { return getKind() == GDVK_SYMBOL;  }
  bool isString()  const { return getKind() == GDVK_STRING;  }
  bool isVector()  const { return getKind() == GDVK_VECTOR;  }
  bool isSet()     const { return getKind() == GDVK_SET;     }
  bool isMap()     const { return getKind() == GDVK_MAP;     }


  // Return <0 if a<b, 0 if a==b, and >0 otherwise.
  friend int compare(GDValue const &a, GDValue const &b);

  // Define operator==, etc.
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDValue)

  // Return the sum of all of the 's_ct_XXXCtorXXX' counts.
  static unsigned countConstructorCalls();


  // Number of contained values.  Result depends on kind of value:
  //   - null: 0
  //   - bool, int, symbol, string: 1
  //   - vector, set: number of elements
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


  // ---- Boolean ----
  // A constructor that just accepts 'bool' creates too many ambiguities
  // so we use a discriminator tag.
  enum BoolTagType { BoolTag };
  GDValue(BoolTagType, bool b);

  void boolSet(bool b);

  bool boolGet() const;


  // ---- Integer ----
  GDVALUE_CORE_CTOR GDValue(GDVInteger i);

  void integerSet(GDVInteger i);

  GDVInteger integerGet() const;


  // ---- Symbol ----
  GDVALUE_CORE_CTOR GDValue(GDVSymbol sym);

  void symbolSet(GDVSymbol sym);

  GDVSymbol symbolGet() const;


  // ---- String ----
  GDVALUE_CORE_CTOR GDValue(GDVString const &str);
  GDVALUE_CORE_CTOR GDValue(GDVString      &&str);

  // Accept string literals.  But doing so in the obvious way causes
  // ambiguity with the constructor that accepts GDVInteger with an
  // argument of "0".  In order to prefer the GDVInteger interpretation,
  // make the ctor that accepts a pointer be a template, but delete the
  // general form.
  template <typename T>
  GDVALUE_CORE_CTOR GDValue(T const *str) = delete;

  // Then define the actual constructor I want as a specialization.
  template <>
  GDVALUE_CORE_CTOR GDValue(char const *str);

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


  // ---- Vector ----
  GDVALUE_CORE_CTOR GDValue(GDVVector const &vec);
  GDVALUE_CORE_CTOR GDValue(GDVVector      &&vec);

  void vectorSet(GDVVector const &vec);
  void vectorSet(GDVVector      &&vec);

  GDVVector const &vectorGet()        const;
  GDVVector       &vectorGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVVector, vector)

  void vectorAppend(GDValue value);

  // Discard extra elements or pad with nulls to match the size.
  void vectorResize(GDVSize newSize);

  void vectorSetValueAt(GDVIndex index, GDValue const &value);
  void vectorSetValueAt(GDVIndex index, GDValue      &&value);

  GDValue const &vectorGetValueAt(GDVIndex index) const;
  GDValue       &vectorGetValueAt(GDVIndex index)     ;

  void vectorClear();


  // ---- Set ---
  GDVALUE_CORE_CTOR GDValue(GDVSet const &set);
  GDVALUE_CORE_CTOR GDValue(GDVSet      &&set);

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
  GDVALUE_CORE_CTOR GDValue(GDVMap const &map);
  GDVALUE_CORE_CTOR GDValue(GDVMap      &&map);

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

DEFINE_GDV_KIND_ITERABLE(GDVVector, vector)

DEFINE_GDV_KIND_ITERABLE(GDVSet, set)


#undef DEFINE_GDV_KIND_ITERABLE


} // namespace gdv


#endif // SMBASE_GDVALUE_H
