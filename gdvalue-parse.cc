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


void checkGDValueKind(GDValue const &v, GDValueKind kind)
{
  if (v.getKind() != kind) {
    xformatsb("expected " << toString(kind) <<
              ", not " << v.getKindName());
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


void checkIsSequence(GDValue const &v)
{
  if (!v.isSequence()) {
    xformatsb("expected sequence, not " << v.getKindName());
  }
}


void checkIsTuple(GDValue const &v)
{
  if (!v.isTuple()) {
    xformatsb("expected tuple, not " << v.getKindName());
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


void checkIsMap(GDValue const &v)
{
  if (!v.isMap()) {
    xformatsb("expected map, not " << v.getKindName());
  }
}


void checkIsTaggedMap(GDValue const &v)
{
  checkGDValueKind(v, GDVK_TAGGED_MAP);
}


void checkIsTaggedContainer(GDValue const &v)
{
  if (!v.isTaggedContainer()) {
    xformatsb("expected tagged container, not " << v.getKindName());
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


GDValue tupleGetValueAt_parse(GDValue const &v, GDVIndex index)
{
  checkTupleIndex(v, index);
  return v.tupleGetValueAt(index);
}


GDValue mapGetSym_parse(GDValue const &v, char const *symName)
{
  checkIsMap(v);

  if (!v.mapContainsSym(symName)) {
    xformatsb("missing key: " << doubleQuote(symName));
  }

  return v.mapGetSym(symName);
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


CLOSE_NAMESPACE(gdv)


// EOF
