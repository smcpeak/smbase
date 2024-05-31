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


// ---------------------------- GDValueKind ----------------------------
#define CASE(kind) #kind

DEFINE_ENUMERATION_TO_STRING_OR(
  GDValueKind,
  NUM_GDVALUE_KINDS,
  (
    CASE(GDVK_SYMBOL),
    CASE(GDVK_INTEGER),
    CASE(GDVK_STRING),
    CASE(GDVK_SEQUENCE),
    CASE(GDVK_SET),
    CASE(GDVK_MAP),
    CASE(GDVK_SMALL_INTEGER)
  ),
  "GDVK_invalid"
)

#undef CASE


// ------------------------ GDValue static data ------------------------
char const *GDValue::s_symbolName_null  = GDVSymbol::lookupSymbolName("null");
char const *GDValue::s_symbolName_false = GDVSymbol::lookupSymbolName("false");
char const *GDValue::s_symbolName_true  = GDVSymbol::lookupSymbolName("true");

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


// ---------------------- GDValue private helpers ----------------------
void GDValue::clearSelfAndSwapWith(GDValue &obj) noexcept
{
  using std::swap;

  clear();

  #define SWAP_MEMBER(member) \
    swap(m_value.member, obj.m_value.member)

  switch (obj.m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      SWAP_MEMBER(m_symbolName);
      break;

    case GDVK_INTEGER:
      SWAP_MEMBER(m_integer);
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

    case GDVK_SMALL_INTEGER:
      SWAP_MEMBER(m_smallInteger);
      break;
  }

  #undef SWAP_MEMBER

  std::swap(m_kind, obj.m_kind);
}


// --------------------- GDValue ctor/dtor/assign ----------------------
// In a ctor, initialize fields for the null value.
#define INIT_AS_NULL() \
    m_kind(GDVK_SYMBOL), \
    m_value(s_symbolName_null)


GDValue::GDValue() noexcept
  : INIT_AS_NULL()
{
  ++s_ct_ctorDefault;
}


GDValue::~GDValue()
{
  clear();

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
      integerSet(obj.integerGet());
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

    case GDVK_SMALL_INTEGER:
      m_kind = obj.m_kind;
      m_value.m_smallInteger = obj.m_value.m_smallInteger;
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
    clearSelfAndSwapWith(tmp);
  }

  ++s_ct_assignCopy;

  return *this;
}


GDValue &GDValue::operator=(GDValue &&obj)
{
  if (this != &obj) {
    clearSelfAndSwapWith(obj);
  }

  ++s_ct_assignMove;

  return *this;
}


GDValue::GDValue(GDValueKind kind)
  : m_kind(kind),
    m_value(s_symbolName_null)
{
  switch (m_kind) {
    default:
      xfailurePrecondition("invalid kind");

    case GDVK_SYMBOL:
      // Redundant, but for clarity.
      m_value.m_symbolName = s_symbolName_null;
      xassert(m_value.m_symbolName != nullptr);
      break;

    case GDVK_INTEGER:
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

    // GDVK_SMALL_INTEGER is invalid.
  }

  ++s_ct_valueKindCtor;
}


GDValueKind GDValue::getKind() const
{
  if (m_kind == GDVK_SMALL_INTEGER) {
    return GDVK_INTEGER;
  }
  else {
    return m_kind;
  }
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

  if (int ret = a.getKind() - b.getKind()) {
    return ret;
  }

  if (a.m_kind != b.m_kind) {
    // One must be a small integer and another large.
    return compare(a.integerGet(), b.integerGet());
  }

  switch (a.m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      return std::strcmp(a.m_value.m_symbolName, b.m_value.m_symbolName);

    case GDVK_INTEGER:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_integer);

    case GDVK_STRING:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_string);

    case GDVK_SEQUENCE:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_sequence);

    case GDVK_SET:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_set);

    case GDVK_MAP:
      return DEEP_COMPARE_PTR_MEMBERS(m_value.m_map);

    case GDVK_SMALL_INTEGER:
      return COMPARE_MEMBERS(m_value.m_smallInteger);
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
    ;
}


GDVSize GDValue::size() const
{
  switch (m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      return isNull()? 0 : 1;

    case GDVK_INTEGER:
    case GDVK_SMALL_INTEGER:
    case GDVK_STRING:
      return 1;

    case GDVK_SEQUENCE:
      return m_value.m_sequence->size();

    case GDVK_SET:
      return m_value.m_set->size();

    case GDVK_MAP:
      return m_value.m_map->size();
  }
}


bool GDValue::empty() const
{
  return size() == 0;
}


void GDValue::clear()
{
  switch (m_kind) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_SYMBOL:
      break;

    case GDVK_INTEGER:
      delete m_value.m_integer;
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

    case GDVK_SMALL_INTEGER:
      break;
  }

  m_kind = GDVK_SYMBOL;
  m_value.m_symbolName = s_symbolName_null;
}


void GDValue::swap(GDValue &obj) noexcept
{
  GDValue tmp;
  tmp.clearSelfAndSwapWith(obj);
  obj.clearSelfAndSwapWith(*this);
  this->clearSelfAndSwapWith(tmp);
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
    return m_value.m_symbolName == s_symbolName_null;
  }
  return false;
}


// ------------------------------ Boolean ------------------------------
bool GDValue::isBool() const
{
  if (m_kind == GDVK_SYMBOL) {
    return m_value.m_symbolName == s_symbolName_true ||
           m_value.m_symbolName == s_symbolName_false;
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
  clear();
  m_kind = GDVK_SYMBOL;
  m_value.m_symbolName =
    b? s_symbolName_true : s_symbolName_false;
  xassert(m_value.m_symbolName != nullptr);
}


bool GDValue::boolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);

  if (m_value.m_symbolName == s_symbolName_true) {
    return true;
  }
  else if (m_value.m_symbolName == s_symbolName_false) {
    return false;
  }
  else {
    xfailure("not one of the boolean symbols");
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
  clear();

  m_value.m_symbolName = sym.getSymbolName();
  m_kind = GDVK_SYMBOL;
}


GDVSymbol GDValue::symbolGet() const
{
  xassertPrecondition(m_kind == GDVK_SYMBOL);
  return GDVSymbol(GDVSymbol::BypassSymbolLookup, m_value.m_symbolName);
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


GDValue::GDValue(GDVSmallInteger i)
  : INIT_AS_NULL()
{
  m_kind = GDVK_SMALL_INTEGER;
  m_value.m_smallInteger = i;

  ++s_ct_integerSmallIntCtor;
}


bool GDValue::tryStoreSmallInteger(GDVInteger const &i)
{
  std::optional<GDVSmallInteger> smallValueOpt =
    i.getAsOpt<GDVSmallInteger>();

  if (smallValueOpt.has_value()) {
    m_kind = GDVK_SMALL_INTEGER;
    m_value.m_smallInteger = smallValueOpt.value();
    return true;
  }
  else {
    return false;
  }
}


void GDValue::integerSet(GDVInteger const &i)
{
  clear();

  if (!tryStoreSmallInteger(i)) {
    m_kind = GDVK_INTEGER;
    m_value.m_integer = new GDVInteger(i);
  }
}


void GDValue::integerSet(GDVInteger &&i)
{
  clear();

  if (!tryStoreSmallInteger(i)) {
    m_kind = GDVK_INTEGER;
    m_value.m_integer = new GDVInteger(std::move(i));
  }
}


GDVInteger GDValue::integerGet() const
{
  xassertPrecondition(getKind() == GDVK_INTEGER);

  if (m_kind == GDVK_SMALL_INTEGER) {
    return GDVInteger(m_value.m_smallInteger);
  }
  else {
    return *(m_value.m_integer);
  }
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
  clear();
  m_value.m_string = new GDVString(str);
  m_kind = GDVK_STRING;

  ++s_ct_stringSetCopy;
}


void GDValue::stringSet(GDVString &&str)
{
  clear();
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
  clear();
  m_value.m_sequence = new GDVSequence(vec);
  m_kind = GDVK_SEQUENCE;

  ++s_ct_sequenceSetCopy;
}


void GDValue::sequenceSet(GDVSequence &&vec)
{
  clear();
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
  if (index >= size()) {
    sequenceResize(index+1);
  }
  sequenceGetMutable().at(index) = value;
}


void GDValue::sequenceSetValueAt(GDVIndex index, GDValue &&value)
{
  if (index >= size()) {
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
  clear();
  m_value.m_set = new GDVSet(set);
  m_kind = GDVK_SET;

  ++s_ct_setSetCopy;
}


void GDValue::setSet(GDVSet &&set)
{
  clear();
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
  clear();
  m_value.m_map = new GDVMap(map);
  m_kind = GDVK_MAP;

  ++s_ct_mapSetCopy;
}


void GDValue::mapSet(GDVMap &&map)
{
  clear();
  m_value.m_map = new GDVMap(std::move(map));
  m_kind = GDVK_MAP;

  ++s_ct_mapSetMove;
}


GDVMap const &GDValue::mapGet() const
{
  xassertPrecondition(m_kind == GDVK_MAP);
  return *(m_value.m_map);
}


GDVMap &GDValue::mapGetMutable()
{
  xassertPrecondition(m_kind == GDVK_MAP);
  return *(m_value.m_map);
}


DEFINE_GDV_KIND_BEGIN_END(GDVMap, map, GDVK_MAP)


bool GDValue::mapContains(GDValue const &key) const
{
  xassertPrecondition(isMap());
  return m_value.m_map->find(key) != m_value.m_map->end();
}


GDValue const &GDValue::mapGetValueAt(GDValue const &key) const
{
  xassertPrecondition(isMap());
  auto it = m_value.m_map->find(key);
  xassertPrecondition(it != m_value.m_map->end());
  return (*it).second;
}


GDValue &GDValue::mapGetValueAt(GDValue const &key)
{
  xassertPrecondition(isMap());
  auto it = m_value.m_map->find(key);
  xassertPrecondition(it != m_value.m_map->end());
  return (*it).second;
}


void GDValue::mapSetValueAt(GDValue const &key, GDValue const &value)
{
  xassertPrecondition(isMap());

  auto it = m_value.m_map->find(key);
  if (it != m_value.m_map->end()) {
    (*it).second = value;
  }
  else {
    m_value.m_map->insert(std::make_pair(key, value));
  }
}


void GDValue::mapSetValueAt(GDValue &&key, GDValue &&value)
{
  xassertPrecondition(isMap());

  auto it = m_value.m_map->find(key);
  if (it != m_value.m_map->end()) {
    (*it).second = std::move(value);
  }
  else {
    m_value.m_map->emplace(
      std::make_pair(std::move(key), std::move(value)));
  }
}


bool GDValue::mapRemoveKey(GDValue const &key)
{
  xassertPrecondition(isMap());
  return m_value.m_map->erase(key) != 0;
}


void GDValue::mapClear()
{
  xassertPrecondition(isMap());
  return m_value.m_map->clear();
}


CLOSE_NAMESPACE(gdv)


// EOF
