// gdvsymbol.cc
// Code for gdvsymbol.h.

#include "gdvsymbol.h"                 // this module

// this dir
#include "strtable.h"                  // StringTable

// libc++
#include <cstring>                     // std::strcmp


namespace gdv {


StringTable *GDVSymbol::s_stringTable = nullptr;


/*static*/ StringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new StringTable;
  }
  return s_stringTable;
}


GDVSymbol::GDVSymbol()
  : GDVSymbol("")
{}


GDVSymbol::GDVSymbol(std::string const &s)
  : GDVSymbol(s.c_str())
{}


GDVSymbol::GDVSymbol(char const *p)
  : m_symbolName(getStringTable()->add(p))
{}


int compare(GDVSymbol const &a, GDVSymbol const &b)
{
  return std::strcmp(a.getSymbolName(), b.getSymbolName());
}


} // namespace gdv


// EOF
