// gdvsymbol.cc
// Code for gdvsymbol.h.

#include "gdvsymbol.h"                 // this module

#include "compare-util.h"              // COMPARE_MEMBERS


namespace gdv {


GDVSymbol::GDVSymbol()
  : m_symbolName()
{}


GDVSymbol::GDVSymbol(std::string const &s)
  : m_symbolName(s)
{}


GDVSymbol::GDVSymbol(char const *p)
  : m_symbolName(p)
{}


GDVSymbol::~GDVSymbol()
{}


int compare(GDVSymbol const &a, GDVSymbol const &b)
{
  using ::compare;
  return COMPARE_MEMBERS(m_symbolName);
}


} // namespace gdv


// EOF
