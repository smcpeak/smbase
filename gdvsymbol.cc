// gdvsymbol.cc
// Code for gdvsymbol.h.

// This file is in the public domain.

#include "gdvsymbol.h"                 // this module

// this dir
#include "sm-macros.h"                 // PRETEND_USED, OPEN_NAMESPACE
#include "strtable.h"                  // StringTable
#include "xassert.h"                   // xassertdb

// libc++
#include <cassert>                     // assert
#include <cstring>                     // std::strcmp
#include <utility>                     // std::swap


OPEN_NAMESPACE(gdv)


StringTable *GDVSymbol::s_stringTable = nullptr;

char const *GDVSymbol::s_emptySymbolName = nullptr;


STATICDEF StringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new StringTable;
    s_emptySymbolName = s_stringTable->add("");
  }
  return s_stringTable;
}


GDVSymbol::GDVSymbol(std::string const &s)
  : GDVSymbol(s.c_str())
{}


GDVSymbol::GDVSymbol(char const *p)
  : m_symbolName(getStringTable()->add(p))
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


STATICDEF char const *GDVSymbol::lookupSymbolName(char const *name)
{
  return getStringTable()->add(name);
}


STATICDEF char const *GDVSymbol::getEmptySymbolName()
{
  getStringTable();
  assert(s_emptySymbolName);
  return s_emptySymbolName;
}


void GDVSymbol::swap(GDVSymbol &obj)
{
  std::swap(this->m_symbolName, obj.m_symbolName);
}


CLOSE_NAMESPACE(gdv)


// EOF
