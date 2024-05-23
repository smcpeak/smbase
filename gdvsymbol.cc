// gdvsymbol.cc
// Code for gdvsymbol.h.

#include "gdvsymbol.h"                 // this module

// this dir
#include "sm-macros.h"                 // PRETEND_USED
#include "strtable.h"                  // StringTable
#include "xassert.h"                   // xassertdb

// libc++
#include <cassert>                     // assert
#include <cstring>                     // std::strcmp
#include <utility>                     // std::swap


namespace gdv {


StringTable *GDVSymbol::s_stringTable = nullptr;

char const *GDVSymbol::s_emptySymbolName = nullptr;


/*static*/ StringTable *GDVSymbol::getStringTable()
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


/*static*/ char const *GDVSymbol::getEmptySymbolName()
{
  getStringTable();
  assert(s_emptySymbolName);
  return s_emptySymbolName;
}


void GDVSymbol::swap(GDVSymbol &obj)
{
  std::swap(this->m_symbolName, obj.m_symbolName);
}


} // namespace gdv


// EOF
