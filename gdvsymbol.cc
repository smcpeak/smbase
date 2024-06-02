// gdvsymbol.cc
// Code for gdvsymbol.h.

// This file is in the public domain.

#include "gdvsymbol.h"                 // this module

// this dir
#include "codepoint.h"                 // isCIdentifierCharacter, isCIdentifierStartCharacter
#include "sm-macros.h"                 // PRETEND_USED, OPEN_NAMESPACE
#include "stringb.h"                   // stringb
#include "indexed-string-table.h"      // smbase::IndexedStringTable
#include "xassert.h"                   // xassertdb

// libc++
#include <cstring>                     // std::strcmp
#include <iostream>                    // std::ostream
#include <utility>                     // std::swap

using namespace smbase;


OPEN_NAMESPACE(gdv)


IndexedStringTable *GDVSymbol::s_stringTable = nullptr;

GDVSymbol::Index GDVSymbol::s_nullSymbolIndex = 0;


STATICDEF IndexedStringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new IndexedStringTable;
    s_nullSymbolIndex = s_stringTable->add("null");

    // TODO: Maybe turn this into a symbolic constant?
    xassert(s_nullSymbolIndex == 0);
  }
  return s_stringTable;
}


GDVSymbol::GDVSymbol(std::string_view const &s)
  : m_symbolIndex(lookupSymbolIndex(s))
{}


GDVSymbol::GDVSymbol(DirectIndexTag, Index symbolIndex)
  : m_symbolIndex(symbolIndex)
{
  xassert(getStringTable()->validIndex(symbolIndex));
}


int compare(GDVSymbol const &a, GDVSymbol const &b)
{
  return GDVSymbol::getStringTable()->compareIndexedStrings(
    a.m_symbolIndex, b.m_symbolIndex);
}


std::size_t GDVSymbol::size() const
{
  return getSymbolName().size();
}


std::string_view GDVSymbol::getSymbolName() const
{
  return getStringTable()->get(m_symbolIndex);
}


STATICDEF bool GDVSymbol::validSymbolName(std::string_view name)
{
  if (name.empty()) {
    return false;
  }

  for (std::size_t i = 0; i < name.size(); ++i) {
    if (i == 0) {
      if (!isCIdentifierStartCharacter(name[i])) {
        return false;
      }
    }
    else {
      if (!isCIdentifierCharacter(name[i])) {
        return false;
      }
    }
  }

  return true;
}


STATICDEF GDVSymbol::Index GDVSymbol::lookupSymbolIndex(
  std::string_view name)
{
  xassertPrecondition(validSymbolName(name));
  return getStringTable()->add(name);
}


STATICDEF bool GDVSymbol::validIndex(Index i)
{
  return getStringTable()->validIndex(i);
}


STATICDEF int GDVSymbol::compareIndices(Index a, Index b)
{
  return getStringTable()->compareIndexedStrings(a, b);
}


STATICDEF GDVSymbol::Index GDVSymbol::getNullSymbolIndex()
{
  getStringTable();

  // This is always zero.
  return s_nullSymbolIndex;
}


void GDVSymbol::swap(GDVSymbol &obj)
{
  using std::swap;
  swap(this->m_symbolIndex, obj.m_symbolIndex);
}


void GDVSymbol::write(std::ostream &os) const
{
  os << getSymbolName();
}


std::string GDVSymbol::asString() const
{
  return stringb(*this);
}


CLOSE_NAMESPACE(gdv)


// EOF
