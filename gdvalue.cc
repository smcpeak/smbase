// gdvalue.cc
// Code for gdvalue module.

// This file is in the public domain.

#include "gdvalue.h"                   // this module

// this dir
#include "compare-util.h"              // compare, RET_IF_COMPARE
#include "gdvalue-reader.h"            // gdv::GDValueReader
#include "gdvalue-writer.h"            // gdv::GDValueWriter
#include "gdvsymbol.h"                 // gdv::GDVSymbol
#include "syserr.h"                    // smbase::xsyserror
#include "xassert.h"                   // xassert

// libc++
#include <cstring>                     // std::strcmp
#include <fstream>                     // std::{ifstream, ofstream}
#include <new>                         // placement `new`
#include <sstream>                     // std::ostringstream
#include <utility>                     // std::move, std::swap, std::make_pair

using namespace smbase;


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
#define FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(macro) \
  macro(STRING,          String        , string        )  \
  macro(SEQUENCE,        Sequence      , sequence      )  \
  macro(TAGGED_SEQUENCE, TaggedSequence, taggedSequence)  \
  macro(SET,             Set           , set           )  \
  macro(TAGGED_SET,      TaggedSet     , taggedSet     )  \
  macro(MAP,             Map           , map           )  \
  macro(TAGGED_MAP,      TaggedMap     , taggedMap     )

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
    CASE(GDVK_STRING),
    CASE(GDVK_SEQUENCE),
    CASE(GDVK_TAGGED_SEQUENCE),
    CASE(GDVK_SET),
    CASE(GDVK_TAGGED_SET),
    CASE(GDVK_MAP),
    CASE(GDVK_TAGGED_MAP)
  ),
  "GDVK_invalid"
)

#undef CASE


// ------------------------ GDValue static data ------------------------
GDVSymbol::Index GDValue::s_symbolIndex_false = GDVSymbol::lookupSymbolIndex("false");;
GDVSymbol::Index GDValue::s_symbolIndex_true  = GDVSymbol::lookupSymbolIndex("true");;

unsigned GDValue::s_ct_ctorDefault = 0;
unsigned GDValue::s_ct_dtor = 0;
unsigned GDValue::s_ct_ctorCopy = 0;
unsigned GDValue::s_ct_ctorMove = 0;
unsigned GDValue::s_ct_assignCopy = 0;
unsigned GDValue::s_ct_assignMove = 0;
unsigned GDValue::s_ct_valueKindCtor = 0;
unsigned GDValue::s_ct_boolCtor = 0;
unsigned GDValue::s_ct_symbolCtor = 0;
unsigned GDValue::s_ct_integerCtorCopy = 0;
unsigned GDValue::s_ct_integerCtorMove = 0;
unsigned GDValue::s_ct_integerSmallIntCtor = 0;
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

// ---------------------- GDValue private helpers ----------------------
void GDValue::resetSelfAndSwapWith(GDValue &obj) noexcept
{
  using std::swap;

  reset();

  switch (obj.m_kind) {
    default:
      xfailureInvariant("invalid kind");

    #define CASE(KIND, Kind, kind)                    \
      case GDVK_##KIND:                               \
        swap(m_value.m_##kind, obj.m_value.m_##kind); \
        break;

    FOR_EACH_GDV_KIND(CASE)

    #undef CASE
  }

  std::swap(m_kind, obj.m_kind);
}


// --------------------- GDValue ctor/dtor/assign ----------------------
// In a ctor, initialize fields for the null value.
#define INIT_AS_NULL() \
    m_kind(GDVK_SYMBOL), \
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
  switch (obj.m_kind) {
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
  : m_kind(kind),
    m_value(s_symbolIndex_null)
{
  switch (m_kind) {
    default:
      xfailurePrecondition("invalid kind");

    case GDVK_SYMBOL:
      // Redundant, but for clarity.
      m_value.m_symbol = s_symbolIndex_null;
      break;

    case GDVK_INTEGER:
    case GDVK_SMALL_INTEGER:
      m_kind = GDVK_SMALL_INTEGER;
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
  if (m_kind == GDVK_SMALL_INTEGER) {
    return GDVK_INTEGER;
  }
  else {
    return m_kind;
  }
}


bool GDValue::isContainer() const
{
  return isSequence() ||
         isSet() ||
         isMap();
}


bool GDValue::isTaggedContainer() const
{
  return isTaggedSequence() ||
         isTaggedSet() ||
         isTaggedMap();
}


// -------------------------- GDValue compare --------------------------
// This is a candidate for being moved to someplace more general.
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
  return compareOrderedContainer(aSet, bSet);
}


int compare(GDValue const &a, GDValue const &b)
{
  // We need to use the global template to compare the primitives, but
  // gdv::compare shadows it.
  using ::compare;

  // Order first by superkind.
  RET_IF_COMPARE(a.getSuperKind(), b.getSuperKind());

  if (a.m_kind != b.m_kind) {
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

  switch (a.m_kind) {
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
  switch (m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      break;

    case GDVK_SMALL_INTEGER:
      break;

    #define CASE(KIND, Kind, kind) \
      case GDVK_##KIND:            \
        delete m_value.m_##kind;   \
        break;

    FOR_EACH_GDV_ALLOCATED_KIND(CASE)

    #undef CASE
  }

  m_kind = GDVK_SYMBOL;
  m_value.m_symbol = s_symbolIndex_null;
}


void GDValue::swap(GDValue &obj) noexcept
{
  GDValue tmp;
  tmp.resetSelfAndSwapWith(obj);
  obj.resetSelfAndSwapWith(*this);
  this->resetSelfAndSwapWith(tmp);
}


void GDValue::selfCheck() const
{
  switch (m_kind) {
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
        break;

    FOR_EACH_GDV_ALLOCATED_KIND_EXCEPT_INTEGER(CASE)

    #undef CASE
  }
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


void GDValue::writeLines(std::ostream &os,
                         GDValueWriteOptions options) const
{
  options.m_enableIndentation = true;
  write(os, options);
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
  if (m_kind == GDVK_SYMBOL) {
    return m_value.m_symbol == s_symbolIndex_null;
  }
  return false;
}


// ------------------------------ Boolean ------------------------------
bool GDValue::isBool() const
{
  if (m_kind == GDVK_SYMBOL) {
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


void GDValue::boolSet(bool b)
{
  reset();
  m_kind = GDVK_SYMBOL;
  m_value.m_symbol =
    b? s_symbolIndex_true : s_symbolIndex_false;

  // I expect 0 to be the null symbol.
  xassert(m_value.m_symbol != 0);
}


bool GDValue::boolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);

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
  m_kind = GDVK_SYMBOL;
}


GDVSymbol GDValue::symbolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);
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
    m_kind = GDVK_INTEGER;
    m_value.m_integer = new GDVInteger(i);
  }
}


void GDValue::integerSet(GDVInteger &&i)
{
  reset();

  if (!trySmallIntegerSet(i)) {
    m_kind = GDVK_INTEGER;
    m_value.m_integer = new GDVInteger(std::move(i));
  }
}


GDVInteger GDValue::integerGet() const
{
  xassertPrecondition(isInteger());

  if (m_kind == GDVK_SMALL_INTEGER) {
    return GDVInteger(m_value.m_smallInteger);
  }
  else {
    return *(m_value.m_integer);
  }
}


bool GDValue::integerIsNegative() const
{
  xassertPrecondition(isInteger());

  if (m_kind == GDVK_SMALL_INTEGER) {
    return m_value.m_smallInteger < 0;
  }
  else {
    return m_value.m_integer->isNegative();
  }
}


GDVInteger const &GDValue::largeIntegerGet() const
{
  // This has to specifically be a large integer.
  xassertPrecondition(m_kind == GDVK_INTEGER);

  return *(m_value.m_integer);
}


// --------------------------- SmallInteger ----------------------------
GDValue::GDValue(GDVSmallInteger i)
  : INIT_AS_NULL()
{
  smallIntegerSet(i);
  ++s_ct_integerSmallIntCtor;
}


void GDValue::smallIntegerSet(GDVSmallInteger i)
{
  reset();

  m_kind = GDVK_SMALL_INTEGER;
  m_value.m_smallInteger = i;
}


GDVSmallInteger GDValue::smallIntegerGet() const
{
  xassertPrecondition(isSmallInteger());

  return m_value.m_smallInteger;
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


void GDValue::stringSet(GDVString const &str)
{
  reset();
  m_value.m_string = new GDVString(str);
  m_kind = GDVK_STRING;

  ++s_ct_stringSetCopy;
}


void GDValue::stringSet(GDVString &&str)
{
  reset();
  m_value.m_string = new GDVString(std::move(str));
  m_kind = GDVK_STRING;

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
  switch (m_kind) {
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
      m_kind = GDVK_##KIND;                                   \
    }                                                         \
                                                              \
    ++s_ct_##kind##SetCopy;                                   \
  }                                                           \
                                                              \
  void GDValue::kind##Set(GDV##Kind &&container)              \
  {                                                           \
    if (is##Kind()) {                                         \
      kind##GetMutable() = container;                         \
    }                                                         \
    else {                                                    \
      reset();                                                \
      m_value.m_##kind = new GDV##Kind(std::move(container)); \
      m_kind = GDVK_##KIND;                                   \
    }                                                         \
                                                              \
    ++s_ct_##kind##SetMove;                                   \
  }                                                           \
                                                              \
  GDV##Kind const &GDValue::kind##Get() const                 \
  {                                                           \
    xassertPrecondition(is##Kind());                          \
                                                              \
    if (m_kind == GDVK_##KIND) {                              \
      return *(m_value.m_##kind);                             \
    }                                                         \
    else {                                                    \
      xassert(m_kind == GDVK_TAGGED_##KIND);                  \
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


void GDValue::sequenceAppend(GDValue value)
{
  sequenceGetMutable().push_back(value);
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


// ------------------------------- Set ---------------------------------
DEFINE_CONTAINER_CTOR_SET_GET(SET, Set, set)

DEFINE_GDV_KIND_BEGIN_END(Set, set)


bool GDValue::setContains(GDValue const &elt) const
{
  xassertPrecondition(isSet());
  GDVSet const &set = setGet();
  return set.find(elt) != set.end();
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
  xassertPrecondition(isMap());
  return mapGet().find(key) != mapGet().end();
}


GDValue const &GDValue::mapGetValueAt(GDValue const &key) const
{
  xassertPrecondition(isMap());
  auto it = mapGet().find(key);
  xassertPrecondition(it != mapGet().end());
  return (*it).second;
}


GDValue &GDValue::mapGetValueAt(GDValue const &key)
{
  xassertPrecondition(isMap());
  auto it = mapGetMutable().find(key);
  xassertPrecondition(it != mapGetMutable().end());
  return (*it).second;
}


void GDValue::mapSetValueAt(GDValue const &key, GDValue const &value)
{
  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    (*it).second = value;
  }
  else {
    mapGetMutable().insert(std::make_pair(key, value));
  }
}


void GDValue::mapSetValueAt(GDValue &&key, GDValue &&value)
{
  xassertPrecondition(isMap());

  auto it = mapGetMutable().find(key);
  if (it != mapGetMutable().end()) {
    (*it).second = std::move(value);
  }
  else {
    mapGetMutable().emplace(
      std::make_pair(std::move(key), std::move(value)));
  }
}


bool GDValue::mapRemoveKey(GDValue const &key)
{
  xassertPrecondition(isMap());
  return mapGetMutable().erase(key) != 0;
}


void GDValue::mapClear()
{
  xassertPrecondition(isMap());
  return mapGetMutable().clear();
}


// -------------------------- TaggedContainer --------------------------
void GDValue::taggedContainerSetTag(GDVSymbol tag)
{
  switch (m_kind) {
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
  switch (m_kind) {
    default:
      xfailurePrecondition("not a tagged container");

    #define CASE(KIND, Container, container)       \
      case GDVK_TAGGED_##KIND:                     \
        return m_value.m_tagged##Container->m_tag;

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }
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
      m_kind = GDVK_TAGGED_##KIND;                                              \
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
      m_kind = GDVK_TAGGED_##KIND;                                              \
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


CLOSE_NAMESPACE(gdv)


// EOF
