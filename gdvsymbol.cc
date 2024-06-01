// gdvsymbol.cc
// Code for gdvsymbol.h.

// This file is in the public domain.

#include "gdvsymbol.h"                 // this module

// this dir
#include "codepoint.h"                 // isCIdentifierCharacter, isCIdentifierStartCharacter
#include "sm-macros.h"                 // PRETEND_USED, OPEN_NAMESPACE
#include "strtable.h"                  // StringTable
#include "xassert.h"                   // xassertdb

// libc++
#include <cassert>                     // assert
#include <cstring>                     // std::strcmp
#include <utility>                     // std::swap


OPEN_NAMESPACE(gdv)


StringTable *GDVSymbol::s_stringTable = nullptr;

char const *GDVSymbol::s_nullSymbolName = nullptr;


STATICDEF StringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new StringTable;
    s_nullSymbolName = s_stringTable->add("null");
  }
  return s_stringTable;
}


GDVSymbol::GDVSymbol(std::string const &s)
  : GDVSymbol(s.c_str())
{}


GDVSymbol::GDVSymbol(char const *p)
  : m_symbolName(lookupSymbolName(p))
{}


GDVSymbol::GDVSymbol(BypassSymbolLookupTag, char const *symbolName)
  : m_symbolName(symbolName)
{
  // Although the point of this ctor is to bypass the lookup for speed,
  // during development I'd like to check anyway.
  xassertdb(symbolName == getStringTable()->add(symbolName));
}


int compare(GDVSymbol const &a, GDVSymbol const &b)
{
  return std::strcmp(a.getSymbolName(), b.getSymbolName());
}


std::size_t GDVSymbol::size() const
{
  return std::strlen(m_symbolName);
}


STATICDEF bool GDVSymbol::validSymbolName(char const *name)
{
  if (!isCIdentifierStartCharacter(*name)) {
    return false;
  }
  ++name;

  for (; *name; ++name) {
    if (!isCIdentifierCharacter(*name)) {
      return false;
    }
  }

  return true;
}


STATICDEF char const *GDVSymbol::lookupSymbolName(char const *name)
{
  xassertPrecondition(validSymbolName(name));
  return getStringTable()->add(name);
}


STATICDEF char const *GDVSymbol::getNullSymbolName()
{
  getStringTable();
  assert(s_nullSymbolName);
  return s_nullSymbolName;
}


void GDVSymbol::swap(GDVSymbol &obj)
{
  std::swap(this->m_symbolName, obj.m_symbolName);
}


CLOSE_NAMESPACE(gdv)


// EOF
