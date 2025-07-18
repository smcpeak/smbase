// gdvalue-parser.cc
// Code for `gdvalue-parser.h`.

#include "smbase/gdvalue-parser.h"     // this module

#include "smbase/exc.h"                // THROW
#include "smbase/gdvalue.h"            // GDValue
#include "smbase/overflow.h"           // convertNumberOpt
#include "smbase/sm-macros.h"          // DMEMB, MDMEMB
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert

#include <optional>                    // std::optional
#include <sstream>                     // std::ostringstream
#include <string_view>                 // std::string_view
#include <utility>                     // std::move


OPEN_NAMESPACE(gdv)


// ---------------------------- GDVNavStep -----------------------------
void GDVNavStep::copyUnionMember(GDVNavStep const &obj)
{
  switch (m_kind) {
    default:
      xfailure("bad kind");

    case SK_INDEX:
      m_index = obj.m_index;
      break;

    case SK_KEY:
      m_key = obj.m_key;
      break;

    case SK_VALUE:
      m_value = obj.m_value;
      break;
  }
}


GDVNavStep::GDVNavStep(StepIsIndex, GDVIndex index)
  : m_kind(SK_INDEX),
    m_index(index)
{}


GDVNavStep::GDVNavStep(StepIsKey, GDValue const *key)
  : m_kind(SK_KEY),
    m_key(key)
{}


GDVNavStep::GDVNavStep(StepIsValue, GDValue const *value)
  : m_kind(SK_VALUE),
    m_value(value)
{}


GDVNavStep::GDVNavStep(GDVNavStep const &obj)
  : m_kind(obj.m_kind)
{
  copyUnionMember(obj);
}


GDVNavStep &GDVNavStep::operator=(GDVNavStep const &obj)
{
  if (this != &obj) {
    m_kind = obj.m_kind;
    copyUnionMember(obj);
  }
  return *this;
}


std::string GDVNavStep::asString() const
{
  // When we're printing an access path, the performance cost of
  // decimalization is not important, and the integers are probably more
  // meaningful to the user in decimal.
  static GDValueWriteOptions const options =
    GDValueWriteOptions().setWriteLargeIntegersAsDecimal(true);

  switch (m_kind) {
    default:
      xfailure("bad kind");

    case SK_INDEX:
      return stringb("[" << m_index << "]");

    case SK_KEY:
      // The notation perhaps suggests we are "at" the indicated value,
      // rather than using it to traverse to something else.
      return stringb("@" << m_key->asString(options));

    case SK_VALUE:
      return stringb("." << m_value->asString(options));
  }
}


GDValue const *GDVNavStep::getSpecifiedChild(GDValue const *parent) const
{
  xassert(parent->isContainer());

  switch (parent->getKind()) {
    default:
      xfailure("bad GDValue kind");

    case GDVK_SEQUENCE:
    case GDVK_TAGGED_SEQUENCE:
      xassert(m_kind == SK_INDEX);
      return &( parent->sequenceGetValueAt(m_index) );

    case GDVK_TUPLE:
    case GDVK_TAGGED_TUPLE:
      xassert(m_kind == SK_INDEX);
      return &( parent->tupleGetValueAt(m_index) );

    case GDVK_SET:
    case GDVK_TAGGED_SET:
      xassert(m_kind == SK_KEY);
      xassert( &( parent->setGetValue(*m_key) ) == m_key );
      return m_key;

    case GDVK_MAP:
    case GDVK_TAGGED_MAP:
    case GDVK_ORDERED_MAP:
    case GDVK_TAGGED_ORDERED_MAP:
      if (m_kind == SK_KEY) {
        xassert( &( parent->mapGetKeyAt(*m_key) ) == m_key );
        return m_key;
      }
      else {
        xassert(m_kind == SK_VALUE);

        // Note that we are using `*m_value` as a *key*.  The name
        // reflects where we are going, rather than the meaning of the
        // thing itself.
        return &( parent->mapGetValueAt(*m_value) );
      }
  }
}


// --------------------------- GDValueParser ---------------------------
bool GDValueParser::s_selfCheckCtors = false;


#define POSSIBLY_SELFCHECK_THIS() \
  if (s_selfCheckCtors) {         \
    selfCheck();                  \
  }                               \
  else                            \
    (void)0 /* user ; */


GDValueParser::~GDValueParser()
{}


GDValueParser::GDValueParser(GDValueParser const &obj)
  : DMEMB(m_topLevel),
    DMEMB(m_value),
    DMEMB(m_path)
{
  POSSIBLY_SELFCHECK_THIS();
}


GDValueParser::GDValueParser(GDValueParser      &&obj)
  : MDMEMB(m_topLevel),
    MDMEMB(m_value),
    MDMEMB(m_path)
{
  POSSIBLY_SELFCHECK_THIS();
}


GDValueParser::GDValueParser(GDValue const &topLevel)
  : m_topLevel(&topLevel),
    m_value(&topLevel),
    m_path()
{
  POSSIBLY_SELFCHECK_THIS();
}


GDValueParser::GDValueParser(GDValueParser const &parent, GDVNavStep step)
  : m_topLevel(parent.m_topLevel),
    m_value(step.getSpecifiedChild(parent.m_value)),
    m_path(parent.m_path)
{
  m_path.push_back(step);
  POSSIBLY_SELFCHECK_THIS();
}


std::string GDValueParser::valueGDVN() const
{
  return getValue().asString();
}


std::string GDValueParser::pathString() const
{
  std::ostringstream oss;

  oss << "<top>";

  for (GDVNavStep const &step : m_path) {
    oss << step.asString();
  }

  return oss.str();
}


void GDValueParser::selfCheck() const
{
  // Walk from the top level down to the current value using the path.
  GDValue const *v = m_topLevel;
  for (GDVNavStep const &step : m_path) {
    v = step.getSpecifiedChild(v);
  }

  // We should end up at the current value.
  xassert(m_value == v);
}


#define RELAY_QUERY(rettype, name) \
  rettype GDValueParser::name() const { return m_value->name(); }

RELAY_QUERY(GDValueKind, getKind)
RELAY_QUERY(char const *, getKindName)
RELAY_QUERY(char const *, getKindCommonName)
RELAY_QUERY(GDValueKind, getSuperKind)
RELAY_QUERY(bool, isSymbol)
RELAY_QUERY(bool, isInteger)
RELAY_QUERY(bool, isSmallInteger)
RELAY_QUERY(bool, isString)
RELAY_QUERY(bool, isSequence)
RELAY_QUERY(bool, isTaggedSequence)
RELAY_QUERY(bool, isTuple)
RELAY_QUERY(bool, isTaggedTuple)
RELAY_QUERY(bool, isSet)
RELAY_QUERY(bool, isTaggedSet)
RELAY_QUERY(bool, isMap)
RELAY_QUERY(bool, isTaggedMap)
RELAY_QUERY(bool, isOrderedMap)
RELAY_QUERY(bool, isTaggedOrderedMap)
RELAY_QUERY(bool, isPOMap)
RELAY_QUERY(bool, isTaggedPOMap)
RELAY_QUERY(bool, isContainer)
RELAY_QUERY(bool, isTaggedContainer)
RELAY_QUERY(bool, isOrderedContainer)
RELAY_QUERY(bool, isUnorderedContainer)


void GDValueParser::throwError(std::string &&msg) const
{
  THROW(XGDValueError(*this, std::move(msg)));
}


#define throwError_stringb(stuff) throwError(stringb(stuff))


void GDValueParser::checkKind(GDValueKind kind) const
{
  if (getKind() != kind) {
    throwError_stringb(
      "expected " << kindCommonName(kind) <<
      ", not " << getKindCommonName());
  }
}


// Define a check function that calls `isXXX()` rather than checking the
// kind directly, perhaps because there is more than one kind code that
// qualifies (or just for uniformity).
#define DEFINE_CHECK_IS_KIND(Kind, desc)                   \
  void GDValueParser::checkIs##Kind() const                \
  {                                                        \
    if (!is##Kind()) {                                     \
      throwError_stringb(                                  \
        "expected " desc ", not " << getKindCommonName()); \
    }                                                      \
  }


// Define a zero-argument function that relays to the underlying value
// after checking the kind.
#define RELAY_KIND_SPECIFIC_QUERY0(Kind, rettype, name) \
  rettype GDValueParser::name() const                   \
  {                                                     \
    checkIs##Kind();                                    \
    return m_value->name();                             \
  }

// Same, for a one-argument function.
#define RELAY_KIND_SPECIFIC_QUERY1(Kind, rettype, name, type1, param1) \
  rettype GDValueParser::name(type1 param1) const                      \
  {                                                                    \
    checkIs##Kind();                                                   \
    return m_value->name(param1);                                      \
  }


// ---- Symbol ----
void GDValueParser::checkIsSymbol() const
{
  checkKind(GDVK_SYMBOL);
}

RELAY_QUERY(bool, isNull)
RELAY_QUERY(bool, isBool)

RELAY_KIND_SPECIFIC_QUERY0(Symbol, GDVSymbol, symbolGet)
RELAY_KIND_SPECIFIC_QUERY0(Symbol, std::string_view, symbolGetName)


// ---- Integer ----
DEFINE_CHECK_IS_KIND(Integer, "integer")

RELAY_KIND_SPECIFIC_QUERY0(Integer, GDVInteger, integerGet)
RELAY_KIND_SPECIFIC_QUERY0(Integer, bool, integerIsNegative)
RELAY_KIND_SPECIFIC_QUERY0(Integer, GDVInteger const &, largeIntegerGet)


// ---- SmallInteger ----
DEFINE_CHECK_IS_KIND(SmallInteger, "small integer")

RELAY_KIND_SPECIFIC_QUERY0(SmallInteger, GDVSmallInteger, smallIntegerGet)


// ---- String ----
DEFINE_CHECK_IS_KIND(String, "string")

RELAY_KIND_SPECIFIC_QUERY0(String, GDVString const &, stringGet)


// ---- Container ----
DEFINE_CHECK_IS_KIND(Container, "container")

RELAY_KIND_SPECIFIC_QUERY0(Container, GDVSize, containerSize)
RELAY_KIND_SPECIFIC_QUERY0(Container, bool, containerIsEmpty)


// ---- Sequence ----
DEFINE_CHECK_IS_KIND(Sequence, "sequence")

RELAY_KIND_SPECIFIC_QUERY0(Sequence, GDVSequence const &, sequenceGet)


GDValueParser GDValueParser::sequenceGetValueAt(GDVIndex index) const
{
  checkIsSequence();

  if (!( index < containerSize() )) {
    throwError_stringb(
      "expected sequence to have element at index " << index <<
      ", but it only has " << containerSize() << " elements");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_INDEX, index));
}


// ---- Tuple ----
DEFINE_CHECK_IS_KIND(Tuple, "tuple")

RELAY_KIND_SPECIFIC_QUERY0(Tuple, GDVTuple const &, tupleGet)


GDValueParser GDValueParser::tupleGetValueAt(GDVIndex index) const
{
  checkIsTuple();

  if (!( index < containerSize() )) {
    throwError_stringb(
      "expected tuple to have element at index " << index <<
      ", but it only has " << containerSize() << " elements");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_INDEX, index));
}


// ---- Set ----
DEFINE_CHECK_IS_KIND(Set, "set")

RELAY_KIND_SPECIFIC_QUERY0(Set, GDVSet const &, setGet)
RELAY_KIND_SPECIFIC_QUERY1(Set, bool, setContains, GDValue const &, elt)


GDValueParser GDValueParser::setGetValue(GDValue const &elt) const
{
  if (!setContains(elt)) {
    throwError_stringb(
      "expected set to have element " << elt <<
      ", but it does not");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_KEY,
               &( m_value->setGetValue(elt) )));
}


// ---- Map ----
DEFINE_CHECK_IS_KIND(Map, "map")

RELAY_KIND_SPECIFIC_QUERY0(Map, GDVMap const &, mapGet)
RELAY_KIND_SPECIFIC_QUERY1(Map, bool, mapContains, GDValue const &, key)


GDValueParser GDValueParser::mapGetKeyAt(GDValue const &key) const
{
  if (!mapContains(key)) {
    throwError_stringb(
      "expected map to have key " << key <<
      ", but it does not");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_KEY,
               &( m_value->mapGetKeyAt(key) )));
}


GDValueParser GDValueParser::mapGetValueAt(GDValue const &key) const
{
  if (!mapContains(key)) {
    throwError_stringb(
      "expected map to have key " << key <<
      ", but it does not");
  }

  // This is perhaps a little confusing:
  //
  // 1. We use `key`, which probably not in the container, to look up
  //    the corresponding key object that *is* in the container.
  //
  // 2. We store a pointer to the key that is in the container in the
  //    step object, while saying we want to use that key to navigate to
  //    the corresponding *value*.
  //
  // 3. The newly constructed parser object will append that step to its
  //    path, and also immediately apply it to navigate to the value.
  //
  // As the code is currently written, including the `mapContains` call
  // at the start of this function, we perform three map lookups (all
  // in the same map, with the same or equivalent keys).  That could be
  // optimized later.
  //
  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_VALUE,
               &( m_value->mapGetKeyAt(key) )));
}


RELAY_KIND_SPECIFIC_QUERY1(Map, bool, mapContainsSym, char const *, symName)


GDValueParser GDValueParser::mapGetValueAtSym(char const *symName) const
{
  return mapGetValueAt(GDVSymbol(symName));
}


GDValueParser GDValueParser::mapGetValueAtStr(char const *str) const
{
  return mapGetValueAt(GDVString(str));
}


std::optional<GDValueParser> GDValueParser::mapGetValueAtOpt(GDValue const &key) const
{
  if (!mapContains(key)) {
    return std::nullopt;
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_VALUE,
               &( m_value->mapGetKeyAt(key) )));
}


std::optional<GDValueParser> GDValueParser::mapGetValueAtSymOpt(char const *symName) const
{
  return mapGetValueAtOpt(GDVSymbol(symName));
}


std::optional<GDValueParser> GDValueParser::mapGetValueAtStrOpt(char const *str) const
{
  return mapGetValueAtOpt(GDVString(str));
}


// ---- OrderedMap ----
DEFINE_CHECK_IS_KIND(OrderedMap, "ordered map")
DEFINE_CHECK_IS_KIND(POMap, "(possibly ordered) map")

RELAY_KIND_SPECIFIC_QUERY0(OrderedMap, GDVOrderedMap const &, orderedMapGet)
RELAY_KIND_SPECIFIC_QUERY1(OrderedMap, bool, orderedMapContains, GDValue const &, key)


GDValueParser GDValueParser::orderedMapGetKeyAt(GDValue const &key) const
{
  if (!orderedMapContains(key)) {
    throwError_stringb(
      "expected ordered map to have key " << key <<
      ", but it does not");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_KEY,
               &( m_value->orderedMapGetKeyAt(key) )));
}


GDValueParser GDValueParser::orderedMapGetValueAt(GDValue const &key) const
{
  if (!orderedMapContains(key)) {
    throwError_stringb(
      "expected ordered map to have key " << key <<
      ", but it does not");
  }

  return GDValueParser(*this,
    GDVNavStep(GDVNavStep::STEP_IS_VALUE,
               &( m_value->orderedMapGetKeyAt(key) )));
}


bool GDValueParser::orderedMapContainsSym(char const *symName) const
{
  return orderedMapContains(GDVSymbol(symName));
}


GDValueParser GDValueParser::orderedMapGetValueAtSym(char const *symName) const
{
  return orderedMapGetValueAt(symName);
}


// ---- TaggedContainer ----
DEFINE_CHECK_IS_KIND(TaggedContainer, "tagged container")

RELAY_KIND_SPECIFIC_QUERY0(TaggedContainer, GDVSymbol, taggedContainerGetTag)
RELAY_KIND_SPECIFIC_QUERY0(TaggedContainer, std::string_view, taggedContainerGetTagName)


void GDValueParser::checkContainerTag(char const *symName) const
{
  if (taggedContainerGetTagName() != symName) {
    throwError_stringb(
      "expected container to have tag " << GDVSymbol(symName) <<
      ", but it instead has tag " << taggedContainerGetTag());
  }
}


// ---- Tagged Map ----
DEFINE_CHECK_IS_KIND(TaggedMap, "tagged map")


void GDValueParser::checkTaggedMapTag(char const *symName) const
{
  checkIsTaggedMap();
  checkContainerTag(symName);
}


// ---- Tagged OrderedMap ----
DEFINE_CHECK_IS_KIND(TaggedOrderedMap, "tagged ordered map")


void GDValueParser::checkTaggedOrderedMapTag(char const *symName) const
{
  checkIsTaggedOrderedMap();
  checkContainerTag(symName);
}


// --------------------------- XGDValueError ---------------------------
XGDValueError::~XGDValueError()
{}


XGDValueError::XGDValueError(
  GDValueParser const &parser,
  std::string &&message)
  : m_parser(parser),
    m_message(std::move(message))
{}


XGDValueError::XGDValueError(XGDValueError const &obj)
  : DMEMB(m_parser),
    DMEMB(m_message)
{}


std::string XGDValueError::getConflict() const
{
  return stringb(
    "At GDV path " << m_parser.pathString() << ": " << m_message);
}


// ------------------------------ GDVPTo -------------------------------
bool GDVPTo<bool>::f(GDValueParser const &p)
{
  p.checkIsSymbol();

  std::string_view symName = p.symbolGetName();
  if (symName == "true") {
    return true;
  }
  else if (symName == "false") {
    return false;
  }
  else {
    p.throwError(stringb(
      "expected symbol `true` or `false`, not " << p.valueGDVN()));
    return false;  // not reached
  }
}


int GDVPTo<int>::f(GDValueParser const &p)
{
  if (std::optional<int> n = convertNumberOpt<int>(p.smallIntegerGet())) {
    return *n;
  }
  else {
    p.throwError(stringb(
      "number too large to represent as `int`: " << p.valueGDVN()));
    return 0;  // not reached
  }
}


std::string GDVPTo<std::string>::f(GDValueParser const &p)
{
  return p.stringGet();
}


CLOSE_NAMESPACE(gdv)


// EOF
