// gdvalue.cc
// Code for gdvalue module.

// This file is in the public domain.

#include "gdvalue.h"                             // this module

// this dir
#include "smbase/compare-util.h"                 // smbase::compare, RET_IF_COMPARE
#include "smbase/exc.h"                          // GENERIC_CATCH_{BEGIN,END}
#include "smbase/gdv-binary64-float.h"           // gdv::GDVBinary64Float
#include "smbase/gdv-ordered-map.h"              // gdv::GDVOrderedMap
#include "smbase/gdvalue-reader.h"               // gdv::GDValueReader
#include "smbase/gdvalue-writer.h"               // gdv::GDValueWriter
#include "smbase/gdvsymbol.h"                    // gdv::GDVSymbol
#include "smbase/overflow.h"                     // convertNumberOpt
#include "smbase/safe-int-conv.h"                // smbase::IsSafelyConvertible_v
#include "smbase/sm-trace.h"                     // INIT_TRACE, etc.
#include "smbase/stringb.h"                      // stringb
#include "smbase/syserr.h"                       // smbase::xsyserror
#include "smbase/type-name-and-size-ops.h"       // makeTypeNameAndSizeForType
#include "smbase/xassert.h"                      // xassert
#include "smbase/xoverflow.h"                    // smbase::XNumericConversion{OutsideRange,FromAP}

// libc++
#include <fstream>                               // std::{ifstream, ofstream}
#include <sstream>                               // std::ostringstream
#include <string_view>                           // std::string_view
#include <utility>                               // std::move, std::swap, std::make_pair
#include <vector>                                // std::vector

using namespace smbase;


INIT_TRACE("gdvalue");


OPEN_NAMESPACE(gdv)


// ------------------------ GDVTaggedContainer -------------------------
template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::~GDVTaggedContainer()
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::GDVTaggedContainer()
  : m_tag(),
    m_container()
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::GDVTaggedContainer(
  GDVSymbol tag,
  CONTAINER const &container)
  : m_tag(tag),
    m_container(container)
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::GDVTaggedContainer(
  GDVSymbol tag,
  CONTAINER &&container)
  : m_tag(tag),
    m_container(std::move(container))
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::GDVTaggedContainer(
  GDVTaggedContainer const &obj)
  : DMEMB(m_tag),
    DMEMB(m_container)
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER>::GDVTaggedContainer(
  GDVTaggedContainer &&obj)
  : MDMEMB(m_tag),
    MDMEMB(m_container)
{}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER> &GDVTaggedContainer<CONTAINER>::operator=(
  GDVTaggedContainer const &obj)
{
  if (this != &obj) {
    CMEMB(m_tag);
    CMEMB(m_container);
  }
  return *this;
}


template <typename CONTAINER>
GDVTaggedContainer<CONTAINER> &GDVTaggedContainer<CONTAINER>::operator=(
  GDVTaggedContainer &&obj)
{
  if (this != &obj) {
    MCMEMB(m_tag);
    MCMEMB(m_container);
  }
  return *this;
}


template <typename CONTAINER>
void GDVTaggedContainer<CONTAINER>::swap(GDVTaggedContainer &obj)
{
  m_tag.swap(obj.m_tag);
  m_container.swap(obj.m_container);
}


template <typename CONTAINER>
int compare(
  GDVTaggedContainer<CONTAINER> const &a,
  GDVTaggedContainer<CONTAINER> const &b)
{
  RET_IF_COMPARE_MEMBERS(m_tag);
  RET_IF_COMPARE_MEMBERS(m_container);
  return 0;
}


// ---------------------------- GDValueKind ----------------------------
// Like `FOR_EACH_GDV_ALLOCATED_KIND`, but missing Integer.
#define FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(macro)        \
  macro(BINARY64_FLOAT,     Binary64Float   , binary64Float   )  \
  macro(STRING,             String          , string          )  \
  macro(SEQUENCE,           Sequence        , sequence        )  \
  macro(TAGGED_SEQUENCE,    TaggedSequence  , taggedSequence  )  \
  macro(TUPLE,              Tuple           , tuple           )  \
  macro(TAGGED_TUPLE,       TaggedTuple     , taggedTuple     )  \
  macro(SET,                Set             , set             )  \
  macro(TAGGED_SET,         TaggedSet       , taggedSet       )  \
  macro(MAP,                Map             , map             )  \
  macro(TAGGED_MAP,         TaggedMap       , taggedMap       )  \
  macro(ORDERED_MAP,        OrderedMap      , orderedMap      )  \
  macro(TAGGED_ORDERED_MAP, TaggedOrderedMap, taggedOrderedMap)

// Invoke `macro` for all of the kinds where the data is represented
// using an owner pointer to a GDVXXX object.
#define FOR_EACH_GDV_ALLOCATED_KIND(macro)               \
  macro(INTEGER,         Integer       , integer       ) \
  FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(macro)

// Invoke `macro` for all of the kinds.
#define FOR_EACH_GDV_KIND(macro)                         \
  macro(SYMBOL,          Symbol        , symbol        ) \
  macro(SMALL_INTEGER,   SmallInteger  , smallInteger  ) \
  FOR_EACH_GDV_ALLOCATED_KIND(macro)


#define CASE(kind) #kind

DEFINE_ENUMERATION_TO_STRING_OR(
  GDValueKind,
  NUM_GDVALUE_KINDS,
  (
    // I can't apply `FOR_EACH_GDV_KIND` here because that would make
    // one too many commas.
    CASE(GDVK_SYMBOL),
    CASE(GDVK_INTEGER),
    CASE(GDVK_SMALL_INTEGER),
    CASE(GDVK_BINARY64_FLOAT),
    CASE(GDVK_STRING),
    CASE(GDVK_SEQUENCE),
    CASE(GDVK_TAGGED_SEQUENCE),
    CASE(GDVK_TUPLE),
    CASE(GDVK_TAGGED_TUPLE),
    CASE(GDVK_SET),
    CASE(GDVK_TAGGED_SET),
    CASE(GDVK_MAP),
    CASE(GDVK_TAGGED_MAP),
    CASE(GDVK_ORDERED_MAP),
    CASE(GDVK_TAGGED_ORDERED_MAP)
  ),
  "GDVK_invalid"
)

#undef CASE


char const *kindCommonName(GDValueKind gdvk)
{
  RETURN_ENUMERATION_STRING_OR(
    GDValueKind,
    NUM_GDVALUE_KINDS,
    (
      "symbol",
      "integer",
      "small integer",
      "binary64 float",
      "string",
      "sequence",
      "tagged sequence",
      "tuple",
      "tagged tuple",
      "set",
      "tagged set",
      "map",
      "tagged map",
      "ordered map",
      "tagged ordered map"
    ),
    gdvk,
    "(invalid GDValueKind)"
  )
}


// ------------------------ GDValue static data ------------------------
GDVSymbol::Index GDValue::s_symbolIndex_false = GDVSymbol::lookupSymbolIndex("false");;
GDVSymbol::Index GDValue::s_symbolIndex_true  = GDVSymbol::lookupSymbolIndex("true");;

unsigned GDValue::s_ct_ctorDefault = 0;
unsigned GDValue::s_ct_dtor = 0;
unsigned GDValue::s_ct_ctorCopy = 0;
unsigned GDValue::s_ct_ctorMove = 0;
unsigned GDValue::s_ct_ctorTaggedContainer = 0;
unsigned GDValue::s_ct_assignCopy = 0;
unsigned GDValue::s_ct_assignMove = 0;
unsigned GDValue::s_ct_valueKindCtor = 0;
unsigned GDValue::s_ct_boolCtor = 0;
unsigned GDValue::s_ct_symbolCtor = 0;
unsigned GDValue::s_ct_integerCtorCopy = 0;
unsigned GDValue::s_ct_integerCtorMove = 0;
unsigned GDValue::s_ct_integerSmallIntCtor = 0;
unsigned GDValue::s_ct_binary64FloatCtorCopy = 0;
unsigned GDValue::s_ct_binary64FloatCtorMove = 0;
unsigned GDValue::s_ct_stringCtorCopy = 0;
unsigned GDValue::s_ct_stringCtorMove = 0;
unsigned GDValue::s_ct_stringSetCopy = 0;
unsigned GDValue::s_ct_stringSetMove = 0;

#define DEFINE_CTOR_COUNTS(KIND, Kind, kind)         \
  unsigned GDValue::s_ct_##kind##CtorCopy = 0;       \
  unsigned GDValue::s_ct_##kind##CtorMove = 0;       \
  unsigned GDValue::s_ct_##kind##SetCopy = 0;        \
  unsigned GDValue::s_ct_##kind##SetMove = 0;        \
  unsigned GDValue::s_ct_tagged##Kind##CtorCopy = 0; \
  unsigned GDValue::s_ct_tagged##Kind##CtorMove = 0;

FOR_EACH_GDV_CONTAINER(DEFINE_CTOR_COUNTS)

#undef DEFINE_CTOR_COUNTS


GDValueWriteOptions GDValue::s_defaultWriteOptions;


// ---------------------- GDValue private helpers ----------------------
void GDValue::setKindNoLoc(GDValueKind kind)
{
  m_kindSourceLocation = GDValueKindSourceLocation(kind);
}


void GDValue::resetSelfAndSwapWith(GDValue &obj) noexcept
{
  using std::swap;

  GENERIC_CATCH_BEGIN

  reset();

  switch (obj.getKind()) {
    default:
      xfailureInvariant("invalid kind");

    #define CASE(KIND, Kind, kind)                    \
      case GDVK_##KIND:                               \
        swap(m_value.m_##kind, obj.m_value.m_##kind); \
        break;

    FOR_EACH_GDV_KIND(CASE)

    #undef CASE
  }

  std::swap(m_kindSourceLocation, obj.m_kindSourceLocation);

  GENERIC_CATCH_END
}


// --------------------- GDValue ctor/dtor/assign ----------------------
// In a ctor, initialize fields for the null value.
#define INIT_AS_NULL()                 \
    m_kindSourceLocation(GDVK_SYMBOL), \
    m_value(s_symbolIndex_null)


GDValue::GDValue() noexcept
  : INIT_AS_NULL()
{
  ++s_ct_ctorDefault;
}


GDValue::~GDValue()
{
  reset();

  ++s_ct_dtor;
}


GDValue::GDValue(GDValue const &obj)
  : INIT_AS_NULL()
{
  switch (obj.getKind()) {
    default:
      xfailureInvariant("invalid kind");

    #define CASE(KIND, Kind, kind)  \
      case GDVK_##KIND:             \
        kind##Set(obj.kind##Get()); \
        break;

    FOR_EACH_GDV_KIND(CASE)

    #undef CASE
  }

  ++s_ct_ctorCopy;
}


GDValue::GDValue(GDValue &&obj)
  : INIT_AS_NULL()
{
  swap(obj);

  ++s_ct_ctorMove;
}


GDValue &GDValue::operator=(GDValue const &obj)
{
  if (this != &obj) {
    GDValue tmp(obj);
    resetSelfAndSwapWith(tmp);
  }

  ++s_ct_assignCopy;

  return *this;
}


GDValue &GDValue::operator=(GDValue &&obj)
{
  if (this != &obj) {
    resetSelfAndSwapWith(obj);
  }

  ++s_ct_assignMove;

  return *this;
}


GDValue::GDValue(GDValueKind kind)
  : m_kindSourceLocation(kind),
    m_value(s_symbolIndex_null)
{
  switch (getKind()) {
    default:
      xfailurePrecondition("invalid kind");

    case GDVK_SYMBOL:
      // Redundant, but for clarity.
      m_value.m_symbol = s_symbolIndex_null;
      break;

    case GDVK_INTEGER:
    case GDVK_SMALL_INTEGER:
      setKindNoLoc(GDVK_SMALL_INTEGER);
      m_value.m_smallInteger = 0;
      break;

    #define CASE(KIND, Kind, kind)        \
      case GDVK_##KIND:                   \
        m_value.m_##kind = new GDV##Kind; \
        break;

    FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(CASE)

    #undef CASE
  }

  ++s_ct_valueKindCtor;
}


GDValueKind GDValue::getSuperKind() const
{
  if (getKind() == GDVK_SMALL_INTEGER) {
    return GDVK_INTEGER;
  }
  else {
    return getKind();
  }
}


char const *GDValue::getKindName() const
{
  return toString(getKind());
}


char const *GDValue::getKindCommonName() const
{
  return kindCommonName(getKind());
}


bool GDValue::isContainer() const
{
  return isOrderedContainer() ||
         isUnorderedContainer();
}


bool GDValue::isTaggedContainer() const
{
  return isTaggedSequence() ||
         isTaggedTuple() ||
         isTaggedSet() ||
         isTaggedMap() ||
         isTaggedOrderedMap();
}


bool GDValue::isOrderedContainer() const
{
  return isSequence() ||
         isTuple() ||
         isOrderedMap();
}


bool GDValue::isUnorderedContainer() const
{
  return isSet() ||
         isMap();
}


// -------------------------- GDValue compare --------------------------
// TODO: This is a candidate for being moved to someplace more general.
template <typename CONTAINER>
static int compareOrderedContainer(CONTAINER const &aContainer,
                                   CONTAINER const &bContainer)
{
  auto aIt = aContainer.begin();
  auto bIt = bContainer.begin();

  while (aIt != aContainer.end() &&
         bIt != bContainer.end()) {
    // The first unequal elements decide the overall comparison.
    RET_IF_COMPARE(*aIt, *bIt);

    ++aIt;
    ++bIt;
  }

  if (aIt != aContainer.end()) {
    // 'aContainer' was longer, so it compares greater.
    return +1;
  }
  else if (bIt != bContainer.end()) {
    return -1;
  }
  else {
    return 0;
  }
}


template <typename T>
static int compare(std::vector<T> const &aVec,
                   std::vector<T> const &bVec)
{
  return compareOrderedContainer(aVec, bVec);
}


template <typename T>
static int compare(std::set<T> const &aSet,
                   std::set<T> const &bSet)
{
  // Although a set is unordered, std::set iterates in the right order
  // such that comparing as if ordered produces the right result.
  return compareOrderedContainer(aSet, bSet);
}


int compare(GDValue const &a, GDValue const &b)
{
  // We need to use the global template to compare the primitives, but
  // gdv::compare shadows it.
  using ::compare;

  // Order first by superkind.
  RET_IF_COMPARE(a.getSuperKind(), b.getSuperKind());

  if (a.getKind() != b.getKind()) {
    if (a.getSuperKind() == GDVK_INTEGER) {
      // Both are integers, but one is large and the other is small.
      // First compare the signs, swapping the order since false<true
      // but neg<pos.
      RET_IF_COMPARE(b.integerIsNegative(), a.integerIsNegative());

      // Both are negative or both are positive.
      bool neg = a.integerIsNegative();

      // For positive integers, small<large.  For negative, flip.
      RET_IF_COMPARE(b.isSmallInteger() != neg,
                     a.isSmallInteger() != neg)
    }

    xfailure("should not get here");
  }

  switch (a.getKind()) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      return GDVSymbol::compareIndices(
        a.m_value.m_symbol, b.m_value.m_symbol);

    case GDVK_SMALL_INTEGER:
      return COMPARE_MEMBERS(m_value.m_smallInteger);

    #define CASE(KIND, Kind, kind)                         \
      case GDVK_##KIND:                                    \
        return DEEP_COMPARE_PTR_MEMBERS(m_value.m_##kind);

    FOR_EACH_GDV_ALLOCATED_KIND(CASE)

    #undef CASE
  }
}


// ------------------- GDValue general container ops -------------------
STATICDEF unsigned GDValue::countConstructorCalls()
{
  return
    + s_ct_ctorDefault
    + s_ct_ctorCopy
    + s_ct_ctorMove
    + s_ct_ctorTaggedContainer
    + s_ct_valueKindCtor
    + s_ct_boolCtor
    + s_ct_symbolCtor
    + s_ct_integerSmallIntCtor

    #define CASE(KIND, Kind, kind) \
      + s_ct_##kind##CtorCopy      \
      + s_ct_##kind##CtorMove

    FOR_EACH_GDV_ALLOCATED_KIND(CASE)

    #undef CASE

    ;
}


void GDValue::reset()
{
  switch (getKind()) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
    case GDVK_SMALL_INTEGER:
      break;

    #define CASE(KIND, Kind, kind) \
      case GDVK_##KIND:            \
        delete m_value.m_##kind;   \
        break;

    FOR_EACH_GDV_ALLOCATED_KIND(CASE)

    #undef CASE
  }

  setKindNoLoc(GDVK_SYMBOL);
  m_value.m_symbol = s_symbolIndex_null;
}


void GDValue::swap(GDValue &obj) noexcept
{
  GDValue tmp;
  tmp.resetSelfAndSwapWith(obj);
  obj.resetSelfAndSwapWith(*this);
  this->resetSelfAndSwapWith(tmp);
}


static void checkAllocatedPtr(void const *)
{
  // General case, nothing to do.
}

static void checkAllocatedPtr(GDVOrderedMap const *p)
{
  p->selfCheck();
}

static void checkAllocatedPtr(GDVTaggedOrderedMap const *p)
{
  p->m_container.selfCheck();
}

static void checkAllocatedPtr(GDVBinary64Float const *p)
{
  p->selfCheck();
}


void GDValue::selfCheck() const
{
  switch (getKind()) {
    default:
      xfailureInvariant("bad kind");

    case GDVK_SYMBOL:
      xassertInvariant(GDVSymbol::validIndex(m_value.m_symbol));
      break;

    case GDVK_INTEGER:
      // It must not be possible to represent the value as a small
      // integer.
      xassertInvariant(
        !m_value.m_integer->getAsOpt<GDVSmallInteger>().has_value());
      break;

    case GDVK_SMALL_INTEGER:
      break;

    #define CASE(KIND, Kind, kind)                     \
      case GDVK_##KIND:                                \
        xassertInvariant(m_value.m_##kind != nullptr); \
        checkAllocatedPtr(m_value.m_##kind);           \
        break;

    FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(CASE)

    #undef CASE
  }
}


// -------------------------- Source location --------------------------
bool GDValue::hasSourceLocation() const
{
  return m_kindSourceLocation.hasSourceLocation();
}


GDValueSourceLocation GDValue::sourceLocation() const
{
  return m_kindSourceLocation.sourceLocation();
}


std::optional<GDValueSourceLocation> GDValue::sourceLocationOpt() const
{
  return m_kindSourceLocation.sourceLocationOpt();
}


void GDValue::clearSourceLocation()
{
  m_kindSourceLocation.clearSourceLocation();
}


void GDValue::setSourceLocation(GDValueSourceLocation loc)
{
  m_kindSourceLocation.setSourceLocation(loc);
}


void GDValue::setSourceLocationOpt(
  std::optional<GDValueSourceLocation> locOpt)
{
  m_kindSourceLocation.setSourceLocationOpt(locOpt);
}


// --------------------------- Write as text ---------------------------
void GDValue::write(std::ostream &os,
                    GDValueWriteOptions options) const
{
  GDValueWriter writer(os, options);
  writer.write(*this);
}


std::string GDValue::asString(GDValueWriteOptions options) const
{
  std::ostringstream oss;
  write(oss, options);
  return oss.str();
}


std::string GDValue::asIndentedString(GDValueWriteOptions options) const
{
  options.m_enableIndentation = true;
  return asString(options);
}


std::string GDValue::asIndentedStringLevel(
  int indentLevel,
  GDValueWriteOptions options) const
{
  options.m_indentLevel = indentLevel;
  return asIndentedString(options);
}


void GDValue::writeIndented(std::ostream &os,
                            GDValueWriteOptions options) const
{
  options.m_enableIndentation = true;
  write(os, options);
}


void GDValue::writeLines(std::ostream &os,
                         GDValueWriteOptions options) const
{
  writeIndented(os, options);
  os << "\n";
}


std::string GDValue::asLinesString(GDValueWriteOptions options) const
{
  std::ostringstream oss;
  writeLines(oss, options);
  return oss.str();
}


void GDValue::writeToFile(
  std::string const &fileName,
  GDValueWriteOptions options) const
{
  // TODO: I should wrap this behavior in a class.
  std::ofstream outFile(fileName.c_str(), std::ios_base::binary);
  if (!outFile) {
    xsyserror("open (for writing)", fileName);
  }

  write(outFile, options);
  outFile << '\n';
}


// --------------------------- Read as text ----------------------------
STATICDEF std::optional<GDValue> GDValue::readNextValue(std::istream &is)
{
  GDValueReader reader(is, std::nullopt);
  return reader.readNextValue();
}


STATICDEF GDValue GDValue::readFromStream(std::istream &is)
{
  GDValueReader reader(is, std::nullopt);
  return reader.readExactlyOneValue();
}


STATICDEF GDValue GDValue::readFromString(std::string const &str)
{
  std::istringstream iss(str);
  return readFromStream(iss);
}


STATICDEF GDValue GDValue::readFromStringView(std::string_view sv)
{
  // There is no way to directly construct an `istringstream` from a
  // `string_view`, which seems a bit broken to me.  (Obviously, a copy
  // has to be made, but `istringstream` internally carries a `string`
  // so could do so itself.)  In C++20, `istringstream` has a
  // constructor that will move a `string` argument into its internal
  // buffer, thus fixing the problem, but I'm using C++17 for now so
  // this does an extra copy.
  return readFromString(std::string(sv));
}


STATICDEF GDValue GDValue::readFromFile(std::string const &fileName)
{
  // TODO: I should wrap this behavior in a class.
  std::ifstream inFile(fileName.c_str(), std::ios_base::binary);
  if (!inFile) {
    xsyserror("open (for reading)", fileName);
  }

  GDValueReader reader(inFile, fileName);
  return reader.readExactlyOneValue();
}


// ------------------------------- Null --------------------------------
bool GDValue::isNull() const
{
  if (getKind() == GDVK_SYMBOL) {
    return m_value.m_symbol == s_symbolIndex_null;
  }
  return false;
}


// ------------------------------ Boolean ------------------------------
bool GDValue::isBool() const
{
  if (getKind() == GDVK_SYMBOL) {
    return m_value.m_symbol == s_symbolIndex_true ||
           m_value.m_symbol == s_symbolIndex_false;
  }
  return false;
}


GDValue::GDValue(BoolTagType, bool b)
  : INIT_AS_NULL()
{
  boolSet(b);

  ++s_ct_boolCtor;
}


STATICDEF GDValue GDValue::makeBool(bool b)
{
  return GDValue(BoolTag, b);
}


void GDValue::boolSet(bool b)
{
  reset();
  setKindNoLoc(GDVK_SYMBOL);
  m_value.m_symbol =
    b? s_symbolIndex_true : s_symbolIndex_false;

  // I expect 0 to be the null symbol.
  xassert(m_value.m_symbol != 0);
}


bool GDValue::boolGet() const
{
  xassertPrecondition(getKind() == GDVK_SYMBOL);

  if (m_value.m_symbol == s_symbolIndex_true) {
    return true;
  }
  else if (m_value.m_symbol == s_symbolIndex_false) {
    return false;
  }
  else {
    xfailurePrecondition("value is not a boolean");
    return false;  // Not reached.
  }
}


// ------------------------------ Symbol -------------------------------
GDValue::GDValue(GDVSymbol sym)
  : INIT_AS_NULL()
{
  symbolSet(sym);

  ++s_ct_symbolCtor;
}


void GDValue::symbolSet(GDVSymbol sym)
{
  reset();

  m_value.m_symbol = sym.getSymbolIndex();
  setKindNoLoc(GDVK_SYMBOL);
}


GDVSymbol GDValue::symbolGet() const
{
  xassertPrecondition(getKind() == GDVK_SYMBOL);
  return GDVSymbol(GDVSymbol::DirectIndex, m_value.m_symbol);
}


std::string_view GDValue::symbolGetName() const
{
  return symbolGet().getSymbolName();
}


// ------------------------------ Integer ------------------------------
GDValue::GDValue(GDVInteger const &i)
  : INIT_AS_NULL()
{
  integerSet(i);

  ++s_ct_integerCtorCopy;
}


GDValue::GDValue(GDVInteger &&i)
  : INIT_AS_NULL()
{
  integerSet(std::move(i));

  ++s_ct_integerCtorMove;
}


bool GDValue::trySmallIntegerSet(GDVInteger const &i)
{
  std::optional<GDVSmallInteger> smallValueOpt =
    i.getAsOpt<GDVSmallInteger>();

  if (smallValueOpt.has_value()) {
    smallIntegerSet(smallValueOpt.value());
    return true;
  }
  else {
    return false;
  }
}


void GDValue::integerSet(GDVInteger const &i)
{
  reset();

  if (!trySmallIntegerSet(i)) {
    setKindNoLoc(GDVK_INTEGER);
    m_value.m_integer = new GDVInteger(i);
  }
}


void GDValue::integerSet(GDVInteger &&i)
{
  reset();

  if (!trySmallIntegerSet(i)) {
    setKindNoLoc(GDVK_INTEGER);
    m_value.m_integer = new GDVInteger(std::move(i));
  }
}


GDVInteger GDValue::integerGet() const
{
  xassertPrecondition(isInteger());

  if (getKind() == GDVK_SMALL_INTEGER) {
    return GDVInteger(m_value.m_smallInteger);
  }
  else {
    return *(m_value.m_integer);
  }
}


bool GDValue::integerIsNegative() const
{
  xassertPrecondition(isInteger());

  if (getKind() == GDVK_SMALL_INTEGER) {
    return m_value.m_smallInteger < 0;
  }
  else {
    return m_value.m_integer->isNegative();
  }
}


GDVInteger const &GDValue::largeIntegerGet() const
{
  // This has to specifically be a large integer.
  xassertPrecondition(getKind() == GDVK_INTEGER);

  return *(m_value.m_integer);
}


template <typename T,
          typename>
T GDValue::integerGetAs() const
{
  if (std::optional<T> value = integerGetAsOpt<T>()) {
    return *value;
  }
  else {
    if (isSmallInteger()) {
      THROW(XNumericConversionOutsideRange(
        stringb(smallIntegerGet()),
        makeTypeNameAndSizeForType<GDVSmallInteger>(),
        makeTypeNameAndSizeForType<T>()
      ));
    }
    else {
      THROW(XNumericConversionFromAP(
        "GDVInteger",
        largeIntegerGet().toString(),
        std::is_signed_v<T>,
        sizeof(T)
      ));
    }
  }

  // Not reached.
  return T();
}


template <typename T,
          typename>
std::optional<T> GDValue::integerGetAsOpt() const
{
  if (isSmallInteger()) {
    return convertNumberOpt<T>(smallIntegerGet());
  }
  else {
    return largeIntegerGet().getAsOpt<T>();
  }
}


// -------------------- integerGetAs specialization --------------------
// Define the specializations we want.
#define DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(PRIM) \
  template                                                 \
  PRIM GDValue::integerGetAs() const;                      \
                                                           \
  template                                                 \
  std::optional<PRIM> GDValue::integerGetAsOpt() const;


DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(char)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(signed char)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned char)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(short)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned short)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(int)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(long)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned long)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(long long)
DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS(unsigned long long)


#undef DEFINE_INTEGER_GET_AS_METHOD_SPECIALIZATIONS


// --------------------------- SmallInteger ----------------------------
#define DEFINE_PRIMITIVE_INT_CTOR(NUMBER)                           \
  GDValue::GDValue(NUMBER i)                                        \
    : INIT_AS_NULL()                                                \
  {                                                                 \
    if constexpr (IsSafelyConvertible_v<NUMBER, GDVSmallInteger>) { \
      smallIntegerSet(static_cast<GDVSmallInteger>(i));             \
      ++s_ct_integerSmallIntCtor;                                   \
    }                                                               \
    else {                                                          \
      integerSet(GDVInteger(i));                                    \
      ++s_ct_integerCtorMove;                                       \
    }                                                               \
  }

DEFINE_PRIMITIVE_INT_CTOR(short)
DEFINE_PRIMITIVE_INT_CTOR(unsigned short)
DEFINE_PRIMITIVE_INT_CTOR(int)
DEFINE_PRIMITIVE_INT_CTOR(unsigned int)
DEFINE_PRIMITIVE_INT_CTOR(long)
DEFINE_PRIMITIVE_INT_CTOR(unsigned long)
DEFINE_PRIMITIVE_INT_CTOR(long long)
DEFINE_PRIMITIVE_INT_CTOR(unsigned long long)

#undef DEFINE_PRIMITIVE_INT_CTOR


void GDValue::smallIntegerSet(GDVSmallInteger i)
{
  reset();

  setKindNoLoc(GDVK_SMALL_INTEGER);
  m_value.m_smallInteger = i;
}


GDVSmallInteger GDValue::smallIntegerGet() const
{
  xassertPrecondition(isSmallInteger());

  return m_value.m_smallInteger;
}


// --------------------------- Binary64Float ---------------------------
GDValue::GDValue(GDVBinary64Float const &v)
  : INIT_AS_NULL()
{
  binary64FloatSet(v);

  ++s_ct_binary64FloatCtorCopy;
}


GDValue::GDValue(GDVBinary64Float &&v)
  : INIT_AS_NULL()
{
  binary64FloatSet(std::move(v));

  ++s_ct_binary64FloatCtorMove;
}


void GDValue::binary64FloatSet(GDVBinary64Float const &v)
{
  reset();

  m_value.m_binary64Float = new GDVBinary64Float(v);
  setKindNoLoc(GDVK_BINARY64_FLOAT);
}


void GDValue::binary64FloatSet(GDVBinary64Float &&v)
{
  reset();

  m_value.m_binary64Float = new GDVBinary64Float(std::move(v));
  setKindNoLoc(GDVK_BINARY64_FLOAT);
}


GDVBinary64Float const &GDValue::binary64FloatGet() const
{
  xassertPrecondition(isBinary64Float());

  return *(m_value.m_binary64Float);
}


/*static*/ GDValue GDValue::fromFloat(float d)
{
  return GDValue(GDVBinary64Float(d));
}


/*static*/ GDValue GDValue::fromDouble(double d)
{
  return GDValue(GDVBinary64Float(d));
}


float GDValue::binary64FloatGetAsFloat() const
{
  return float(binary64FloatGet().getAsDouble());
}


double GDValue::binary64FloatGetAsDouble() const
{
  return binary64FloatGet().getAsDouble();
}


// ------------------------------ String -------------------------------
GDValue::GDValue(GDVString const &str)
  : INIT_AS_NULL()
{
  stringSet(str);

  ++s_ct_stringCtorCopy;
}


GDValue::GDValue(GDVString &&str)
  : INIT_AS_NULL()
{
  stringSet(std::move(str));

  ++s_ct_stringCtorMove;
}


template <>
GDValue::GDValue(char const *str)
  : GDValue(GDVString(str))
{}


GDValue::GDValue(std::string_view sv)
  : GDValue(GDVString(sv))
{}


void GDValue::stringSet(GDVString const &str)
{
  reset();
  m_value.m_string = new GDVString(str);
  setKindNoLoc(GDVK_STRING);

  ++s_ct_stringSetCopy;
}


void GDValue::stringSet(GDVString &&str)
{
  reset();
  m_value.m_string = new GDVString(std::move(str));
  setKindNoLoc(GDVK_STRING);

  ++s_ct_stringSetMove;
}


GDVString const &GDValue::stringGet() const
{
  xassertPrecondition(isString());
  return *(m_value.m_string);
}


GDVString &GDValue::stringGetMutable()
{
  xassertPrecondition(isString());
  return *(m_value.m_string);
}


// Define the kind-specific begin/end methods that are not defined in
// clas `GDValue` class body.
#define DEFINE_GDV_KIND_BEGIN_END(Kind, kind)             \
  GDV##Kind::const_iterator GDValue::kind##CBegin() const \
  {                                                       \
    xassertPrecondition(is##Kind());                      \
    return kind##Get().cbegin();                          \
  }                                                       \
                                                          \
  GDV##Kind::const_iterator GDValue::kind##CEnd() const   \
  {                                                       \
    xassertPrecondition(is##Kind());                      \
    return kind##Get().cend();                            \
  }                                                       \
                                                          \
  GDV##Kind::iterator GDValue::kind##Begin()              \
  {                                                       \
    xassertPrecondition(is##Kind());                      \
    return kind##GetMutable().begin();                    \
  }                                                       \
                                                          \
  GDV##Kind::iterator GDValue::kind##End()                \
  {                                                       \
    xassertPrecondition(is##Kind());                      \
    return kind##GetMutable().end();                      \
  }


DEFINE_GDV_KIND_BEGIN_END(String, string)


// ---------------------------- Container ------------------------------
GDVSize GDValue::containerSize() const
{
  switch (getKind()) {
    default:
      xfailurePrecondition("not a container");

    #define CASE(KIND, Kind, kind)                         \
      case GDVK_##KIND:                                    \
        return m_value.m_##kind->size();                   \
                                                           \
      case GDVK_TAGGED_##KIND:                             \
        return m_value.m_tagged##Kind->m_container.size();

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }
}


bool GDValue::containerIsEmpty() const
{
  return containerSize() == 0;
}


// ----------------------------- Sequence ------------------------------
// Define the constructor, `XXXSet`, and `XXXGet` methods for a
// particular kind of container.
#define DEFINE_CONTAINER_CTOR_SET_GET(KIND, Kind, kind)       \
  GDValue::GDValue(GDV##Kind const &container)                \
    : INIT_AS_NULL()                                          \
  {                                                           \
    kind##Set(container);                                     \
                                                              \
    ++s_ct_##kind##CtorCopy;                                  \
  }                                                           \
                                                              \
  GDValue::GDValue(GDV##Kind &&container)                     \
    : INIT_AS_NULL()                                          \
  {                                                           \
    kind##Set(std::move(container));                          \
                                                              \
    ++s_ct_##kind##CtorMove;                                  \
  }                                                           \
                                                              \
  void GDValue::kind##Set(GDV##Kind const &container)         \
  {                                                           \
    if (is##Kind()) {                                         \
      kind##GetMutable() = container;                         \
    }                                                         \
    else {                                                    \
      reset();                                                \
      m_value.m_##kind = new GDV##Kind(container);            \
      setKindNoLoc(GDVK_##KIND);                              \
    }                                                         \
                                                              \
    ++s_ct_##kind##SetCopy;                                   \
  }                                                           \
                                                              \
  void GDValue::kind##Set(GDV##Kind &&container)              \
  {                                                           \
    if (is##Kind()) {                                         \
      kind##GetMutable() = std::move(container);              \
    }                                                         \
    else {                                                    \
      reset();                                                \
      m_value.m_##kind = new GDV##Kind(std::move(container)); \
      setKindNoLoc(GDVK_##KIND);                              \
    }                                                         \
                                                              \
    ++s_ct_##kind##SetMove;                                   \
  }                                                           \
                                                              \
  GDV##Kind const &GDValue::kind##Get() const                 \
  {                                                           \
    xassertPrecondition(is##Kind());                          \
                                                              \
    if (getKind() == GDVK_##KIND) {                           \
      return *(m_value.m_##kind);                             \
    }                                                         \
    else {                                                    \
      xassert(getKind() == GDVK_TAGGED_##KIND);               \
      return m_value.m_tagged##Kind->m_container;             \
    }                                                         \
  }                                                           \
                                                              \
  GDV##Kind &GDValue::kind##GetMutable()                      \
  {                                                           \
    return const_cast<GDV##Kind&>(kind##Get());               \
  }


DEFINE_CONTAINER_CTOR_SET_GET(SEQUENCE, Sequence, sequence)

DEFINE_GDV_KIND_BEGIN_END(Sequence, sequence)


GDVSize GDValue::sequenceSize() const
{
  xassertPrecondition(isSequence());
  return containerSize();
}


void GDValue::sequenceAppend(GDValue const &value)
{
  sequenceGetMutable().push_back(value);
}


void GDValue::sequenceAppend(GDValue &&value)
{
  sequenceGetMutable().push_back(std::move(value));
}


void GDValue::sequenceResize(GDVSize newSize)
{
  sequenceGetMutable().resize(newSize);
}


void GDValue::sequenceSetValueAt(GDVIndex index, GDValue const &value)
{
  if (index >= containerSize()) {
    sequenceResize(index+1);
  }
  sequenceGetMutable().at(index) = value;
}


void GDValue::sequenceSetValueAt(GDVIndex index, GDValue &&value)
{
  if (index >= containerSize()) {
    sequenceResize(index+1);
  }
  sequenceGetMutable().at(index) = std::move(value);
}


GDValue const &GDValue::sequenceGetValueAt(GDVIndex index) const
{
  return sequenceGet().at(index);
}


GDValue &GDValue::sequenceGetValueAt(GDVIndex index)
{
  return sequenceGetMutable().at(index);
}


void GDValue::sequenceClear()
{
  sequenceGetMutable().clear();
}


// ------------------------------ Tuple --------------------------------
DEFINE_CONTAINER_CTOR_SET_GET(TUPLE, Tuple, tuple)

DEFINE_GDV_KIND_BEGIN_END(Tuple, tuple)


void GDValue::tupleAppend(GDValue const &value)
{
  tupleGetMutable().push_back(value);
}


void GDValue::tupleAppend(GDValue &&value)
{
  tupleGetMutable().push_back(std::move(value));
}


void GDValue::tupleResize(GDVSize newSize)
{
  tupleGetMutable().resize(newSize);
}


void GDValue::tupleSetValueAt(GDVIndex index, GDValue const &value)
{
  if (index >= containerSize()) {
    tupleResize(index+1);
  }
  tupleGetMutable().at(index) = value;
}


void GDValue::tupleSetValueAt(GDVIndex index, GDValue &&value)
{
  if (index >= containerSize()) {
    tupleResize(index+1);
  }
  tupleGetMutable().at(index) = std::move(value);
}


GDValue const &GDValue::tupleGetValueAt(GDVIndex index) const
{
  return tupleGet().at(index);
}


GDValue &GDValue::tupleGetValueAt(GDVIndex index)
{
  return tupleGetMutable().at(index);
}


void GDValue::tupleClear()
{
  tupleGetMutable().clear();
}


// ------------------------------- Set ---------------------------------
DEFINE_CONTAINER_CTOR_SET_GET(SET, Set, set)

DEFINE_GDV_KIND_BEGIN_END(Set, set)


bool GDValue::setContains(GDValue const &elt) const
{
  xassertPrecondition(isSet());
  GDVSet const &set = setGet();
  return set.find(elt) != set.end();
}


GDValue const &GDValue::setGetValue(GDValue const &elt) const
{
  GDVSet const &set = setGet();
  auto it = set.find(elt);
  xassert(it != set.end());
  return *it;
}


bool GDValue::setInsert(GDValue const &elt)
{
  xassertPrecondition(isSet());
  auto res = setGetMutable().insert(elt);
  return res.second;
}


bool GDValue::setInsert(GDValue &&elt)
{
  xassertPrecondition(isSet());
  auto res = setGetMutable().insert(std::move(elt));
  return res.second;
}


bool GDValue::setRemove(GDValue const &elt)
{
  xassertPrecondition(isSet());
  return setGetMutable().erase(elt) != 0;
}


void GDValue::setClear()
{
  xassertPrecondition(isSet());
  return setGetMutable().clear();
}


// ------------------------------- Map ---------------------------------
DEFINE_CONTAINER_CTOR_SET_GET(MAP, Map, map)

DEFINE_GDV_KIND_BEGIN_END(Map, map)


bool GDValue::mapContains(GDValue const &key) const
{
  if (isOrderedMap()) {
    return orderedMapContains(key);
  }

  xassertPrecondition(isMap());
  return mapGet().find(key) != mapGet().end();
}


GDVMapEntry const &GDValue::mapGetEntryAt(GDValue const &key) const
{
  if (isOrderedMap()) {
    return orderedMapGetEntryAt(key);
  }

  xassertPrecondition(isMap());
  auto const &m = mapGet();
  auto it = m.find(key);
  if (it == m.end()) {
    TRACE1("mapGetValueAt: Tried to get value for key " << key <<
           " but it is not present in map " << *this << ".");
  }
  xassertPrecondition(it != m.end());
  return *it;
}


GDValue const &GDValue::mapGetKeyAt(GDValue const &key) const
{
  return mapGetEntryAt(key).first;
}


GDValue const &GDValue::mapGetValueAt(GDValue const &key) const
{
  return mapGetEntryAt(key).second;
}


GDValue &GDValue::mapGetValueAt(GDValue const &key)
{
  if (isOrderedMap()) {
    return orderedMapGetValueAt(key);
  }

  xassertPrecondition(isMap());
  auto it = mapGetMutable().find(key);
  xassertPrecondition(it != mapGetMutable().end());
  return (*it).second;
}


bool GDValue::mapInsertValueAt(GDValue const &key, GDValue const &value)
{
  if (isOrderedMap()) {
    return orderedMapInsertValueAt(key, value);
  }

  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    // Do not change the map if the key is already mapped.
    return false;  // Means nothing happened.
  }
  else {
    mapGetMutable().insert(std::make_pair(key, value));
    return true;   // Means insertion happened.
  }
}


bool GDValue::mapInsertValueAt(GDValue &&key, GDValue &&value)
{
  if (isOrderedMap()) {
    return orderedMapInsertValueAt(std::move(key), std::move(value));
  }

  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    return false;
  }
  else {
    mapGetMutable().emplace(
      std::make_pair(std::move(key), std::move(value)));
    return true;
  }
}


bool GDValue::mapSetValueAt(GDValue const &key, GDValue const &value)
{
  if (isOrderedMap()) {
    return orderedMapSetValueAt(key, value);
  }

  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    (*it).second = value;
    return false;  // Means assignment happened.
  }
  else {
    mapGetMutable().insert(std::make_pair(key, value));
    return true;   // Means insertion happened.
  }
}


bool GDValue::mapSetValueAt(GDValue &&key, GDValue &&value)
{
  if (isOrderedMap()) {
    return orderedMapSetValueAt(std::move(key), std::move(value));
  }

  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    (*it).second = std::move(value);
    return false;
  }
  else {
    mapGetMutable().emplace(
      std::make_pair(std::move(key), std::move(value)));
    return true;
  }
}


bool GDValue::mapRemoveKey(GDValue const &key)
{
  if (isOrderedMap()) {
    return orderedMapRemoveKey(key);
  }

  xassertPrecondition(isMap());
  return mapGetMutable().erase(key) != 0;
}


void GDValue::mapClear()
{
  if (isOrderedMap()) {
    orderedMapClear();
    return;
  }

  xassertPrecondition(isMap());
  return mapGetMutable().clear();
}


bool GDValue::mapContainsSym(char const *symName) const
{
  return mapContains(GDVSymbol(symName));
}


GDValue const &GDValue::mapGetValueAtSym(char const *symName) const
{
  return mapGetValueAt(GDVSymbol(symName));
}


GDValue &GDValue::mapGetValueAtSym(char const *symName)
{
  return mapGetValueAt(GDVSymbol(symName));
}


void GDValue::mapSetValueAtSym(char const *symName, GDValue const &value)
{
  mapSetValueAt(GDVSymbol(symName), value);
}


void GDValue::mapSetValueAtSym(char const *symName, GDValue &&value)
{
  mapSetValueAt(GDVSymbol(symName), std::move(value));
}


bool GDValue::mapRemoveKeySym(char const *symName)
{
  return mapRemoveKey(GDVSymbol(symName));
}


// ---------------------------- OrderedMap -----------------------------
DEFINE_CONTAINER_CTOR_SET_GET(ORDERED_MAP, OrderedMap, orderedMap)

DEFINE_GDV_KIND_BEGIN_END(OrderedMap, orderedMap)


/*static*/ GDValue GDValue::createOrderedMap(
  std::initializer_list<GDVMapEntry> ilist)
{
  return GDValue(GDVOrderedMap(ilist));
}


bool GDValue::orderedMapContains(GDValue const &key) const
{
  xassertPrecondition(isOrderedMap());
  return orderedMapGet().contains(key);
}


GDVMapEntry const &GDValue::orderedMapGetEntryAt(GDValue const &key) const
{
  xassertPrecondition(isOrderedMap());
  return orderedMapGet().entryAtKey(key);
}


GDValue const &GDValue::orderedMapGetKeyAt(GDValue const &key) const
{
  return orderedMapGetEntryAt(key).first;
}


GDValue const &GDValue::orderedMapGetValueAt(GDValue const &key) const
{
  return orderedMapGetEntryAt(key).second;
}


GDValue &GDValue::orderedMapGetValueAt(GDValue const &key)
{
  xassertPrecondition(isOrderedMap());
  return orderedMapGetMutable().valueAtKey(key);
}


bool GDValue::orderedMapInsertValueAt(GDValue const &key, GDValue const &value)
{
  xassertPrecondition(isOrderedMap());

  return orderedMapGetMutable().insert({key, value});
}


bool GDValue::orderedMapInsertValueAt(GDValue &&key, GDValue &&value)
{
  xassertPrecondition(isOrderedMap());

  return orderedMapGetMutable().insert({
    std::move(key), std::move(value)});
}


bool GDValue::orderedMapSetValueAt(GDValue const &key, GDValue const &value)
{
  xassertPrecondition(isOrderedMap());

  return orderedMapGetMutable().setValueAtKey(key, value);
}


bool GDValue::orderedMapSetValueAt(GDValue &&key, GDValue &&value)
{
  xassertPrecondition(isOrderedMap());

  return orderedMapGetMutable().setValueAtKey(
    std::move(key), std::move(value));
}


bool GDValue::orderedMapRemoveKey(GDValue const &key)
{
  xassertPrecondition(isOrderedMap());
  return orderedMapGetMutable().eraseKey(key);
}


void GDValue::orderedMapClear()
{
  xassertPrecondition(isOrderedMap());
  return orderedMapGetMutable().clear();
}


bool GDValue::orderedMapContainsSym(char const *symName) const
{
  return orderedMapContains(GDVSymbol(symName));
}


GDValue const &GDValue::orderedMapGetValueAtSym(char const *symName) const
{
  return orderedMapGetValueAt(GDVSymbol(symName));
}


GDValue &GDValue::orderedMapGetValueAtSym(char const *symName)
{
  return orderedMapGetValueAt(GDVSymbol(symName));
}


void GDValue::orderedMapSetValueAtSym(char const *symName, GDValue const &value)
{
  orderedMapSetValueAt(GDVSymbol(symName), value);
}


void GDValue::orderedMapSetValueAtSym(char const *symName, GDValue &&value)
{
  orderedMapSetValueAt(GDVSymbol(symName), std::move(value));
}


bool GDValue::orderedMapRemoveKeySym(char const *symName)
{
  return orderedMapRemoveKey(GDVSymbol(symName));
}


GDValue const &GDValue::orderedMapGetKeyAtIndex(GDVIndex index) const
{
  xassertPrecondition(isOrderedMap());
  xassertPrecondition(index < containerSize());

  return orderedMapGet().entryAtIndex(index).first;
}


// -------------------------- TaggedContainer --------------------------
GDValue::GDValue(GDValueKind kind, GDVSymbol tag)
  : INIT_AS_NULL()
{
  switch (kind) {
    default:
      xfailurePrecondition("not a tagged container kind");

    #define CASE(KIND, Container, container) \
      case GDVK_TAGGED_##KIND:               \
        setKindNoLoc(kind);                  \
        m_value.m_tagged##Container =        \
          new GDVTagged##Container(tag, {}); \
        break;

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }

  ++s_ct_ctorTaggedContainer;
}


void GDValue::taggedContainerSetTag(GDVSymbol tag)
{
  switch (getKind()) {
    default:
      xfailurePrecondition("not a tagged container");

    #define CASE(KIND, Container, container)      \
      case GDVK_TAGGED_##KIND:                    \
        m_value.m_tagged##Container->m_tag = tag; \
        break;

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }
}


GDVSymbol GDValue::taggedContainerGetTag() const
{
  switch (getKind()) {
    default:
      xfailurePrecondition("not a tagged container");

    #define CASE(KIND, Container, container)       \
      case GDVK_TAGGED_##KIND:                     \
        return m_value.m_tagged##Container->m_tag;

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }
}


std::string_view GDValue::taggedContainerGetTagName() const
{
  return taggedContainerGetTag().getSymbolName();
}


#define DEFINE_TAGGED_CONTAINER_METHODS(KIND, Container, container)             \
  GDValue::GDValue(GDVTagged##Container const &tcont)                           \
    : INIT_AS_NULL()                                                            \
  {                                                                             \
    tagged##Container##Set(tcont);                                              \
    ++s_ct_tagged##Container##CtorCopy;                                         \
  }                                                                             \
                                                                                \
  GDValue::GDValue(GDVTagged##Container &&tcont)                                \
    : INIT_AS_NULL()                                                            \
  {                                                                             \
    tagged##Container##Set(std::move(tcont));                                   \
    ++s_ct_tagged##Container##CtorMove;                                         \
  }                                                                             \
                                                                                \
  void GDValue::tagged##Container##Set(GDVTagged##Container const &tcont)       \
  {                                                                             \
    if (isTagged##Container()) {                                                \
      tagged##Container##GetMutable() = tcont;                                  \
    }                                                                           \
    else {                                                                      \
      reset();                                                                  \
      setKindNoLoc(GDVK_TAGGED_##KIND);                                         \
      m_value.m_tagged##Container = new GDVTagged##Container(tcont);            \
    }                                                                           \
  }                                                                             \
                                                                                \
  void GDValue::tagged##Container##Set(GDVTagged##Container &&tcont)            \
  {                                                                             \
    if (isTagged##Container()) {                                                \
      tagged##Container##GetMutable() = std::move(tcont);                       \
    }                                                                           \
    else {                                                                      \
      reset();                                                                  \
      setKindNoLoc(GDVK_TAGGED_##KIND);                                         \
      m_value.m_tagged##Container = new GDVTagged##Container(std::move(tcont)); \
    }                                                                           \
  }                                                                             \
                                                                                \
  GDVTagged##Container const &GDValue::tagged##Container##Get() const           \
  {                                                                             \
    xassertPrecondition(isTagged##Container());                                 \
    return *(m_value.m_tagged##Container);                                      \
  }                                                                             \
                                                                                \
  GDVTagged##Container &GDValue::tagged##Container##GetMutable()                \
  {                                                                             \
    xassertPrecondition(isTagged##Container());                                 \
    return *(m_value.m_tagged##Container);                                      \
  }


FOR_EACH_GDV_CONTAINER(DEFINE_TAGGED_CONTAINER_METHODS)


#define EXPLICITLY_INSTANTIATE(KIND, Kind, kind) \
  template class GDVTaggedContainer<GDV##Kind>;

FOR_EACH_GDV_CONTAINER(EXPLICITLY_INSTANTIATE)


// ----------------------- Member serialization ------------------------
char const *stripMemberPrefix(char const *name)
{
  if (name[0] == 'm' &&
      name[1] == '_') {
    return name+2;
  }
  else {
    return name;
  }
}


// ----------------------------- fromGDVN ------------------------------
GDValue fromGDVN(std::string const &str)
{
  return GDValue::readFromString(str);
}


GDValue fromGDVN(std::string_view sv)
{
  return GDValue::readFromStringView(sv);
}


GDValue fromGDVN(char const *str)
{
  return fromGDVN(std::string_view(str));
}


CLOSE_NAMESPACE(gdv)


// EOF
