// gdvalue-parse.cc
// Code for gdvalue-parse.h.

#include "gdvalue-parse.h"             // this module

#include "smbase/gdvalue.h"            // GDValue
#include "smbase/exc.h"                // xformatsb
#include "smbase/overflow.h"           // convertNumberOpt
#include "smbase/string-util.h"        // doubleQuote

#include <optional>                    // std::optional
#include <string_view>                 // std::string_view


OPEN_NAMESPACE(gdv)


// ------------------- Stand-alone parsing functions -------------------
void checkGDValueKind(GDValue const &v, GDValueKind kind)
{
  if (v.getKind() != kind) {
    xformatsb("expected " << kindCommonName(kind) <<
              ", not " << v.getKindCommonName());
  }
}


void checkIsSymbol(GDValue const &v)
{
  checkGDValueKind(v, GDVK_SYMBOL);
}


void checkIsSmallInteger(GDValue const &v)
{
  checkGDValueKind(v, GDVK_SMALL_INTEGER);
}


void checkIsString(GDValue const &v)
{
  checkGDValueKind(v, GDVK_STRING);
}


GDVString stringGet_parse(GDValue const &v)
{
  checkIsString(v);
  return v.stringGet();
}


void checkIsSequence(GDValue const &v)
{
  if (!v.isSequence()) {
    xformatsb("expected sequence, not " << v.getKindCommonName());
  }
}


void checkIsTuple(GDValue const &v)
{
  if (!v.isTuple()) {
    xformatsb("expected tuple, not " << v.getKindCommonName());
  }
}


void checkTupleIndex(GDValue const &v, GDVIndex index)
{
  checkIsTuple(v);
  if (index >= v.containerSize()) {
    xformatsb("attempt to access tuple at index " << index <<
              " but it only has " << v.containerSize() <<
              " elements");
  }
}


void checkIsSet(GDValue const &v)
{
  if (!v.isSet()) {
    xformatsb("expected set, not " << v.getKindCommonName());
  }
}


void checkIsMap(GDValue const &v)
{
  if (!v.isMap()) {
    xformatsb("expected map, not " << v.getKindCommonName());
  }
}


void checkIsTaggedMap(GDValue const &v)
{
  checkGDValueKind(v, GDVK_TAGGED_MAP);
}


void checkIsPOMap(GDValue const &v)
{
  if (!v.isPOMap()) {
    xformatsb("expected map or ordered map, not " <<
              v.getKindCommonName());
  }
}


void checkIsTaggedContainer(GDValue const &v)
{
  if (!v.isTaggedContainer()) {
    xformatsb("expected tagged container, not " << v.getKindCommonName());
  }
}


void checkContainerTag(GDValue const &v, char const *symName)
{
  checkIsTaggedContainer(v);

  std::string_view actual = v.taggedContainerGetTagName();
  if (actual != symName) {
    xformatsb("expected tag " << doubleQuote(symName) <<
              ", not " << doubleQuote(actual));
  }
}


void checkTaggedMapTag(GDValue const &v, char const *symName)
{
  checkGDValueKind(v, GDVK_TAGGED_MAP);
  checkContainerTag(v, symName);
}


void checkTaggedOrderedMapTag(GDValue const &v, char const *symName)
{
  checkGDValueKind(v, GDVK_TAGGED_ORDERED_MAP);
  checkContainerTag(v, symName);
}


GDValue tupleGetValueAt_parse(GDValue const &v, GDVIndex index)
{
  checkTupleIndex(v, index);
  return v.tupleGetValueAt(index);
}


GDValue mapGetSym_parse(GDValue const &v, char const *symName)
{
  checkIsPOMap(v);

  GDValue key{GDVSymbol(symName)};
  if (!v.mapContains(key)) {
    xformatsb("missing key: " << key.asString());
  }

  return v.mapGetValueAt(key);
}


GDValue mapGetSym_parseOpt(GDValue const &v, char const *symName)
{
  checkIsPOMap(v);

  GDValue key{GDVSymbol(symName)};
  if (!v.mapContains(key)) {
    return GDValue();
  }

  return v.mapGetValueAt(key);
}


GDValue mapGetValueAtStr_parse(GDValue const &v, char const *str)
{
  checkIsPOMap(v);

  GDValue key(str);
  if (!v.mapContains(key)) {
    xformatsb("missing key: " << key.asString());
  }

  return v.mapGetValueAt(key);
}


GDValue mapGetValueAtStr_parseOpt(GDValue const &v, char const *str)
{
  checkIsPOMap(v);

  GDValue key(str);
  if (!v.mapContains(key)) {
    return GDValue();
  }

  return v.mapGetValueAt(key);
}


// ------------------------------- GDVTo -------------------------------
bool GDVTo<bool>::f(GDValue const &v)
{
  checkIsSymbol(v);

  std::string_view symName = v.symbolGetName();
  if (symName == "true") {
    return true;
  }
  else if (symName == "false") {
    return false;
  }
  else {
    xformatsb("expected symbol `true` or `false`, not " << v.asString());
    return false;  // not reached
  }
}


int GDVTo<int>::f(GDValue const &v)
{
  checkIsSmallInteger(v);

  if (std::optional<int> n = convertNumberOpt<int>(v.smallIntegerGet())) {
    return *n;
  }
  else {
    xformatsb("too large to represent as `int`: " << v.asString());
    return 0;  // not reached
  }
}


std::string GDVTo<std::string>::f(GDValue const &v)
{
  checkIsString(v);

  return v.stringGet();
}


// ---------------------- Member de/serialization ----------------------
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


CLOSE_NAMESPACE(gdv)


// EOF
