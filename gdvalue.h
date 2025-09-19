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

#include "gdvalue-fwd.h"                         // fwds for this module

// IWYU pragma: begin_exports
#include "smbase/gdv-binary64-float-fwd.h"       // gdv::GDVBinary64Float
#include "smbase/gdv-containers-fwd.h"           // FOR_EACH_GDV_CONTAINER
#include "smbase/gdv-ordered-map-iface.h"        // gdv::GDVOrderedMap
#include "smbase/gdvalue-kind.h"                 // gdv::GDValueKind
#include "smbase/gdvalue-srcloc.h"               // gdv::GDValueSourceLocation
#include "smbase/gdvalue-types.h"                // gdv::{GDVSize, GDVIndex, GDVInteger, GDVSmallInteger, GDVString, GDVSequence, GDVSet, GDVMap, GDVOrderedMap, GDVMapEntry}
#include "smbase/gdvalue-write-options.h"        // gdv::GDValueWriteOptions
#include "smbase/gdvsymbol.h"                    // gdv::GDVSymbol
#include "smbase/gdvtuple.h"                     // gdv::GDVTuple
#include "smbase/sm-integer.h"                   // smbase::Integer
// IWYU pragma: end_exports

// this dir
#include "smbase/compare-util-iface.h"           // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "smbase/gdvalue-kind-srcloc.h"          // GDValueKindSourceLocation
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NULLABLE
#include "smbase/sm-pp-util.h"                   // SM_PP_COMMA_MAP
#include "smbase/std-optional-fwd.h"             // std::optional
#include "smbase/std-string-fwd.h"               // std::string::{iterator,const_iterator}
#include "smbase/std-string-view-fwd.h"          // std::string_view

// libc++
#include <initializer_list>                      // std::initializer_list
#include <iosfwd>                                // std::ostream
#include <map>                                   // std::map
#include <set>                                   // std::set
#include <string>                                // std::string
#include <type_traits>                           // std::{enable_if, is_convertible, ...}
#include <utility>                               // std::{pair, declval}
#include <vector>                                // std::vector

// Note: For the containers we depend on, a forward declaration is not
// sufficient because we need their `iterator` member types.


OPEN_NAMESPACE(gdv)


// ------------------------ GDVTaggedContainer -------------------------
// A pair of a symbol tag and a container.
//
// The CONTAINER is one of GDVSequence, GDVTuple, GDVSet, GDVMap, or
// GDVOrderedMap.
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

  // Despite the possible additional convenience, I do not allow passing
  // `char const *` as first argument.  The `_sym` literal suffix is
  // sufficiently convenient already.

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


/* See `gdvalue-fwd.h` for the following:

    using GDVTaggedSequence   = GDVTaggedContainer<GDVSequence>;
    using GDVTaggedTuple      = GDVTaggedContainer<GDVTuple>;
    using GDVTaggedSet        = GDVTaggedContainer<GDVSet>;
    using GDVTaggedMap        = GDVTaggedContainer<GDVMap>;
    using GDVTaggedOrderedMap = GDVTaggedContainer<GDVOrderedMap>;
*/


// ----------------------------- GDValue -------------------------------
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
         SmallInteger
       Binary64Float
       String
     Container                  -----+ non-exclusive subtype
       OrderedContainer              |
         Sequence                    V
           TaggedSequence          POMap (possibly-ordered map)
         Tuple                       |
           TaggedTuple               |
         OrderedMap             <----+
           TaggedOrderedMap          |
       UnorderedContainer            |
         Set                         |
           TaggedSet                 |
         Map                    <----+
           TaggedMap

    Every Container is either an OrderedContainer or an
    UnorderedContainer.  Independently, a Container can be a POMap
    (partially-ordered map).  In turn, every POMap is either a Map or an
    OrderedMap.

    Rationale for terminology: It is tempting to rearrange the terms
    like this:

      POMap          -> Map
      Map            -> UnorderedMap
      OrderedMap        (would stay the same)

    The problem with such a rearrangement is that it would clash with
    typical programming language vocabulary, and especially that of C++.
    Specifically, in C++, "map" already means a map whose *keys* have an
    intrinsic order, while "unordered map" means one where the keys do
    not have an intrinsic order.  In contrast, in GDValue, everything
    has an intrinsic order, and OrderedMap is distinguished by also
    having an arbitrary extrinsic order applied to the (key, value)
    pairs.
*/
class GDValue {
private:     // class data
  // Symbol indices with special semantics.
  //
  // I do not store `GDVSymbol` objects because those are mainly meant
  // to safely transport indices across the API.  Inside the
  // implementation, it is more convenient to work with indices
  // directly.
  static inline constexpr GDVSymbol::Index s_symbolIndex_null =
    GDVSymbol::s_nullSymbolIndex;
  static GDVSymbol::Index s_symbolIndex_false;
  static GDVSymbol::Index s_symbolIndex_true;

public:      // class data
  // Expose some method counts for testing purposes.
  static unsigned s_ct_ctorDefault;
  static unsigned s_ct_dtor;
  static unsigned s_ct_ctorCopy;
  static unsigned s_ct_ctorMove;
  static unsigned s_ct_ctorTaggedContainer;
  static unsigned s_ct_assignCopy;
  static unsigned s_ct_assignMove;
  static unsigned s_ct_valueKindCtor;
  static unsigned s_ct_boolCtor;
  static unsigned s_ct_symbolCtor;
  static unsigned s_ct_integerCtorCopy;
  static unsigned s_ct_integerCtorMove;
  static unsigned s_ct_integerSmallIntCtor;
  static unsigned s_ct_binary64FloatCtorCopy;
  static unsigned s_ct_binary64FloatCtorMove;
  static unsigned s_ct_stringCtorCopy;
  static unsigned s_ct_stringCtorMove;
  static unsigned s_ct_stringSetCopy;
  static unsigned s_ct_stringSetMove;

  #define DECLARE_CTOR_COUNTS(KIND, Kind, kind)  \
    static unsigned s_ct_##kind##CtorCopy;       \
    static unsigned s_ct_##kind##CtorMove;       \
    static unsigned s_ct_##kind##SetCopy;        \
    static unsigned s_ct_##kind##SetMove;        \
    static unsigned s_ct_tagged##Kind##CtorCopy; \
    static unsigned s_ct_tagged##Kind##CtorMove;

  FOR_EACH_GDV_CONTAINER(DECLARE_CTOR_COUNTS)

  #undef DECLARE_CTOR_COUNTS

  // Default write options.  This affects `operator<<` among other
  // things.  The initial value is simply the default-constructed value.
  static GDValueWriteOptions s_defaultWriteOptions;

private:     // instance data
  // Tag indicating which kind of value is represented, and for
  // integers, whether we are storing a large or small value.
  //
  // Plus: An optional source location.  The source location is
  // generally set for values that were parsed from GDVN or JSON, and
  // for values constructed with a location.  The location is cleared by
  // the `<kind>Set` methods (such as `boolSet`).
  //
  // It can be set or cleared at any time by a client of this class.
  GDValueKindSourceLocation m_kindSourceLocation;

  // Representation of the value.
  union GDValueUnion {
    // Index of a symbol.
    GDVSymbol::Index m_symbol;

    // These are all owner pointers (when active, of course).
    GDVInteger          *m_integer;
    GDVBinary64Float    *m_binary64Float;
    GDVString           *m_string;
    GDVSequence         *m_sequence;
    GDVTaggedSequence   *m_taggedSequence;
    GDVTuple            *m_tuple;
    GDVTaggedTuple      *m_taggedTuple;
    GDVSet              *m_set;
    GDVTaggedSet        *m_taggedSet;
    GDVMap              *m_map;
    GDVTaggedMap        *m_taggedMap;
    GDVOrderedMap       *m_orderedMap;
    GDVTaggedOrderedMap *m_taggedOrderedMap;

    // The value for `GDVK_SMALL_INTEGER`, which is used anytime an
    // integer is representable as `GDVSmallInteger`.
    //
    // Given that `GDVInteger` also has a small-integer storage option,
    // why have one here?  To avoid allocating an extra object for every
    // integer.  `GDVInteger` still exploits the small integer case for
    // faster *arithmetic* (and for better storage), while this class
    // does so for *storage* only.
    GDVSmallInteger m_smallInteger;

    // It would be possible to embed `GDVBinary64Float` without a space
    // penalty if it is implemented using `double` (which is my
    // expectation).  However, I want the design to accomodate a
    // different implementation, and there is no compelling need to
    // optimize the storage of floats in `GDValue`.

    explicit GDValueUnion(GDVSymbol::Index symbolIndex)
      : m_symbol(symbolIndex)
    {}
  } m_value;

private:     // methods
  // Set `m_kindSourceLocation` to `kind` with no location info.
  void setKindNoLoc(GDValueKind kind);

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
  //   Binary64Float: +0
  //   String: ""
  //   Container: empty
  //   Tagged container: null symbol, empty container
  explicit GDValue(GDValueKind kind);

  // Same, but with a source location.
  explicit GDValue(GDValueKind kind, GDValueSourceLocation loc);


  // Get the kind of value this is.  But see also `getSuperKind()`,
  // which hides the "small integer" implementation detail.
  GDValueKind getKind() const { return m_kindSourceLocation.getKind(); }

  // Return `toString(getKind())`.
  char const *getKindName() const;

  // Return `kindCommonName(getKind())`.
  char const *getKindCommonName() const;

  // Map SmallInteger to Integer, keeping other kinds the same, to get
  // the kind corresponding to the logical superclass.
  GDValueKind getSuperKind() const;

  bool isSymbol()           const { return getKind() == GDVK_SYMBOL;             }
  bool isInteger()          const { return getKind() == GDVK_INTEGER          ||
                                           isSmallInteger();                     }
  bool isSmallInteger()     const { return getKind() == GDVK_SMALL_INTEGER;      }
  bool isBinary64Float()    const { return getKind() == GDVK_BINARY64_FLOAT;     }
  bool isString()           const { return getKind() == GDVK_STRING;             }

  bool isSequence()         const { return getKind() == GDVK_SEQUENCE         ||
                                           isTaggedSequence();                   }
  bool isTaggedSequence()   const { return getKind() == GDVK_TAGGED_SEQUENCE;    }

  bool isTuple()            const { return getKind() == GDVK_TUPLE            ||
                                           isTaggedTuple();                      }
  bool isTaggedTuple()      const { return getKind() == GDVK_TAGGED_TUPLE;       }

  bool isSet()              const { return getKind() == GDVK_SET              ||
                                           isTaggedSet();                        }
  bool isTaggedSet()        const { return getKind() == GDVK_TAGGED_SET;         }

  bool isMap()              const { return getKind() == GDVK_MAP              ||
                                           isTaggedMap();                        }
  bool isTaggedMap()        const { return getKind() == GDVK_TAGGED_MAP;         }

  bool isOrderedMap()       const { return getKind() == GDVK_ORDERED_MAP      ||
                                           isTaggedOrderedMap();                 }
  bool isTaggedOrderedMap() const { return getKind() == GDVK_TAGGED_ORDERED_MAP; }
  bool isPOMap()            const { return isMap()                            ||
                                           isOrderedMap();                       }
  bool isTaggedPOMap()      const { return isTaggedMap()                      ||
                                           isTaggedOrderedMap();                 }

  // True of Sequence, Tuple, Set, Map, and OrderedMap, tagged or not.
  // False of others.
  bool isContainer() const;

  // True of the containers with a tag.
  bool isTaggedContainer() const;

  // True of Sequence, Tuple, and OrderedMap.
  bool isOrderedContainer() const;

  // True of Set and Map.
  bool isUnorderedContainer() const;


  /* Return <0 if a<b, 0 if a==b, and >0 otherwise.

     Comparison is first by value super-kind (`getSuperKind()`), in
     order of GDValueKind.  Then within each super-kind:

       symbol: Ordered lexicographically by code point.  A prefix (e.g.,
       "a") is less than any string it is a prefix of (e.g., "aa").

       integer: Ordered numerically.  Small integers are included in
       this order at their proper numerical position.

       binary64float: Ordered numerically, EXCEPT that -0 < +0 despite
       them being equal numerically per IEEE 754.  Note that means that
       every integer is less than every float when compared as GDValues.

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

       ordered map: Lexicographic by (k,v) pairs, the pairs themselves
       also being ordered lexicographically.

     Note: Since `null`, `false`, and `true` are treated as symbols,
     their relative order is:

       false < null < true

     Tagged containers compare the tag then the container.

     Source locations are IGNORED for comparison.
  */
  friend int compare(GDValue const &a, GDValue const &b);

  // Define operator==, etc.
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDValue)

  // Return the sum of all of the 's_ct_XXXCtorXXX' counts.
  static unsigned countConstructorCalls();


  // Reset to null with no source location.
  void reset();

  // Exchange values with 'obj'.
  void swap(GDValue &obj) noexcept;

  // Assert invariants.
  void selfCheck() const;


  // ---- Source location ----
  // True if this value has source location information.
  bool hasSourceLocation() const;

  // Get the location.
  //
  // Requires: hasSourceLocation()
  GDValueSourceLocation sourceLocation() const;

  // Get the location if we have one.
  std::optional<GDValueSourceLocation> sourceLocationOpt() const;

  // Remove a source location if we have one.
  void clearSourceLocation();

  // Set the location to `loc`.
  void setSourceLocation(GDValueSourceLocation loc);

  // Set it or clear it depending on `locOpt`.
  void setSourceLocationOpt(std::optional<GDValueSourceLocation> locOpt);


  // ---- Write as text ----
  // Write as text (GDVN) to 'os'.  By default this does not use any
  // indentation.
  void write(std::ostream &os,
             GDValueWriteOptions options = s_defaultWriteOptions) const;

  friend std::ostream &operator<<(std::ostream &os, GDValue const &v)
    { v.write(os); return os; }

  // Use 'write' to create a GDVN string.
  std::string asString(
    GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Same as `asString` but enable indentation in `options`.  This will
  // not print a final newline.
  std::string asIndentedString(
    GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Same as `asIndentedString`, but specify the indentation level,
  // which will override what is in `options`.
  std::string asIndentedStringLevel(
    int indentLevel,
    GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Enable indentation in the write options, then write to 'os'.
  void writeIndented(std::ostream &os,
                     GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Enable indentation in the write options, then write to 'os', then
  // write a final newline.
  void writeLines(std::ostream &os,
                  GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Capture what 'writeLines' would write as a string.
  std::string asLinesString(
    GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Write the value to 'fileName', terminated by a final newline.
  // Throw an exception if the file cannot be written.
  void writeToFile(std::string const &fileName,
                   GDValueWriteOptions options = s_defaultWriteOptions) const;

  // Write this value to `os` using indentation and enabling source
  // location printing, then write a newline and flush.
  void dumpTo(std::ostream &os) const;

  // Dump to specific streams.
  void dumpToStdout() const;
  void dumpToStderr() const;
  std::string dumpToString() const;

  // ---- Read as text ----
  // Read the next value from 'is'.  It must read enough to determine
  // that the value is complete, and will block if it is not.  It will
  // leave the input stream at the character after the last in the
  // value, typically using istream::putback to do that.
  //
  // If there is no value before EOF, this returns nullopt.
  //
  // If a syntax error is encountered, throws 'XReader' (declared in
  // `reader.h`).
  //
  static std::optional<GDValue> readNextValue(std::istream &is);

  // Read a single serialized value from 'is', throwing an exception if
  // there is not exactly one value before EOF or it is malformed.
  static GDValue readFromStream(std::istream &is);

  // Read the single serialized value in 'str', throwing an exception if
  // there is not exactly one value or it is malformed.
  static GDValue readFromString(std::string const &str);

  // Semantically the same as `readFromString`.
  static GDValue readFromStringView(std::string_view sv);

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

  // A non-template constructor that accepts 'bool' without any
  // possibility of ambiguity.
  enum BoolTagType { BoolTag };
  /*implicit*/ GDValue(BoolTagType, bool b);

  // A constructor that accepts exactly `bool`.
  template <typename BOOL,
            typename = typename std::enable_if<
                         std::is_same<BOOL, bool>::value>::type>
  /*implicit*/ GDValue(BOOL b)
    : GDValue(BoolTag, b)
  {}

  // Another way to construct `GDValue` from `bool` explicitly.
  static GDValue makeBool(bool b);

  void boolSet(bool b);

  bool boolGet() const;


  // ---- Symbol ----
  /*implicit*/ GDValue(GDVSymbol sym);
  /*implicit*/ GDValue(GDVSymbol sym, GDValueSourceLocation loc);

  void symbolSet(GDVSymbol sym);

  GDVSymbol symbolGet() const;

  // Get a view onto the symbol name that is valid until a symbol lookup
  // happens.
  std::string_view symbolGetName() const;


  // ---- Integer ----
  // The GDValue ctors are safe to use implicitly because they are
  // merely passive containers for data that preserve the information
  // passed as arguments.  Furthermore, making them explicit (which I
  // initially did) *greatly* expands the verbosity and difficulty of
  // reading initializers for complex values.
  /*implicit*/ GDValue(GDVInteger const &i);
  /*implicit*/ GDValue(GDVInteger      &&i);

  /*implicit*/ GDValue(GDVInteger const &i, GDValueSourceLocation loc);
  /*implicit*/ GDValue(GDVInteger      &&i, GDValueSourceLocation loc);

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

  // Allow converting to specific integer types.  This throws
  // `smbase::XNumericConversion` (`xoverflow.h`) if the value does not
  // fit.
  template <typename T,
            typename = std::enable_if<std::is_integral_v<T>>>
  T integerGetAs() const;

  // Return `nullopt` if the value does not fit.
  template <typename T,
            typename = std::enable_if<std::is_integral_v<T>>>
  std::optional<T> integerGetAsOpt() const;


  // ---- SmallInteger ----
  // GDValue does not have a character type, so constructing one with a
  // `char` is probably a mistake.
  /*implicit*/ GDValue(char i) = delete;
  /*implicit*/ GDValue(signed char i) = delete;
  /*implicit*/ GDValue(unsigned char i) = delete;

  // All of these yield small integers if the value fits, and otherwise
  // a full-size (arbitrary precision) integer.
  /*implicit*/ GDValue(short i);
  /*implicit*/ GDValue(unsigned short i);
  /*implicit*/ GDValue(int i);
  /*implicit*/ GDValue(unsigned int i);
  /*implicit*/ GDValue(long i);
  /*implicit*/ GDValue(unsigned long i);
  /*implicit*/ GDValue(long long i);
  /*implicit*/ GDValue(unsigned long long i);

  // TODO: Add overloads that accept a primitive integer and a location?

  // Callers must be careful not to pass a type that will be implicitly
  // converted and truncated, such as `uint64_t`.
  void smallIntegerSet(GDVSmallInteger i);

  // Requires `isSmallInteger()`.
  GDVSmallInteger smallIntegerGet() const;


  // ---- Binary64Float ----
  // It should be safe to construct this implicitly since we have the
  // intermediate `GDVBinary64Float` class, whose ctor is explicit, to
  // prevent unintended conversion from integers, etc.
  /*implicit*/ GDValue(GDVBinary64Float const &v);
  /*implicit*/ GDValue(GDVBinary64Float      &&v);

  /*implicit*/ GDValue(GDVBinary64Float const &v,
                       GDValueSourceLocation loc);
  /*implicit*/ GDValue(GDVBinary64Float      &&v,
                       GDValueSourceLocation loc);

  void binary64FloatSet(GDVBinary64Float const &v);
  void binary64FloatSet(GDVBinary64Float      &&v);

  // Requires `isBinary64Float()`.
  GDVBinary64Float const &binary64FloatGet() const;

  // Allow making binary64Float out of `float` and `double`.  (For the
  // moment I do not allow `long double` because it would not preserve
  // information on typical C++ implementations.)
  static GDValue fromFloat(float d);
  static GDValue fromDouble(double d);

  // Converting back to language floats.
  float binary64FloatGetAsFloat() const;
  double binary64FloatGetAsDouble() const;


  // ---- String ----
  /*implicit*/ GDValue(GDVString const &str);
  /*implicit*/ GDValue(GDVString      &&str);

  /*implicit*/ GDValue(GDVString const &str, GDValueSourceLocation loc);
  /*implicit*/ GDValue(GDVString      &&str, GDValueSourceLocation loc);

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

  // Also accept implicit conversion from `string_view`.
  /*implicit*/ GDValue(std::string_view sv);

  void stringSet(GDVString const &str);
  void stringSet(GDVString      &&str);

  GDVString const &stringGet()        const;
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


  // ---- Container ----
  // Number of elements in the container.
  //
  // Requires `isContainer()`.
  GDVSize containerSize() const;

  // True if `containerSize()==0`.
  bool containerIsEmpty() const;


  // ---- Sequence ----
  /*implicit*/ GDValue(GDVSequence const &seq);
  /*implicit*/ GDValue(GDVSequence      &&seq);

  // TODO: Ctor accepting a sequence and location?  And same for the
  // other containers?

  void sequenceSet(GDVSequence const &seq);
  void sequenceSet(GDVSequence      &&seq);

  // Requires `isSequence()`.
  //
  // TODO: Add the same for other containers?
  GDVSize sequenceSize() const;

  GDVSequence const &sequenceGet()        const;
  GDVSequence       &sequenceGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVSequence, sequence)

  void sequenceAppend(GDValue const &value);
  void sequenceAppend(GDValue      &&value);

  // Discard extra elements or pad with nulls to match the size.
  void sequenceResize(GDVSize newSize);

  void sequenceSetValueAt(GDVIndex index, GDValue const &value);
  void sequenceSetValueAt(GDVIndex index, GDValue      &&value);

  GDValue const &sequenceGetValueAt(GDVIndex index) const;
  GDValue       &sequenceGetValueAt(GDVIndex index)      ;

  void sequenceClear();


  // ---- Tuple ----
  /*implicit*/ GDValue(GDVTuple const &tup);
  /*implicit*/ GDValue(GDVTuple      &&tup);

  void tupleSet(GDVTuple const &tup);
  void tupleSet(GDVTuple      &&tup);

  GDVTuple const &tupleGet()        const;
  GDVTuple       &tupleGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVTuple, tuple)

  void tupleAppend(GDValue const &value);
  void tupleAppend(GDValue      &&value);

  // Discard extra elements or pad with nulls to match the size.
  void tupleResize(GDVSize newSize);

  void tupleSetValueAt(GDVIndex index, GDValue const &value);
  void tupleSetValueAt(GDVIndex index, GDValue      &&value);

  GDValue const &tupleGetValueAt(GDVIndex index) const;
  GDValue       &tupleGetValueAt(GDVIndex index)      ;

  void tupleClear();


  // ---- Set ---
  /*implicit*/ GDValue(GDVSet const &set);
  /*implicit*/ GDValue(GDVSet      &&set);

  void setSet(GDVSet const &set);
  void setSet(GDVSet      &&set);

  GDVSet const &setGet()        const;
  GDVSet       &setGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVSet, set)

  bool setContains(GDValue const &elt) const;

  // Return a reference to the physical `GDValue` that is stored in this
  // set, which will typically be a different object than `elt`, but
  // will compare as structurally equal.  Requires that `elt` be in the
  // set.
  //
  // This reference is of course invalidated if this object is destroyed
  // or the element is removed from the set (like for other references
  // returned from container queries).
  GDValue const &setGetValue(GDValue const &elt) const;

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

  // Return a reference to the entry object for the key, which must be
  // mapped.
  GDVMapEntry const &mapGetEntryAt(GDValue const &key) const;

  // Return a reference to the key stored in this map.
  GDValue const &mapGetKeyAt(GDValue const &key) const;

  // Requires that the key be mapped.
  GDValue const &mapGetValueAt(GDValue const &key) const;
  GDValue       &mapGetValueAt(GDValue const &key)      ;

  // Insert a new mapping and return true, or if the key is already
  // mapped, return false without changing the map.  This is similar to
  // `std::map::insert`.
  bool mapInsertValueAt(GDValue const &key, GDValue const &value);
  bool mapInsertValueAt(GDValue      &&key, GDValue      &&value);

  // Insert a new mapping and return true, or if the key is already
  // mapped, update the value it is mapped to and return false.  This is
  // similar to `std::map::insert_or_assign`.
  bool mapSetValueAt(GDValue const &key, GDValue const &value);
  bool mapSetValueAt(GDValue      &&key, GDValue      &&value);

  // Remove the mapping for `key` and return true, or if the key is not
  // mapped, then return false and do nothng.  This is similar to
  // `std::map::erase`.
  bool mapRemoveKey(GDValue const &key);

  void mapClear();

  // Operations that use symbols, named using `char*`, as keys.  These
  // are provided for syntactic convenience.
  bool mapContainsSym(char const *symName) const;
  GDValue const &mapGetValueAtSym(char const *symName) const;
  GDValue       &mapGetValueAtSym(char const *symName)      ;
  void mapSetValueAtSym(char const *symName, GDValue const &value);
  void mapSetValueAtSym(char const *symName, GDValue      &&value);
  bool mapRemoveKeySym(char const *symName);


  // ---- OrderedMap ----
  /*implicit*/ GDValue(GDVOrderedMap const &map);
  /*implicit*/ GDValue(GDVOrderedMap      &&map);

  // The compile-time dependencies to create a `GDVOrderedMap` directly
  // are a little heavy, so this provides a way to make a `GDValue` that
  // carries an ordered map without going through `GDVOrderedMap`.
  static GDValue createOrderedMap(
    std::initializer_list<GDVMapEntry> ilist);

  // If the current value is a tagged ordered map, these retain the tag.
  void orderedMapSet(GDVOrderedMap const &map);
  void orderedMapSet(GDVOrderedMap      &&map);

  GDVOrderedMap const &orderedMapGet()        const;
  GDVOrderedMap       &orderedMapGetMutable()      ;

  DECLARE_GDV_KIND_ITERATORS(GDVOrderedMap, orderedMap)

  // -- OrderedMap: Operations using keys
  bool orderedMapContains(GDValue const &key) const;

  GDVMapEntry const &orderedMapGetEntryAt(GDValue const &key) const;

  GDValue const &orderedMapGetKeyAt(GDValue const &key) const;

  // Requires that the key be mapped.
  GDValue const &orderedMapGetValueAt(GDValue const &key) const;
  GDValue       &orderedMapGetValueAt(GDValue const &key)      ;

  // Insert a new mapping (appending it to the order) and return true,
  // or if the key is already mapped, return false without changing the
  // map.
  bool orderedMapInsertValueAt(GDValue const &key, GDValue const &value);
  bool orderedMapInsertValueAt(GDValue      &&key, GDValue      &&value);

  // If the key is not already mapped, then the new entry is appended to
  // the order, and true is returned.  Otherwise, overwrite the value
  // for that key and return false.
  bool orderedMapSetValueAt(GDValue const &key, GDValue const &value);
  bool orderedMapSetValueAt(GDValue      &&key, GDValue      &&value);

  // Remove the mapping for `key`.  Return true if it was previously
  // there, and false if it was not.
  bool orderedMapRemoveKey(GDValue const &key);

  void orderedMapClear();

  // Operations that use symbols, named using `char*`, as keys.  These
  // are provided for syntactic convenience.
  bool orderedMapContainsSym(char const *symName) const;
  GDValue const &orderedMapGetValueAtSym(char const *symName) const;
  GDValue       &orderedMapGetValueAtSym(char const *symName)      ;
  void orderedMapSetValueAtSym(char const *symName, GDValue const &value);
  void orderedMapSetValueAtSym(char const *symName, GDValue      &&value);
  bool orderedMapRemoveKeySym(char const *symName);

  // In addition to the dedicated "orderedMap" functions, the following
  // "map" functions also work on ordered maps:
  //
  //   * mapContains
  //   * mapGetValueAt
  //   * mapSetValueAt
  //   * mapRemoveKey
  //   * mapClear
  //   * mapXXXSym

  // -- OrderedMap: Operations using indices
  // Get the key of the entry at `index`, which must be in [0,
  // containerSize()].  The returned reference is invalidated by calling
  // any non-const method.
  GDValue const &orderedMapGetKeyAtIndex(GDVIndex index) const;

  // TODO: More operations on indices.
  // TODO: Insert unmapped key at index.


  // ---- TaggedContainer ----
  // Create a tagged container with its tag.  `kind` must identify a
  // tagged container.
  explicit GDValue(GDValueKind kind, GDVSymbol tag);

  // Note: Every tagged container can be constructed from a
  // `GDVTagged<Container>` using the ctor declared below.

  // These methods require `isTaggedContainer()`.

  void taggedContainerSetTag(GDVSymbol tag);

  GDVSymbol taggedContainerGetTag() const;
  std::string_view taggedContainerGetTagName() const;

  #define GDV_DECLARE_TAGGED_CONTAINER_METHODS(KIND, Kind, kind) \
    /*implicit*/ GDValue(GDVTagged##Kind const &tcont);          \
    /*implicit*/ GDValue(GDVTagged##Kind      &&tcont);          \
                                                                 \
    /*implicit*/ GDValue(GDVTagged##Kind const &tcont,           \
                         GDValueSourceLocation loc);             \
    /*implicit*/ GDValue(GDVTagged##Kind      &&tcont,           \
                         GDValueSourceLocation loc);             \
                                                                 \
    void tagged##Kind##Set(GDVTagged##Kind const &tcont);        \
    void tagged##Kind##Set(GDVTagged##Kind      &&tcont);        \
                                                                 \
    GDVTagged##Kind const &tagged##Kind##Get()        const;     \
    GDVTagged##Kind       &tagged##Kind##GetMutable()      ;     \
                                                                 \
    /* True if this is a tagged##Kind with tag `tag`. */         \
    bool isTagged##Kind##WTag(std::string_view tag) const;

  FOR_EACH_GDV_CONTAINER(GDV_DECLARE_TAGGED_CONTAINER_METHODS)

  #undef GDV_DECLARE_TAGGED_CONTAINER_METHODS

  // Note: The accessors that work on untagged containers also work on
  // their tagged counterparts.


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

DEFINE_GDV_KIND_ITERABLE(GDVTuple, tuple)

DEFINE_GDV_KIND_ITERABLE(GDVSet, set)

DEFINE_GDV_KIND_ITERABLE(GDVMap, map)

DEFINE_GDV_KIND_ITERABLE(GDVOrderedMap, orderedMap)


#undef DEFINE_GDV_KIND_ITERABLE


#define DEFER_INSTANTIATE(KIND, Kind, kind) \
  extern template class GDVTaggedContainer<GDV##Kind>;

// Instantiated in gdvalue.cc.
FOR_EACH_GDV_CONTAINER(DEFER_INSTANTIATE)

#undef DEFER_INSTANTIATE


// -------------------- integerGetAs specialization --------------------
// Explicit instantiation declaration for the `integerGetAs` method
// templates.  This tells the compiler *not* to instantiate them
// implicitly.  There are explicit instantiations in the implementation
// file.
#define DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(PRIM) \
  extern template                                           \
  PRIM GDValue::integerGetAs() const;                       \
                                                            \
  extern template                                           \
  std::optional<PRIM> GDValue::integerGetAsOpt() const;


DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(char)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(signed char)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned char)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(short)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned short)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(int)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(long)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned long)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(long long)
DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned long long)


#undef DECLARE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS


// ----------------------------- toGDValue -----------------------------
/* The purpose of `toGDValue` is to provide something that can be
   overloaded to convert something to `GDValue` when it cannot be
   converted implicitly.  For user-written classes, it's usually best to
   implement `operator GDValue()`, but for classes outside the user's
   control, `toGDValue` can substitute.

   As an alternative to `operator GDValue()`, one can implement the
   `asGDValue()` method, which `toGDValue()` can also call.

   Generally, to call `toGDValue`, use a pattern like this:

     using gdv::toGDValue;      // or "using namespace gdv;"
     toGDValue(...)

   so that the definitions in `gdv` are accessible, but so are those in
   other namespaces, including what is findable by argument-dependent
   lookup.

   For this reason, the macros in this file invoke `toGDValue` without
   qualification, so often require a using declaration or directive to
   work.
*/

// `has_asGDValue_method<T>::value` is true iff `T` has an `asGDValue`
// member (that is not overloaded).
//
// False case:
//
template <typename, typename = void>
struct has_asGDValue_method : std::false_type {};
//
// True case:
//
template <typename T>
struct has_asGDValue_method<T, std::void_t<decltype(&T::asGDValue)> >
  : std::true_type {};


// `toGDValue` for when `T` has an `asGDValue` method.
template <typename T>
typename std::enable_if<has_asGDValue_method<T>::value,
                        GDValue>::type
                     // ^^^^^^^ Return type of this function.
toGDValue(T const &t)
{
  return t.asGDValue();
}


// `toGDValue(bool)` without implicit conversions to `bool`.
template <typename BOOL>
typename std::enable_if<std::is_same<BOOL, bool>::value,
                        GDValue>::type
                     // ^^^^^^^ Return type of this function.
toGDValue(BOOL const &b)
{
  return GDValue::makeBool(b);
}


// `toGDValue(double)` without implicit conversions.
template <typename DOUBLE>
typename std::enable_if<std::is_same<DOUBLE, double>::value,
                        GDValue>::type
                     // ^^^^^^^ Return type of this function.
toGDValue(DOUBLE const &d)
{
  return GDValue::fromDouble(d);
}


// `toGDValue(float)` without implicit conversions.
//
// TODO: Should I have a templated `from` method?
template <typename FLOAT>
typename std::enable_if<std::is_same<FLOAT, float>::value,
                        GDValue>::type
                     // ^^^^^^^ Return type of this function.
toGDValue(FLOAT const &d)
{
  return GDValue::fromFloat(d);
}


// `toGDValue` for when there is an implicit conversion, either because
// there is a matching `GDValue` constructor or because `T` has an
// `operator GDValue()`.  But specifically exclude `bool` since it is
// handled by the overload above.
template <typename T>
typename std::enable_if<std::is_convertible<T, GDValue>::value &&
                          !std::is_same<T, bool>::value,
                        GDValue>::type
                     // ^^^^^^^ Return type of this function.
toGDValue(T const &t)
{
  return t;
}


// For `std::pair`.
template <typename T1, typename T2>
GDValue toGDValue(std::pair<T1,T2> const &p)
{
  GDValue ret(GDVK_TUPLE);

  ret.tupleAppend(toGDValue(p.first));
  ret.tupleAppend(toGDValue(p.second));

  return ret;
}


// Note: Conversions to and from `std::{map,set,tuple}` are declared in
// `gdvalue-{map,set,tuple}.h`.


// For `smbase::OrderedMap`.
template <typename K, typename V>
GDValue toGDValue(smbase::OrderedMap<K,V> const &m)
{
  GDValue ret(GDVK_ORDERED_MAP);

  for (auto const &kv : m) {
    ret.orderedMapSetValueAt(toGDValue(kv.first), toGDValue(kv.second));
  }

  return ret;
}


// If `ptr` is null then yield a null GDValue.  Otherwise, dereference
// it and convert that to a GDValue.
template <typename T>
GDValue nullablePtrToGDValue(T const * NULLABLE ptr)
{
  if (ptr) {
    return toGDValue(*ptr);
  }
  else {
    return GDValue();
  }
}


// -------------------- Automatic ostream inserter ---------------------
// This works in simple cases, but in more complicated translation
// units, it can fail because this declaration must be visible when the
// usage is seen, but the usage might be in a template defined in some
// unrelated header file (such as `expectEq` in `sm-test.h`, mentioned
// below regarding `EXPECT_EQ`).  The usual fix would be to put this
// into a "-fwd.h" file so it can be put first among the #includes, but
// this definition is itself very dependency-laden.  So, at least for
// now, I'm giving up on this idea.
#if 0
// `allows_toGDValue<T>::value` is true if we can do `toGDValue(T)`.
template <typename T, typename = void>
struct allows_toGDValue : std::false_type {};

template <typename T>
struct allows_toGDValue<
  T,
  std::void_t<decltype(toGDValue(std::declval<T>()))>
> : std::true_type {};


// As an additional convenience, if `T` can be converted to `GDValue`,
// then use that for printing.
//
// One place this helps is my `EXPECT_EQ` macro, which first compares
// the objects, then prints them if not equal, for which there isn't an
// easy place to insert a `toGDValue` call (without changing the
// semantics of the comparison).
//
// This is tricky because there are a lot of potential ambiguities to
// avoid.
template <
  typename T,
  std::enable_if_t<
    // Must allow `toGDValue`.
    allows_toGDValue<T>::value &&

    // Must not be a primitive, since then it would already have a
    // better `operator<<`.
    !std::is_fundamental_v<std::decay_t<T>> &&

    // Exclude string literals, character pointers, etc.
    !std::is_pointer_v<std::decay_t<T>> &&

    // This is somewhat of a special case.  There isn't already a way to
    // print these, but converting to `GDValue` simply makes a `GDValue`
    // of that kind, rather than, say, a symbol with the kind's name.
    // So, if I want to print `GDValueKind`, I need to use `toString`.
    !std::is_same_v<std::decay_t<T>, GDValueKind> &&

    // Exclude all of the other things for which there is a `GDValue`
    // constructor but direct printing is preferable.
    !std::is_same_v<std::decay_t<T>, GDVSymbol> &&
    !std::is_same_v<std::decay_t<T>, GDVInteger> &&
    !std::is_same_v<std::decay_t<T>, GDVString> &&
    !std::is_same_v<std::decay_t<T>, std::string_view>,
  int> = 0
>
std::ostream &operator<<(std::ostream &os, T const &t) {
  toGDValue(t).write(os);
  return os;
}
#endif // 0


// --------------------- Serialization convenience ---------------------
// Temporarily (for the enclosing scope) set `amount` as the indent
// level.  This is meant for use before tracing output statements that
// provide some of their own indentation context.
#define GDVALUE_SCOPED_SET_INDENT(amount) \
  SET_RESTORE(GDValue::s_defaultWriteOptions.m_indentLevel, amount)


// Create a key/value pair that uses a symbol as a key.
#define GDV_SKV(name, value) \
  gdv::GDVMapEntry(gdv::GDVSymbol(name), toGDValue(value))

// Stringify an expression to name the symbol.
#define GDV_SKV_EXPR(expr) \
  GDV_SKV(#expr, (expr))


/*
  Render the values of each of several argument expressions as an
  indented string in the GDVN syntax of an ordered map.

  Use it like:

    GDVN_OMAP_EXPRS_LEVEL(2, expr1, expr2, expr3)

  which yields a string like:

    [
      expr1: <GDVN for expr1>
      expr2: <GDVN for expr2>
      expr3: <GDVN for expr3>
    ]

  where the first argument (here, 2) specifies the outermost indentation
  level in increments of two spaces.  In the example above, the closing
  bracket is indented by 2 such levels.  (The opening bracket is too,
  but the output string does not indent the first line.)

  This is primarily meant to be used as part of diagnostic output, to
  easily get a structured printout of several values that can be
  converted to GDValue.
*/
#define GDVN_OMAP_EXPRS_LEVEL(level, ...)      \
  (gdv::GDValue::createOrderedMap({            \
    SM_PP_COMMA_MAP(GDV_SKV_EXPR, __VA_ARGS__) \
  }).asIndentedStringLevel(level))


// More compact form for indentation level 0.
#define GDVN_OMAP_EXPRS(...) \
  GDVN_OMAP_EXPRS_LEVEL(0, __VA_ARGS__)


// ----------------------- Member serialization ------------------------
// If `name` begins with "m_", return `name+2`, thus stripping the
// prefix.  Otherwise return it unchanged.
char const *stripMemberPrefix(char const *name);

// Write `<memb>` to a field of GDValue `m` that is a symbol with the
// same name except without the "m_" prefix (if any).
#define GDV_WRITE_MEMBER_SYM(memb) \
  m.mapSetValueAtSym(gdv::stripMemberPrefix(#memb), toGDValue(memb)) /* user ; */

// Same, but the key is a string rather than a symbol.
#define GDV_WRITE_MEMBER_STR(memb) \
  m.mapSetValueAt(gdv::stripMemberPrefix(#memb), toGDValue(memb)) /* user , */


// Note: There are corresponding deserialization macros in
// `gdvalue-parser.h`.


// ----------------------------- fromGDVN ------------------------------
// Convenience alias for `GDValue::readFromString`.
GDValue fromGDVN(std::string const &str);

// Convenience alias for `GDValue::readFromStringView`.
GDValue fromGDVN(std::string_view sv);

// Resolve overload ambiguity.
GDValue fromGDVN(char const *str);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_H
