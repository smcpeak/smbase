// gdvalue.h
// General Data Value: integer, sequence, map, etc.

// This file is in the public domain.

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
#include "sm-integer.h"                // smbase::Integer
#include "sm-macros.h"                 // OPEN_NAMESPACE

// libc++
#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::int64_t
#include <iosfwd>                      // std::ostream
#include <map>                         // std::map
#include <set>                         // std::set
#include <string>                      // std::string
#include <vector>                      // std::vector
#include <utility>                     // std::pair


OPEN_NAMESPACE(gdv)


// Count of elements.
using GDVSize = std::size_t;

// Index for vectors.
using GDVIndex = std::size_t;

// GDValue(GDVK_INTEGER) holds this.
using GDVInteger = smbase::Integer;

// Stored when the kind is GDVK_SMALL_INTEGER.
using GDVSmallInteger = std::int64_t;

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


// A pair of a symbol tag and a container.
//
// The CONTAINER is one of GDVSequence, GDVSet, or GDVMap.
template <typename CONTAINER>
class GDVTaggedContainer {
public:      // data
  // The tag is meant to inform the consumer of the role that the
  // container plays.
  GDVSymbol m_tag;

  // The associated container.
  CONTAINER m_container;

public:      // methods
  ~GDVTaggedContainer();

  // Null symbol tag, empty container.
  GDVTaggedContainer();

  GDVTaggedContainer(GDVSymbol tag, CONTAINER const &container);
  GDVTaggedContainer(GDVSymbol tag, CONTAINER &&container);

  GDVTaggedContainer(GDVTaggedContainer const &obj);
  GDVTaggedContainer(GDVTaggedContainer &&obj);

  GDVTaggedContainer &operator=(GDVTaggedContainer const &obj);
  GDVTaggedContainer &operator=(GDVTaggedContainer &&obj);

  void swap(GDVTaggedContainer &obj);

  // Lexicographic comparison by tag then container contents.
  template <typename C>
  friend int compare(GDVTaggedContainer<C> const &a,
                     GDVTaggedContainer<C> const &b);
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDVTaggedContainer)
};

using GDVTaggedMap = GDVTaggedContainer<GDVMap>;


// Possible kinds of GDValues.
enum GDValueKind : unsigned char {
  // Symbol: An identifier-like string that acts as a name of something
  // defined elsewhere.  This includes the special symbols `null`,
  // `false`, and `true`.
  GDVK_SYMBOL,

  // Integer: Unbounded mathematical integer.
  GDVK_INTEGER,

  // Small integer: A logical subclass of Integer that fits into the
  // `GDVSmallInteger` type.
  GDVK_SMALL_INTEGER,

  // String: Sequence of Unicode characters encoded as UTF-8.
  GDVK_STRING,

  // Sequence: Ordered sequence of values.
  GDVK_SEQUENCE,

  // Set: Unordered set of (unique) values.
  GDVK_SET,

  // Map: Set of (key, value) pairs that are indexed by key.
  GDVK_MAP,

  // Tag+map.
  GDVK_TAGGED_MAP,

  NUM_GDVALUE_KINDS
};

// Return a string like "GDVK_SYMBOL", or "GDVK_invalid" if 'gdvk' is
// invalid.
char const *toString(GDValueKind gdvk);


/* A General Data Value is a disjoint union of several different types
   of data, enumerated as 'GDValueKind'.

   The logical hierarchy implemented by this class is:

     Scalar
       Symbol
         Null
         Bool
           True
           False
       Integer
         SmallIneger
       String
     Container
       Sequence
         TaggedSequence
       Set
         TaggedSet
       Map
         TaggedMap

   TODO: The Tagged containers aren't implemented yet.
*/
class GDValue {
private:     // class data
  // Symbol indices with special semantics.
  //
  // I do not store `GDVSymbol` objects because those are mainly meant
  // to safely transport indices across the API.  Inside the
  // implementation, it is more convenient to work with indices
  // directly.
  static GDVSymbol::Index s_symbolIndex_null;
  static GDVSymbol::Index s_symbolIndex_false;
  static GDVSymbol::Index s_symbolIndex_true;

public:      // class data
  // Expose some method counts for testing purposes.
  static unsigned s_ct_ctorDefault;
  static unsigned s_ct_dtor;
  static unsigned s_ct_ctorCopy;
  static unsigned s_ct_ctorMove;
  static unsigned s_ct_assignCopy;
  static unsigned s_ct_assignMove;
  static unsigned s_ct_valueKindCtor;
  static unsigned s_ct_boolCtor;
  static unsigned s_ct_symbolCtor;
  static unsigned s_ct_integerCopyCtor;
  static unsigned s_ct_integerMoveCtor;
  static unsigned s_ct_integerSmallIntCtor;
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
  static unsigned s_ct_taggedMapCtorCopy;
  static unsigned s_ct_taggedMapCtorMove;

private:     // instance data
  // Tag indicating which kind of value is represented, and for
  // integers, whether we are storing a large or small value.
  GDValueKind m_kind;

  // Representation of the value.
  union GDValueUnion {
    // Index of a symbol.
    GDVSymbol::Index m_symbolIndex;

    // These are all owner pointers (when active, of course).
    GDVInteger   *m_integer;
    GDVString    *m_string;
    GDVSequence  *m_sequence;
    GDVSet       *m_set;
    GDVMap       *m_map;
    GDVTaggedMap *m_taggedMap;

    // The value for `GDVK_SMALL_INTEGER`, which is used anytime an
    // integer is representable as `GDVSmallInteger`.
    //
    // Given that `GDVInteger` also has a small-integer storage option,
    // why have one here?  To avoid allocating an extra object for every
    // integer.  `GDVInteger` still exploits the small integer case for
    // faster *arithmetic* (and for better storage), while this class
    // does so for *storage* only.
    GDVSmallInteger m_smallInteger;

    explicit GDValueUnion(GDVSymbol::Index symbolIndex)
      : m_symbolIndex(symbolIndex)
    {}
  } m_value;

private:     // methods
  // Reset this object, then take the data in 'obj', leaving 'obj' as
  // the null value.
  void resetSelfAndSwapWith(GDValue &obj) noexcept;

  // If `i` can fit into `m_smallInteger`, store it there and return
  // true, otherwise return false without changing anything.
  bool trySmallIntegerSet(GDVInteger const &i);

public:      // methods
  // Make a `null` symbol value--that is, `isNull()` is true.
  GDValue() noexcept;

  ~GDValue();

  GDValue(GDValue const &obj);
  GDValue(GDValue      &&obj);

  GDValue &operator=(GDValue const &obj);
  GDValue &operator=(GDValue      &&obj);


  // Make an empty/zero value of 'kind':
  //   Symbol: null  (Note: This is not the empty symbol, ``.)
  //   Integer or SmallInteger: 0
  //   String: ""
  //   Container: empty
  //   Tagged container: null symbol, empty container
  explicit GDValue(GDValueKind kind);


  GDValueKind getKind() const { return m_kind; }

  // Map SmallInteger to Integer, keeping other kinds the same, to get
  // the kind corresponding to the logical superclass.
  GDValueKind getSuperKind() const;

  // True of sequence, set, and map.  False of others.
  bool isContainer() const;

  bool isSymbol()       const { return m_kind == GDVK_SYMBOL;        }
  bool isInteger()      const { return m_kind == GDVK_INTEGER ||
                                       isSmallInteger();             }
  bool isSmallInteger() const { return m_kind == GDVK_SMALL_INTEGER; }
  bool isString()       const { return m_kind == GDVK_STRING;        }
  bool isSequence()     const { return m_kind == GDVK_SEQUENCE;      }
  bool isSet()          const { return m_kind == GDVK_SET;           }
  bool isMap()          const { return m_kind == GDVK_MAP ||
                                       isTaggedMap();                }
  bool isTaggedMap()    const { return m_kind == GDVK_TAGGED_MAP;    }

  bool isTaggedContainer() const;


  /* Return <0 if a<b, 0 if a==b, and >0 otherwise.

     Comparison is first by value kind, in order of GDValueKind.  Then
     within each kind:

       symbol: Ordered lexicographically by code point.  A prefix (e.g.,
       "a") is less than any string it is a prefix of (e.g., "aa").

       integer: Ordered numerically.

       string: Lexicographic, like symbol.

       sequence: Lexicographic by element order.

       set: A<B iff there exists an element E such that:
              for all D less than E:
                D is in A and B or D is missing from A and B
              E is in B but not A

       map: A<B iff there exists a key K such that:
              for all J less than K:
                J is missing from both A and B or A[J] == B[J]
              K is in B but not A, or A[K] < B[K]

     Note: Since `null`, `false`, and `true` are treated as symbols,
     their relative order is:

       false < null < true
  */
  friend int compare(GDValue const &a, GDValue const &b);

  // Define operator==, etc.
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDValue)

  // Return the sum of all of the 's_ct_XXXCtorXXX' counts.
  static unsigned countConstructorCalls();


  // Reset to null.
  void reset();

  // Exchange values with 'obj'.
  void swap(GDValue &obj) noexcept;

  // Assert invariants.
  void selfCheck() const;


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


  // ---- Null ----
  // Null is the symbol `null`.
  bool isNull() const;


  // ---- Boolean ----
  // A boolean is a symbol that is either `false` or `true`.
  bool isBool() const;

  // A constructor that just accepts 'bool' creates too many ambiguities
  // so we use a discriminator tag.  But with the tag, this should be
  // safe to make usable implicitly in brace initializers.
  enum BoolTagType { BoolTag };
  /*implicit*/ GDValue(BoolTagType, bool b);

  void boolSet(bool b);

  bool boolGet() const;


  // ---- Symbol ----
  /*implicit*/ GDValue(GDVSymbol sym);

  void symbolSet(GDVSymbol sym);

  GDVSymbol symbolGet() const;


  // ---- Integer ----
  // The GDValue ctors are safe to use implicitly because they are
  // merely passive containers for data that preserve the information
  // passed as arguments.  Furthermore, making them explicit (which I
  // initially did) *greatly* expands the verbosity and difficulty of
  // reading initializers for complex values.
  /*implicit*/ GDValue(GDVInteger const &i);
  /*implicit*/ GDValue(GDVInteger      &&i);

  void integerSet(GDVInteger const &i);
  void integerSet(GDVInteger      &&i);

  // This does not return a `const &` because there might not be an
  // existing `GDVInteger` object to return.  In the common case of
  // storing a small integer, this does no allocation.  But that comes
  // at the expense of doing an extra allocation (versus returning a
  // reference) when we are storing a large integer.
  GDVInteger integerGet() const;

  // True if the integer is negative.
  //
  // Requires `isInteger()`.
  bool integerIsNegative() const;

  // Given that the value cannot be represented as a `GDVSmallInteger`,
  // return a reference to the large integer.  This method should only
  // be used when there is some performance justification for it, as it
  // couples the client more closely to this class's implementation than
  // calling `integerGet()` does.
  //
  // Requires `isInteger() && !isSmallInteger()`.
  GDVInteger const &largeIntegerGet() const;


  // ---- SmallInteger ----
  // This overload is needed to allow `GDValue(0)` to work because
  // otherwise it is ambiguous between the ctor accepting `GDVInteger`
  // and the ones accepting `GDVString`.
  /*implicit*/ GDValue(GDVSmallInteger i);

  void smallIntegerSet(GDVSmallInteger i);

  // Requires `isSmallInteger()`.
  GDVSmallInteger smallIntegerGet() const;


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

  // For containers, I think it is important for efficiency to be able
  // to modify them in-place, hence the "mutable" methods.  Strings have
  // aspects of both a scalar and a container, so it's plausible to
  // allow them to be mutated too.  However, I don't often mutate
  // strings, the potential efficiency benefit for them is limited, and
  // I'd like to leave the door open to a small-string optimization, so
  // for now there is no `stringGetMutable`.

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


  // ---- Container ----
  // Number of elements in the container.
  //
  // Requires `isContainer()`.
  GDVSize containerSize() const;

  // True if the size is zero.
  bool containerIsEmpty() const;


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

  // If the current value is a tagged map, these retain the tag.
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


  // ---- TaggedContainer ----
  // These methods require `isTaggedContainer()`.

  void taggedContainerSetTag(GDVSymbol tag);

  GDVSymbol taggedContainerGetTag() const;


  // ---- TaggedMap ----
  /*implicit*/ GDValue(GDVTaggedMap const &tmap);
  /*implicit*/ GDValue(GDVTaggedMap      &&tmap);

  void taggedMapSet(GDVTaggedMap const &tmap);
  void taggedMapSet(GDVTaggedMap      &&tmap);

  GDVTaggedMap const &taggedMapGet()        const;
  GDVTaggedMap       &taggedMapGetMutable()      ;

  // The map accessors work on tagged maps too.


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

DEFINE_GDV_KIND_ITERABLE(GDVMap, map)


#undef DEFINE_GDV_KIND_ITERABLE


// Instantiated in gdvalue.cc.
extern template class GDVTaggedContainer<GDVMap>;


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_H
