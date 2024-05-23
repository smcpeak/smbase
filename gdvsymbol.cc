// gdvsymbol.cc
// Code for gdvsymbol.h.

#include "gdvsymbol.h"                 // this module

// this dir
#include "sm-macros.h"                 // PRETEND_USED
#include "strtable.h"                  // StringTable

// libc++
#include <cassert>                     // assert
#include <cstring>                     // std::strcmp
#include <utility>                     // std::swap

// Crude diagnostic facility.
//
// TODO: Move the better tracing module from print-clang-ast into smbase
// and use it here.
#if 0
  #include <iostream>                  // std::{cout, endl}
  #define TRACE(stuff) std::cout << stuff << std::endl /* user ; */
#else
  #define TRACE(stuff) /*nothing*/
#endif


namespace gdv {


StringTable *GDVSymbol::s_stringTable = nullptr;

char const *GDVSymbol::s_emptySymbolName = nullptr;

std::size_t GDVSymbol::s_numSymbolCtorCalls = 0;

std::size_t GDVSymbol::s_numSymbolDtorCalls = 0;


/*static*/ StringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new StringTable;
    s_emptySymbolName = s_stringTable->add("");
  }
  return s_stringTable;
}


/*static*/ char const *GDVSymbol::getEmptySymbolName()
{
  getStringTable();
  assert(s_emptySymbolName);
  return s_emptySymbolName;
}


/*static*/ void GDVSymbol::incCtorCalls(GDVSymbol *ptr)
{
  ++s_numSymbolCtorCalls;

  PRETEND_USED(ptr);
  TRACE("ctor " << s_numSymbolCtorCalls << ": " << (void*)ptr);
}


/*static*/ void GDVSymbol::incDtorCalls(GDVSymbol *ptr)
{
  ++s_numSymbolDtorCalls;

  PRETEND_USED(ptr);
  TRACE("dtor " << s_numSymbolDtorCalls << ": " << (void*)ptr);
}


GDVSymbol::GDVSymbol(std::string const &s)
  : GDVSymbol(s.c_str())
{}


GDVSymbol::GDVSymbol(char const *p)
  : m_symbolName(getStringTable()->add(p))
{
  incCtorCalls(this);
}


int compare(GDVSymbol const &a, GDVSymbol const &b)
{
  return std::strcmp(a.getSymbolName(), b.getSymbolName());
}


void GDVSymbol::swap(GDVSymbol &obj)
{
  std::swap(this->m_symbolName, obj.m_symbolName);
}


} // namespace gdv


// EOF
