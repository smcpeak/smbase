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
#define CASE(kind) #kind

DEFINE_ENUMERATION_TO_STRING_OR(
  GDValueKind,
  NUM_GDVALUE_KINDS,
  (
    CASE(GDVK_SYMBOL),
    CASE(GDVK_INTEGER),
    CASE(GDVK_SMALL_INTEGER),
    CASE(GDVK_STRING),
    CASE(GDVK_SEQUENCE),
    CASE(GDVK_SET),
    CASE(GDVK_MAP),
    CASE(GDVK_TAGGED_MAP)
  ),
  "GDVK_invalid"
)

#undef CASE


// ------------------------ GDValue static data ------------------------
GDVSymbol::Index GDValue::s_symbolIndex_null  = GDVSymbol::lookupSymbolIndex("null");
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
unsigned GDValue::s_ct_integerCopyCtor = 0;
unsigned GDValue::s_ct_integerMoveCtor = 0;
unsigned GDValue::s_ct_integerSmallIntCtor = 0;
unsigned GDValue::s_ct_stringCtorCopy = 0;
unsigned GDValue::s_ct_stringCtorMove = 0;
unsigned GDValue::s_ct_stringSetCopy = 0;
unsigned GDValue::s_ct_stringSetMove = 0;
unsigned GDValue::s_ct_sequenceCtorCopy = 0;
unsigned GDValue::s_ct_sequenceCtorMove = 0;
unsigned GDValue::s_ct_sequenceSetCopy = 0;
unsigned GDValue::s_ct_sequenceSetMove = 0;
unsigned GDValue::s_ct_setCtorCopy = 0;
unsigned GDValue::s_ct_setCtorMove = 0;
unsigned GDValue::s_ct_setSetCopy = 0;
unsigned GDValue::s_ct_setSetMove = 0;
unsigned GDValue::s_ct_mapCtorCopy = 0;
unsigned GDValue::s_ct_mapCtorMove = 0;
unsigned GDValue::s_ct_mapSetCopy = 0;
unsigned GDValue::s_ct_mapSetMove = 0;
unsigned GDValue::s_ct_taggedMapCtorCopy = 0;
unsigned GDValue::s_ct_taggedMapCtorMove = 0;


// ---------------------- GDValue private helpers ----------------------
void GDValue::resetSelfAndSwapWith(GDValue &obj) noexcept
{
  using std::swap;

  reset();

  #define SWAP_MEMBER(member) \
    swap(m_value.member, obj.m_value.member)

  switch (obj.m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      SWAP_MEMBER(m_symbolIndex);
      break;

    case GDVK_INTEGER:
      SWAP_MEMBER(m_integer);
      break;

    case GDVK_SMALL_INTEGER:
      SWAP_MEMBER(m_smallInteger);
      break;

    case GDVK_STRING:
      SWAP_MEMBER(m_string);
      break;

    case GDVK_SEQUENCE:
      SWAP_MEMBER(m_sequence);
      break;

    case GDVK_SET:
      SWAP_MEMBER(m_set);
      break;

    case GDVK_MAP:
      SWAP_MEMBER(m_map);
      break;

    case GDVK_TAGGED_MAP:
      SWAP_MEMBER(m_taggedMap);
      break;
  }

  #undef SWAP_MEMBER

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

    case GDVK_SYMBOL:
      symbolSet(obj.symbolGet());
      break;

    case GDVK_INTEGER:
      integerSet(obj.largeIntegerGet());
      break;

    case GDVK_SMALL_INTEGER:
      smallIntegerSet(obj.smallIntegerGet());
      break;

    case GDVK_STRING:
      stringSet(obj.stringGet());
      break;

    case GDVK_SEQUENCE:
      sequenceSet(obj.sequenceGet());
      break;

    case GDVK_SET:
      setSet(obj.setGet());
      break;

    case GDVK_MAP:
      mapSet(obj.mapGet());
      break;

    case GDVK_TAGGED_MAP:
      taggedMapSet(obj.taggedMapGet());
      break;
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
      m_value.m_symbolIndex = s_symbolIndex_null;
      break;

    case GDVK_INTEGER:
    case GDVK_SMALL_INTEGER:
      m_kind = GDVK_SMALL_INTEGER;
      m_value.m_smallInteger = 0;
      break;

    case GDVK_STRING:
      m_value.m_string = new GDVString;
      break;

    case GDVK_SEQUENCE:
      m_value.m_sequence = new GDVSequence;
      break;

    case GDVK_SET:
      m_value.m_set = new GDVSet;
      break;

    case GDVK_MAP:
      m_value.m_map = new GDVMap;
      break;

    case GDVK_TAGGED_MAP:
      m_value.m_taggedMap = new GDVTaggedMap;
      break;
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
  return isTaggedMap();
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
        a.m_value.m_symbolIndex, b.m_value.m_symbolIndex);

    case GDVK_INTEGER:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_integer);

    case GDVK_SMALL_INTEGER:
      return COMPARE_MEMBERS(m_value.m_smallInteger);

    case GDVK_STRING:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_string);

    case GDVK_SEQUENCE:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_sequence);

    case GDVK_SET:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_set);

    case GDVK_MAP:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_map);

    case GDVK_TAGGED_MAP:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_taggedMap);
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
    + s_ct_integerCopyCtor
    + s_ct_integerMoveCtor
    + s_ct_integerSmallIntCtor
    + s_ct_stringCtorCopy
    + s_ct_stringCtorMove
    + s_ct_sequenceCtorCopy
    + s_ct_sequenceCtorMove
    + s_ct_setCtorCopy
    + s_ct_setCtorMove
    + s_ct_mapCtorCopy
    + s_ct_mapCtorMove
    + s_ct_taggedMapCtorCopy
    + s_ct_taggedMapCtorMove
    ;
}


void GDValue::reset()
{
  switch (m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      break;

    case GDVK_INTEGER:
      delete m_value.m_integer;
      break;

    case GDVK_SMALL_INTEGER:
      break;

    case GDVK_STRING:
      delete m_value.m_string;
      break;

    case GDVK_SEQUENCE:
      delete m_value.m_sequence;
      break;

    case GDVK_SET:
      delete m_value.m_set;
      break;

    case GDVK_MAP:
      delete m_value.m_map;
      break;

    case GDVK_TAGGED_MAP:
      delete m_value.m_taggedMap;
      break;
  }

  m_kind = GDVK_SYMBOL;
  m_value.m_symbolIndex = s_symbolIndex_null;
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
      xassertInvariant(GDVSymbol::validIndex(m_value.m_symbolIndex));
      break;

    case GDVK_INTEGER:
      // It must not be possible to represent the value as a small
      // integer.
      xassertInvariant(
        !m_value.m_integer->getAsOpt<GDVSmallInteger>().has_value());
      break;

    case GDVK_SMALL_INTEGER:
      break;

    case GDVK_STRING:
      xassertInvariant(m_value.m_string != nullptr);
      break;

    case GDVK_SEQUENCE:
      xassertInvariant(m_value.m_sequence != nullptr);
      break;

    case GDVK_SET:
      xassertInvariant(m_value.m_set != nullptr);
      break;

    case GDVK_MAP:
      xassertInvariant(m_value.m_map != nullptr);
      break;

    case GDVK_TAGGED_MAP:
      xassertInvariant(m_value.m_taggedMap != nullptr);
      break;
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
    return m_value.m_symbolIndex == s_symbolIndex_null;
  }
  return false;
}


// ------------------------------ Boolean ------------------------------
bool GDValue::isBool() const
{
  if (m_kind == GDVK_SYMBOL) {
    return m_value.m_symbolIndex == s_symbolIndex_true ||
           m_value.m_symbolIndex == s_symbolIndex_false;
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
  m_value.m_symbolIndex =
    b? s_symbolIndex_true : s_symbolIndex_false;

  // I expect 0 to be the null symbol.
  xassert(m_value.m_symbolIndex != 0);
}


bool GDValue::boolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);

  if (m_value.m_symbolIndex == s_symbolIndex_true) {
    return true;
  }
  else if (m_value.m_symbolIndex == s_symbolIndex_false) {
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

  m_value.m_symbolIndex = sym.getSymbolIndex();
  m_kind = GDVK_SYMBOL;
}


GDVSymbol GDValue::symbolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);
  return GDVSymbol(GDVSymbol::DirectIndex, m_value.m_symbolIndex);
}


// ------------------------------ Integer ------------------------------
GDValue::GDValue(GDVInteger const &i)
  : INIT_AS_NULL()
{
  integerSet(i);

  ++s_ct_integerCopyCtor;
}


GDValue::GDValue(GDVInteger &&i)
  : INIT_AS_NULL()
{
  integerSet(std::move(i));

  ++s_ct_integerMoveCtor;
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
  xassertPrecondition(m_kind == GDVK_STRING);
  return *(m_value.m_string);
}


// Define the begin/end methods that are not defined in the GDValue
// class body.
#define DEFINE_GDV_KIND_BEGIN_END(GDVKindName, kindName, GDVK_CODE) \
  GDVKindName::const_iterator GDValue::kindName##CBegin() const     \
  {                                                                 \
    xassertPrecondition(m_kind == GDVK_CODE);                       \
    return m_value.m_##kindName->cbegin();                          \
  }                                                                 \
                                                                    \
                                                                    \
  GDVKindName::const_iterator GDValue::kindName##CEnd() const       \
  {                                                                 \
    xassertPrecondition(m_kind == GDVK_CODE);                       \
    return m_value.m_##kindName->cend();                            \
  }                                                                 \
                                                                    \
                                                                    \
  GDVKindName::iterator GDValue::kindName##Begin()                  \
  {                                                                 \
    xassertPrecondition(m_kind == GDVK_CODE);                       \
    return m_value.m_##kindName->begin();                           \
  }                                                                 \
                                                                    \
                                                                    \
  GDVKindName::iterator GDValue::kindName##End()                    \
  {                                                                 \
    xassertPrecondition(m_kind == GDVK_CODE);                       \
    return m_value.m_##kindName->end();                             \
  }

DEFINE_GDV_KIND_BEGIN_END(GDVString, string, GDVK_STRING)


// ---------------------------- Container ------------------------------
GDVSize GDValue::containerSize() const
{
  switch (m_kind) {
    default:
      xfailurePrecondition("not a container");

    case GDVK_SEQUENCE:
      return m_value.m_sequence->size();

    case GDVK_SET:
      return m_value.m_set->size();

    case GDVK_MAP:
      return m_value.m_map->size();

    case GDVK_TAGGED_MAP:
      return m_value.m_taggedMap->m_container.size();
  }
}


bool GDValue::containerIsEmpty() const
{
  return containerSize() == 0;
}


// ----------------------------- Sequence ------------------------------
GDValue::GDValue(GDVSequence const &vec)
  : INIT_AS_NULL()
{
  sequenceSet(vec);

  ++s_ct_sequenceCtorCopy;
}


GDValue::GDValue(GDVSequence &&vec)
  : INIT_AS_NULL()
{
  sequenceSet(std::move(vec));

  ++s_ct_sequenceCtorMove;
}


void GDValue::sequenceSet(GDVSequence const &vec)
{
  reset();
  m_value.m_sequence = new GDVSequence(vec);
  m_kind = GDVK_SEQUENCE;

  ++s_ct_sequenceSetCopy;
}


void GDValue::sequenceSet(GDVSequence &&vec)
{
  reset();
  m_value.m_sequence = new GDVSequence(std::move(vec));
  m_kind = GDVK_SEQUENCE;

  ++s_ct_sequenceSetMove;
}


GDVSequence const &GDValue::sequenceGet() const
{
  xassertPrecondition(m_kind == GDVK_SEQUENCE);
  return *(m_value.m_sequence);
}


GDVSequence &GDValue::sequenceGetMutable()
{
  xassertPrecondition(m_kind == GDVK_SEQUENCE);
  return *(m_value.m_sequence);
}


DEFINE_GDV_KIND_BEGIN_END(GDVSequence, sequence, GDVK_SEQUENCE)


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
GDValue::GDValue(GDVSet const &set)
  : INIT_AS_NULL()
{
  setSet(set);

  ++s_ct_setCtorCopy;
}


GDValue::GDValue(GDVSet &&set)
  : INIT_AS_NULL()
{
  setSet(std::move(set));

  ++s_ct_setCtorMove;
}


void GDValue::setSet(GDVSet const &set)
{
  reset();
  m_value.m_set = new GDVSet(set);
  m_kind = GDVK_SET;

  ++s_ct_setSetCopy;
}


void GDValue::setSet(GDVSet &&set)
{
  reset();
  m_value.m_set = new GDVSet(std::move(set));
  m_kind = GDVK_SET;

  ++s_ct_setSetMove;
}


GDVSet const &GDValue::setGet() const
{
  xassertPrecondition(m_kind == GDVK_SET);
  return *(m_value.m_set);
}


GDVSet &GDValue::setGetMutable()
{
  xassertPrecondition(m_kind == GDVK_SET);
  return *(m_value.m_set);
}


DEFINE_GDV_KIND_BEGIN_END(GDVSet, set, GDVK_SET)


bool GDValue::setContains(GDValue const &elt) const
{
  xassertPrecondition(isSet());
  return m_value.m_set->find(elt) != m_value.m_set->end();
}


bool GDValue::setInsert(GDValue const &elt)
{
  xassertPrecondition(isSet());
  auto res = m_value.m_set->insert(elt);
  return res.second;
}


bool GDValue::setInsert(GDValue &&elt)
{
  xassertPrecondition(isSet());
  auto res = m_value.m_set->insert(std::move(elt));
  return res.second;
}


bool GDValue::setRemove(GDValue const &elt)
{
  xassertPrecondition(isSet());
  return m_value.m_set->erase(elt) != 0;
}


void GDValue::setClear()
{
  xassertPrecondition(isSet());
  return m_value.m_set->clear();
}


// ------------------------------- Map ---------------------------------
GDValue::GDValue(GDVMap const &map)
  : INIT_AS_NULL()
{
  mapSet(map);

  ++s_ct_mapCtorCopy;
}


GDValue::GDValue(GDVMap &&map)
  : INIT_AS_NULL()
{
  mapSet(std::move(map));

  ++s_ct_mapCtorMove;
}


void GDValue::mapSet(GDVMap const &map)
{
  if (isMap()) {
    mapGetMutable() = map;
  }
  else {
    reset();
    m_value.m_map = new GDVMap(map);
    m_kind = GDVK_MAP;
  }

  ++s_ct_mapSetCopy;
}


void GDValue::mapSet(GDVMap &&map)
{
  if (isMap()) {
    mapGetMutable() = map;
  }
  else {
    reset();
    m_value.m_map = new GDVMap(std::move(map));
    m_kind = GDVK_MAP;
  }

  ++s_ct_mapSetMove;
}


GDVMap const &GDValue::mapGet() const
{
  xassertPrecondition(isMap());

  if (m_kind == GDVK_MAP) {
    return *(m_value.m_map);
  }
  else {
    xassert(m_kind == GDVK_TAGGED_MAP);
    return m_value.m_taggedMap->m_container;
  }
}


GDVMap &GDValue::mapGetMutable()
{
  return const_cast<GDVMap&>(mapGet());
}


// For the moment this is just used for `map` but the plan is to use it
// for others later.
#define DEFINE_GDV_TAGGABLE_CONTAINER_BEGIN_END(GDVKindName, kindName, isKindName) \
  GDVKindName::const_iterator GDValue::kindName##CBegin() const                    \
  {                                                                                \
    xassertPrecondition(isKindName());                                             \
    return kindName##Get().cbegin();                                               \
  }                                                                                \
                                                                                   \
                                                                                   \
  GDVKindName::const_iterator GDValue::kindName##CEnd() const                      \
  {                                                                                \
    xassertPrecondition(isKindName());                                             \
    return kindName##Get().cend();                                                 \
  }                                                                                \
                                                                                   \
                                                                                   \
  GDVKindName::iterator GDValue::kindName##Begin()                                 \
  {                                                                                \
    xassertPrecondition(isKindName());                                             \
    return kindName##GetMutable().begin();                                         \
  }                                                                                \
                                                                                   \
                                                                                   \
  GDVKindName::iterator GDValue::kindName##End()                                   \
  {                                                                                \
    xassertPrecondition(isKindName());                                             \
    return kindName##GetMutable().end();                                           \
  }


DEFINE_GDV_TAGGABLE_CONTAINER_BEGIN_END(GDVMap, map, isMap)


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

    case GDVK_TAGGED_MAP:
      m_value.m_taggedMap->m_tag = tag;
      break;
  }
}


GDVSymbol GDValue::taggedContainerGetTag() const
{
  switch (m_kind) {
    default:
      xfailurePrecondition("not a tagged container");

    case GDVK_TAGGED_MAP:
      return m_value.m_taggedMap->m_tag;
  }
}


// ----------------------------- TaggedMap -----------------------------
GDValue::GDValue(GDVTaggedMap const &tmap)
  : INIT_AS_NULL()
{
  taggedMapSet(tmap);
  ++s_ct_taggedMapCtorCopy;
}


GDValue::GDValue(GDVTaggedMap &&tmap)
  : INIT_AS_NULL()
{
  taggedMapSet(std::move(tmap));
  ++s_ct_taggedMapCtorMove;
}


void GDValue::taggedMapSet(GDVTaggedMap const &tmap)
{
  if (isTaggedMap()) {
    taggedMapGetMutable() = tmap;
  }
  else {
    reset();
    m_kind = GDVK_TAGGED_MAP;
    m_value.m_taggedMap = new GDVTaggedMap(tmap);
  }
}


void GDValue::taggedMapSet(GDVTaggedMap &&tmap)
{
  if (isTaggedMap()) {
    taggedMapGetMutable() = std::move(tmap);
  }
  else {
    reset();
    m_kind = GDVK_TAGGED_MAP;
    m_value.m_taggedMap = new GDVTaggedMap(std::move(tmap));
  }
}


GDVTaggedMap const &GDValue::taggedMapGet() const
{
  xassertPrecondition(isTaggedMap());
  return *(m_value.m_taggedMap);
}


GDVTaggedMap &GDValue::taggedMapGetMutable()
{
  xassertPrecondition(isTaggedMap());
  return *(m_value.m_taggedMap);
}


template class GDVTaggedContainer<GDVMap>;


CLOSE_NAMESPACE(gdv)


// EOF
