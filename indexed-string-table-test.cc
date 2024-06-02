// indexed-string-table-test.cc
// Tests for `indexed-string-table` module.

#include "indexed-string-table.h"      // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi, smbase_loopj
#include "sm-random.h"                 // sm_random
#include "sm-test.h"                   // EXPECT_EQ, DIAG, tout

#include <cstdlib>                     // std::{atoi, getenv}
#include <map>                         // std::map
#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testFixed()
{
  DIAG("---- testFixed ----");

  IndexedStringTable st;
  EXPECT_EQ(st.size(), 0);
  st.selfCheck();

  using Index = IndexedStringTable::Index;

  Index iFoo = st.add("foo");
  EXPECT_EQ(st.size(), 1);
  EXPECT_EQ(iFoo, 0);
  EXPECT_EQ(st.get(iFoo), "foo");
  EXPECT_EQ(st.add("foo"), iFoo);
  st.selfCheck();

  Index iBar = st.add("bar");
  EXPECT_EQ(st.size(), 2);
  EXPECT_EQ(iBar, 1);
  EXPECT_EQ(st.get(iBar), "bar");
  EXPECT_EQ(st.add("bar"), iBar);
  EXPECT_EQ(st.get(iFoo), "foo");
  EXPECT_EQ(st.add("foo"), iFoo);
  st.selfCheck();

  std::string hasNul("has\0nul", 7);
  Index iHN = st.add(hasNul);
  EXPECT_EQ(st.size(), 3);
  EXPECT_EQ(iHN, 2);
  EXPECT_EQ(st.get(iHN), hasNul);
  EXPECT_EQ(st.add(hasNul), iHN);
  st.selfCheck();

  // Just "has" without any more characters.  This would collide with
  // the previous entry if we stopped at the first NUL.
  std::string has("has");
  Index iH = st.add(has);
  EXPECT_EQ(st.size(), 4);
  EXPECT_EQ(iH, 3);
  EXPECT_EQ(st.get(iH), has);
  EXPECT_EQ(st.add(has), iH);
  EXPECT_EQ(st.get(iHN), hasNul);
  EXPECT_EQ(st.add(hasNul), iHN);
  st.selfCheck();

  std::string manyXs(2000, 'x');
  Index iXs = st.add(manyXs);
  EXPECT_EQ(st.size(), 5);
  EXPECT_EQ(iXs, 4);
  EXPECT_EQ(st.get(iXs), manyXs);
  EXPECT_EQ(st.add(manyXs), iXs);
  EXPECT_EQ(st.get(iBar), "bar");
  EXPECT_EQ(st.add("bar"), iBar);
  st.selfCheck();

  st.printStats(tout);
}


// Naive implementation of the same interface.
class AltIndexedStringTable {
public:      // data
  std::map<std::string, std::size_t> m_stringToIndex;
  std::vector<std::string> m_indexToString;

public:      // methods
  AltIndexedStringTable()
    : m_stringToIndex(),
      m_indexToString()
  {}

  std::size_t size() const
  {
    std::size_t ret = m_indexToString.size();
    xassert(m_stringToIndex.size() == ret);
    return ret;
  }

  std::size_t add(std::string_view sv)
  {
    std::string str(sv);
    auto it = m_stringToIndex.find(str);
    if (it == m_stringToIndex.end()) {
      std::size_t index = size();
      m_stringToIndex.insert({str, index});
      m_indexToString.push_back(str);
      return index;
    }
    else {
      return (*it).second;
    }
  }

  std::string_view get(std::size_t index)
  {
    return m_indexToString.at(index);
  }
};


std::string randomString()
{
  std::vector<char> vec;

  int len = sm_random(20);
  if (len == 19) {
    // Occasionally use a longer length.
    len = sm_random(2000);
  }
  smbase_loopi(len) {
    vec.push_back((char)(unsigned char)sm_random(256));
  }

  return std::string(vec.begin(), vec.end());
}


void testRandom()
{
  DIAG("--- testRandom ----");

  using Index = IndexedStringTable::Index;

  int iters = 100;
  if (char const *itersStr = std::getenv("INDEXED_STRING_TABLE_ITERS")) {
    iters = std::atoi(itersStr);
    PVAL(iters);
  }

  smbase_loopi(iters) {
    IndexedStringTable st;
    AltIndexedStringTable altST;

    smbase_loopj(iters) {
      EXPECT_EQ(st.size(), altST.size());

      // Add
      {
        std::string str = randomString();
        Index index1 = st.add(str);
        std::size_t index2 = altST.add(str);
        EXPECT_EQ(index1, index2);
      }

      // Probe.
      {
        Index index = sm_random(st.size());
        std::string_view sv1 = st.get(index);
        std::string_view sv2 = altST.get(index);
        EXPECT_EQ(sv1, sv2);
      }
    }
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_indexed_string_table()
{
  testFixed();
  testRandom();
}


// EOF
