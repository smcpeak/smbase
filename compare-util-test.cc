// compare-util-test.cc
// Tests for `compare-util`.

#include "smbase/compare-util.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, TABLESIZE
#include "smbase/sm-test-order.h"      // EXPECT_STRICTLY_ORDERED
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/xassert.h"            // xassert

#include <memory>                      // std::{make_unique, unique_ptr}
#include <string>                      // std::string

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class Data {
public:      // data
  // Simple scalar.
  int m_n;

  // Class-typed data member.
  std::string m_s;

  // Owner pointer.
  std::unique_ptr<int> m_intPtr;

public:      // methods
  Data(int n, std::string const &s, int ipValue)
    : m_n(n),
      m_s(s),
      m_intPtr(std::make_unique<int>(ipValue))
  {}

  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(Data);
};


// This is the prototypical pattern I expect to use to define
// `compareTo`.
int Data::compareTo(Data const &b) const
{
  // This is necessary to use `RET_IF_COMPARE_MEMBERS`.  It also
  // provides a degree of symmetry between the objects being compared.
  // It would be better of `compare` could be a friend function, but
  // that is not possible due to broken C++ access control; see comments
  // on `DECLARE_COMPARETO_AND_DEFINE_RELATIONALS`.
  auto const &a = *this;

  RET_IF_COMPARE_MEMBERS(m_n);
  RET_IF_COMPARE_MEMBERS(m_s);
  RET_IF_DEEP_COMPARE_PTR_MEMBERS(m_intPtr);

  return 0;
}


void test_sortedDataArray()
{
  Data data[] = {
    // Test `Data` objects in sorted order.
    { 3, "d", 7 },
    { 3, "e", 7 },
    { 4, "c", 7 },
    { 4, "d", 7 },
    { 4, "d", 8 },
    { 4, "e", 7 },
  };
  int n = TABLESIZE(data);

  for (int i = 0; i < n; ++i) {
    EXN_CONTEXT_EXPR(i);
    for (int j = 0; j < n; ++j) {
      EXN_CONTEXT_EXPR(j);
      xassert(compare(data[i], data[j]) == compare(i, j));
    }
  }
}


// Exercise case of a subclass.
class MoreData : public Data {
public:      // data
  int m_z;

public:      // methods
  MoreData(int n, std::string s, int ipv, int z)
    : Data(n, s, ipv),
      m_z(z)
  {}

  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(MoreData);
};

int MoreData::compareTo(MoreData const &b) const
{
  auto const &a = *this;
  RET_IF_COMPARE_SUBOBJS(Data);
  RET_IF_COMPARE_MEMBERS(m_z);
  return 0;
}


void test_sortedMoreDataArray()
{
  MoreData data[] = {
    { 3, "d", 7, 9 },
    { 3, "e", 7, 8 },
    { 3, "e", 7, 9 },
  };
  int n = TABLESIZE(data);

  for (int i = 0; i < n; ++i) {
    EXN_CONTEXT_EXPR(i);
    for (int j = 0; j < n; ++j) {
      EXN_CONTEXT_EXPR(j);
      xassert(compare(data[i], data[j]) == compare(i, j));
    }
  }
}


// Simple wrapped integer class with comparison to itself and `int`.
class Integer {
public:      // data
  int m_x;

public:      // methods
  Integer(int x) : m_x(x) {}

  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(Integer);
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS_TO_OTHER(Integer, int);

  // This is not unused; `clangd` is confused.
  [[maybe_unused]]
  friend std::ostream &operator<<(std::ostream &os, Integer const &obj)
    { return os << obj.m_x; }
};

int Integer::compareTo(Integer const &b) const
{
  using smbase::compare;
  auto const &a = *this;

  return COMPARE_MEMBERS(m_x);
}

int Integer::compareTo(int const &b) const
{
  using smbase::compare;

  return compare(m_x, b);
}


void test_compareInteger()
{
  Integer i0(0);
  Integer i1(1);
  Integer i2(2);
  EXPECT_STRICTLY_ORDERED(Integer, i0, i1, i2);

  EXPECT_COMPARE(i0, 0,  0);
  EXPECT_COMPARE(i0, 1, -1);
  EXPECT_COMPARE(i0, 2, -1);

  EXPECT_COMPARE(i1, 0, +1);
  EXPECT_COMPARE(i1, 1,  0);
  EXPECT_COMPARE(i1, 2, -1);

  EXPECT_COMPARE(i2, 0, +1);
  EXPECT_COMPARE(i2, 1, +1);
  EXPECT_COMPARE(i2, 2,  0);
}


CLOSE_ANONYMOUS_NAMESPACE


void test_compare_util()
{
  test_sortedDataArray();
  test_sortedMoreDataArray();
  test_compareInteger();
}


// EOF
