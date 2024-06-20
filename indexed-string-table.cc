// indexed-string-table.cc
// Code for `indexed-string-table` module.

#include "indexed-string-table.h"      // this module

#include "compare-util.h"              // compare
#include "overflow.h"                  // convertNumber
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "string-hash.h"               // smbase::stringHash
#include "xassert.h"                   // xassertPrecondition

#include <cstring>                     // std::memcpy
#include <iostream>                    // std::ostream
#include <string_view>                 // std::string_view


OPEN_NAMESPACE(smbase)


std::string_view IndexedStringTable::StoredString::getStringView() const
{
  if (m_index < 0) {
    LookupString const *ls = static_cast<LookupString const *>(this);

    return std::string_view(ls->m_string, m_size);
  }

  else {
    return std::string_view(reinterpret_cast<char const*>(this+1), m_size);
  }
}


STATICDEF void const *IndexedStringTable::getKeyFromSS(void *dataSS)
{
  return dataSS;
}


STATICDEF unsigned IndexedStringTable::hashSS(void const *dataSS)
{
  StoredString const *ss = static_cast<StoredString const *>(dataSS);

  std::string_view sv = ss->getStringView();
  return stringHash(sv.data(), sv.size());
}


STATICDEF bool IndexedStringTable::equalKeys(
  void const *dataSS1, void const *dataSS2)
{
  std::string_view sv1 =
    static_cast<StoredString const *>(dataSS1)->getStringView();
  std::string_view sv2 =
    static_cast<StoredString const *>(dataSS2)->getStringView();

  return sv1 == sv2;
}


IndexedStringTable::~IndexedStringTable()
{
  clear();
}


IndexedStringTable::IndexedStringTable()
  : m_allocator(),
    m_stringToIndex(&getKeyFromSS,
                    &hashSS,
                    &equalKeys),
    m_indexToString()
{}


IndexedStringTable::Index IndexedStringTable::size() const
{
  return static_cast<Index>(m_indexToString.size());
}


IndexedStringTable::Index IndexedStringTable::add(std::string_view str)
{
  // To perform a lookup, we make a temporary `LookupString` and
  // populate it with the details from `sv`.
  LookupString ls;
  ls.m_index = -1;           // It is not stored in the table.
  ls.m_size = static_cast<Size>(str.size());
  ls.m_string = str.data();

  if (void *existing = m_stringToIndex.get(&ls)) {
    StoredString *existingSS = static_cast<StoredString*>(existing);
    return existingSS->m_index;
  }

  // Must add a new string.
  std::size_t ssSize = sizeof(StoredString) + str.size();
  unsigned char *newBlock = m_allocator.allocate(ssSize);
  StoredString *newSS = reinterpret_cast<StoredString*>(newBlock);
  newSS->m_index = size();
  newSS->m_size = convertNumber<Size>(str.size());
  std::memcpy(reinterpret_cast<char*>(newSS+1),
              str.data(),
              str.size());

  // Insert it into the tables.
  m_stringToIndex.add(newSS, newSS);
  m_indexToString.push_back(newSS);

  return newSS->m_index;
}


std::string_view IndexedStringTable::get(Index index) const
{
  xassertPrecondition(validIndex(index));

  return m_indexToString.at(index)->getStringView();
}


int IndexedStringTable::compareIndexedStrings(Index a, Index b) const
{
  std::string_view svA = get(a);
  std::string_view svB = get(b);

  using ::compare;
  return compare(svA, svB);
}


void IndexedStringTable::clear()
{
  m_indexToString.clear();
  m_stringToIndex.clear();
  m_allocator.clear();
}


void IndexedStringTable::printStats(std::ostream &os) const
{
  m_allocator.printStats(os);
  m_stringToIndex.printStats(os);
}


void IndexedStringTable::selfCheck() const
{
  m_allocator.selfCheck();
  m_stringToIndex.selfCheck();

  Index i = 0;
  for (StoredString *ss : m_indexToString) {
    xassertInvariant(ss->m_index == i);
    xassertInvariant(m_stringToIndex.get(ss) == ss);

    ++i;
  }

  xassertInvariant(m_stringToIndex.getNumEntries() == size());
}


CLOSE_NAMESPACE(smbase)


// EOF
