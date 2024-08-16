// gdvsymbol.cc
// Code for gdvsymbol.h.

// This file is in the public domain.

#include "gdvsymbol.h"                           // this module

// this dir
#include "smbase/codepoint.h"                    // isCIdentifierCharacter, isCIdentifierStartCharacter
#include "smbase/gdvalue-writer.h"               // GDValueWriter::writeOneQuotedStringChar
#include "smbase/sm-macros.h"                    // PRETEND_USED, OPEN_NAMESPACE
#include "smbase/stringb.h"                      // stringb
#include "smbase/indexed-string-table.h"         // smbase::IndexedStringTable
#include "smbase/xassert.h"                      // xassertdb

// libc++
#include <cstring>                               // std::strcmp
#include <iostream>                              // std::ostream
#include <string>                                // std::string
#include <string_view>                           // std::string_view
#include <utility>                               // std::swap

using namespace smbase;


OPEN_NAMESPACE(gdv)


IndexedStringTable * NULLABLE GDVSymbol::s_stringTable = nullptr;


STATICDEF IndexedStringTable *GDVSymbol::getStringTable()
{
  if (!s_stringTable) {
    s_stringTable = new IndexedStringTable;
    Index i = s_stringTable->add("null");
    xassert(i == s_nullSymbolIndex);
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


STATICDEF bool GDVSymbol::validUnquotedSymbolName(std::string_view name)
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


void GDVSymbol::swap(GDVSymbol &obj)
{
  using std::swap;
  swap(this->m_symbolIndex, obj.m_symbolIndex);
}


void GDVSymbol::write(std::ostream &os, bool forceQuotes) const
{
  std::string_view name = getSymbolName();
  if (!forceQuotes && validUnquotedSymbolName(name)) {
    os << name;
  }
  else {
    os << '`';
    for (char c : name) {
      bool const useUndelimitedHexEscapes = false;
      GDValueWriter::writeOneQuotedStringChar(os, c, '`',
        useUndelimitedHexEscapes);
    }
    os << '`';
  }
}


std::string GDVSymbol::asString() const
{
  return stringb(*this);
}


GDVSymbol operator ""_sym(char const *name, std::size_t len)
{
  return GDVSymbol(std::string_view(name, len));
}


CLOSE_NAMESPACE(gdv)


// EOF
