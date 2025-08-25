// sm-test-order.h
// Check that ordering tests yield expected results.

// See license.txt for copyright and terms of use.

// This is split from `sm-test` for dependency reasons.

#ifndef SMBASE_SM_TEST_ORDER_H
#define SMBASE_SM_TEST_ORDER_H

#include "smbase/compare-util-iface.h" // smbase::compare
#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cstddef>                     // std::size_t
#include <functional>                  // std::reference_wrapper
#include <vector>                      // std::vector


// A reference to a value, and a string label.
template <typename T>
class LabeledValue {
public:      // data
  // The value.
  T const &m_value;

  // The label.
  char const *m_label;

public:      // methods
  LabeledValue(T const &value, char const *label)
    : IMEMBFP(value),
      IMEMBFP(label)
  {}

  T const &operator*() const { return m_value; }
};


// Check that comparing `a` and `b`, in that order, yields `expect`.
template <typename T>
void expectCompareOneWay(
  LabeledValue<T> const &a,
  LabeledValue<T> const &b,
  int expect)
{
  EXN_CONTEXT("a=" << a.m_label << "=" << *a);
  EXN_CONTEXT("b=" << b.m_label << "=" << *b);

  using smbase::compare;
  EXPECT_EQ(compare(*a, *b), expect);

  // Verify direct binary comparison.
  EXPECT_EQ((*a == *b), (expect == 0));
  EXPECT_EQ((*a != *b), (expect != 0));
  EXPECT_EQ((*a <= *b), (expect <= 0));
  EXPECT_EQ((*a <  *b), (expect <  0));
  EXPECT_EQ((*a >= *b), (expect >= 0));
  EXPECT_EQ((*a >  *b), (expect >  0));
}


// Check that comparing `a` and `b` yields `expect`, and that comparing
// them in the opposite order yields the opposite result.
template <typename T>
void expectCompare(
  LabeledValue<T> const &a,
  LabeledValue<T> const &b,
  int expect)
{
  expectCompareOneWay(a, b, expect);
  expectCompareOneWay(b, a, -expect);
}


// Macro to stringify arguments.
#define EXPECT_COMPARE(a, b, expect) \
  expectCompare(LabeledValue(a, #a), LabeledValue(b, #b), expect)


// Check that the elements of `vec` are in strictly increasing order by
// doing a full set of pairwise comparisons.
//
// Note that, apparently due to the way `std::reference_wrapper` works,
// the arguments must be variables, not general expressions.
template <typename T>
void checkStrictlyOrdered(
  char const *label,
  std::vector<std::reference_wrapper<T const>> const &vec)
{
  EXN_CONTEXT("checkStrictlyOrdered: " << label);

  for (std::size_t i = 0; i < vec.size(); ++i) {
    EXN_CONTEXT_EXPR(i);
    EXPECT_COMPARE(vec[i], vec[i], 0);

    for (std::size_t j = i+1; j < vec.size(); ++j) {
      EXN_CONTEXT_EXPR(j);
      EXPECT_COMPARE(vec[i], vec[j], -1);
    }
  }
}


// Macro to stringify arguments.
#define EXPECT_STRICTLY_ORDERED(T, ...)                           \
  checkStrictlyOrdered(                                           \
    SM_PP_STRINGIFY_ARGS(__VA_ARGS__),                            \
    std::vector<std::reference_wrapper<T const>>({__VA_ARGS__}))


#endif // SMBASE_SM_TEST_ORDER_H
