// sum-tree-test.cc
// Tests for `sum-tree` module.

#include "smbase/sum-tree.h"           // module under test

#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, EMEMB
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <vector>                      // std::vector

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE



// Reference implementation of a SumTree: simple but inefficient.
template <typename T>
class RefTree {
public:      // types
  using size_type = std::size_t;
  using Summary = typename T::Summary;
  using LookupResult = std::pair<T const &, Summary>;

private:     // data
  // The primary sequence.
  std::vector<T> m_vec;

public:      // methods
  ~RefTree()
  {}

  RefTree()
    : m_vec()
  {}

  void selfCheck() const
  {}

  size_type size() const
  {
    return m_vec.size();
  }

  T const &atC(size_type index) const
  {
    xassertPrecondition(cc::z_le_lt(index, size()));
    return m_vec.at(index);
  }

  Summary summary() const
  {
    Summary s = Summary();
    for (T const &t : m_vec) {
      s += t.summary();
    }
    return s;
  }

  LookupResult lookup(Summary s) const
  {
    xassertPrecondition(Summary() <= s &&
                                     s < summary());

    for (T const &t : m_vec) {
      if (s < t.summary()) {
        return {t, s};
      }
      s -= t.summary();
    }

    xfailure("not reached");
  }

  std::vector<LookupResult> allElements() const
  {
    std::vector<LookupResult> ret;

    Summary s = Summary();
    for (T const &t : m_vec) {
      ret.push_back({t, s});
      s += t.summary();
    }

    return ret;
  }

  operator GDValue() const
  {
    GDValue m(GDVK_TAGGED_ORDERED_MAP, "Ref"_sym);

    GDV_WRITE_MEMBER_SYM(m_vec);

    return m;
  }

  void clear()
  {
    m_vec.clear();
  }

  void append(T const &t)
  {
    m_vec.push_back(t);
  }
};


// SumTree and RefTree.
template <typename T>
class BothTrees {
public:      // types
  using size_type = std::size_t;
  using Summary = typename T::Summary;
  using LookupResult = std::pair<T const &, Summary>;

public:      // data
  SumTree<T> m_sumTree;
  RefTree<T> m_refTree;

private:     // methods
  static void expectEqLR(
    LookupResult const &actualElt, LookupResult const &expectElt)
  {
    EXPECT_EQ_GDVSER(actualElt.first, expectElt.first);
    EXPECT_EQ(actualElt.second, expectElt.second);
  }

public:      // methods
  ~BothTrees()
  {}

  BothTrees()
    : m_sumTree(),
      m_refTree()
  {}

  void selfCheck() const
  {
    m_sumTree.selfCheck();
    m_refTree.selfCheck();

    // `size`
    EXPECT_EQ(m_sumTree.size(), m_refTree.size());

    // `atC`
    for (size_type i=0; i < m_sumTree.size(); ++i) {
      // This checks equality.
      atC(i);
    }

    // `summary`
    EXPECT_EQ(m_sumTree.summary(), m_refTree.summary());

    // `lookup`
    for (Summary s = Summary(); s < m_sumTree.summary(); ++s) {
      EXN_CONTEXT_EXPR(s);

      expectEqLR(m_sumTree.lookup(s), m_refTree.lookup(s));
    }

    // `allElements`
    std::vector<LookupResult> actualVec = m_sumTree.allElements();
    std::vector<LookupResult> expectVec = m_refTree.allElements();

    EXPECT_EQ(actualVec.size(), expectVec.size());

    for (std::size_t i=0; i < actualVec.size(); ++i) {
      EXN_CONTEXT_EXPR(i);

      expectEqLR(actualVec.at(i), expectVec.at(i));
    }
  }

  size_type size() const
  {
    size_type actual = m_sumTree.size();
    EXPECT_EQ(actual, m_refTree.size());
    return actual;
  }

  T const &atC(size_type index) const
  {
    T const &actual = m_sumTree.atC(index);
    T const &expect = m_refTree.atC(index);
    EXPECT_EQ_GDVSER(actual, expect);
    return actual;
  }

  Summary summary() const
  {
    Summary actual = m_sumTree.summary();
    EXPECT_EQ(actual, m_refTree.summary());
    return actual;
  }

  LookupResult lookup(Summary s) const
  {
    LookupResult actual = m_sumTree.lookup(s);
    LookupResult expect = m_refTree.lookup(s);

    EXPECT_EQ(actual.second, expect.second);
    xassert(&actual.first == &expect.first);

    return actual;
  }

  std::vector<LookupResult> allElements() const
  {
    return m_sumTree.allElements();
  }

  operator GDValue() const
  {
    GDValue m(GDVK_TAGGED_ORDERED_MAP, "BothTrees"_sym);

    GDV_WRITE_MEMBER_SYM(m_sumTree);
    GDV_WRITE_MEMBER_SYM(m_refTree);

    return m;
  }

  void append(T const &t)
  {
    m_sumTree.append(t);
    m_refTree.append(t);
  }

  void clear()
  {
    m_sumTree.clear();
    m_refTree.clear();
  }
};


// An integer, whose summary is itself.
class SummarizableInt {
public:      // types
  using Summary = int;

public:      // data
  int m_value{};

public:
  Summary summary() const
  {
    return m_value;
  }

  bool operator==(SummarizableInt const &obj) const
  {
    return EMEMB(m_value);
  }

  operator GDValue() const
  {
    return GDValue(m_value);
  }
};


void test_basics()
{
  TEST_FUNC();

  using BT = BothTrees<SummarizableInt>;
  BT both;
  both.selfCheck();
  EXPECT_EQ(both.size(), 0);
  EXPECT_EQ(both.summary(), 0);

  SummarizableInt i1{5};
  VPVAL(i1.m_value);
  both.append(i1);
  both.selfCheck();

  EXPECT_EQ(both.size(), 1);
  EXPECT_EQ(both.summary(), 5);
  EXPECT_EQ_GDVSER(both.atC(0), i1);

  EXPECT_EQ_GDVSER(both.allElements(), (std::vector<BT::LookupResult>{
    { i1, 0 },
  }));

  SummarizableInt i2{10};
  VPVAL(i2.m_value);
  both.append(i2);
  both.selfCheck();

  EXPECT_EQ(both.size(), 2);
  EXPECT_EQ(both.summary(), 15);
  EXPECT_EQ_GDVSER(both.atC(0), i1);
  EXPECT_EQ_GDVSER(both.atC(1), i2);

  EXPECT_EQ_GDVSER(both.allElements(), (std::vector<BT::LookupResult>{
    { i1, 0 },
    { i2, 5 },
  }));

  SummarizableInt i3{7};
  VPVAL(i3.m_value);
  both.append(i3);
  both.selfCheck();

  EXPECT_EQ(both.size(), 3);
  EXPECT_EQ(both.summary(), 22);
  EXPECT_EQ_GDVSER(both.atC(0), i1);
  EXPECT_EQ_GDVSER(both.atC(1), i2);
  EXPECT_EQ_GDVSER(both.atC(2), i3);

  EXPECT_EQ_GDVSER(both.allElements(), (std::vector<BT::LookupResult>{
    { i1, 0 },
    { i2, 5 },
    { i3, 15 },
  }));

  // Tree before any balance rotations are needed.
  VPVAL(toGDValue(both).asIndentedString());
  EXPECT_EQ_GDV(toGDValue(both), fromGDVN(R"(
    BothTrees[
      sumTree: SumTree[
        T: `{anonymous}::SummarizableInt`
        root: InteriorNode[
          summary: 22
          height: 2
          balanceFactor: -1
          size: 3
          left: Leaf[summary:5 height:0 data:5]
          right: InteriorNode[
            summary: 17
            height: 1
            balanceFactor: 0
            size: 2
            left: Leaf[summary:10 height:0 data:10]
            right: Leaf[summary:7 height:0 data:7]
          ]
        ]
      ]
      refTree: Ref[vec:[5 10 7]]
    ]
  )"));

  SummarizableInt i4{9};
  VPVAL(i4.m_value);
  both.append(i4);
  both.selfCheck();

  EXPECT_EQ(both.size(), 4);
  EXPECT_EQ(both.summary(), 31);
  EXPECT_EQ_GDVSER(both.atC(3), i4);

  EXPECT_EQ_GDVSER(both.allElements(), (std::vector<BT::LookupResult>{
    { i1, 0 },
    { i2, 5 },
    { i3, 15 },
    { i4, 22 },
  }));

  // Result of insertion that triggers balance rotations.
  VPVAL(toGDValue(both).asIndentedString());
  EXPECT_EQ_GDV(toGDValue(both), fromGDVN(R"(
    BothTrees[
      sumTree: SumTree[
        T: `{anonymous}::SummarizableInt`
        root: InteriorNode[
          summary: 31
          height: 2
          balanceFactor: 0
          size: 4
          left: InteriorNode[
            summary: 15
            height: 1
            balanceFactor: 0
            size: 2
            left: Leaf[summary:5 height:0 data:5]
            right: Leaf[summary:10 height:0 data:10]
          ]
          right: InteriorNode[
            summary: 16
            height: 1
            balanceFactor: 0
            size: 2
            left: Leaf[summary:7 height:0 data:7]
            right: Leaf[summary:9 height:0 data:9]
          ]
        ]
      ]
      refTree: Ref[vec:[5 10 7 9]]
    ]
  )"));

  for (int i=0; i < 20; ++i) {
    EXN_CONTEXT_EXPR(i);
    both.append({i});
    both.selfCheck();
    EXPECT_EQ(both.size(), 5 + i);
    EXPECT_EQ(both.summary(), 31 + (i * (i+1) / 2));
    EXPECT_EQ_GDVSER(both.atC(4 + i), SummarizableInt{i});
  }

  VPVAL(toGDValue(both).asIndentedString());

  // Test `clear`.
  both.clear();
  EXPECT_EQ(both.size(), 0);
  EXPECT_EQ(both.summary(), 0);
  both.selfCheck();

  // Make sure we can still `append`.
  both.append(i1);
  EXPECT_EQ(both.size(), 1);
  EXPECT_EQ(both.summary(), 5);
  EXPECT_EQ_GDVSER(both.atC(0), i1);
  both.selfCheck();
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sum_tree()
{
  test_basics();
}


// EOF
