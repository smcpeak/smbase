// optional-util-test.cc
// Tests for `optional-util`.

#include "optional-util.h"             // module under test

#include "smbase/optional-opll.h"      // operator<<(std::optional)
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <algorithm>                   // std::{min, max}
#include <functional>                  // std::invoke
#include <optional>                    // std::optional
#include <type_traits>                 // std::{invoke_result_t, remove_reference_t}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void testOptionalToString()
{
  std::optional<int> none;
  std::optional<int> one(1);

  EXPECT_EQ(optionalToString(none, "NONE"), "NONE");
  EXPECT_EQ(optionalToString(one, "NONE"), "1");
}


void testLiftToOptional()
{
  std::optional<int> none;
  std::optional<int> one(1);
  std::optional<int> two(2);

  // Minimum.
  {
    // Work around the problem of passing an overloaded function to a
    // higher-order function.
    auto myMin = [](int a, int b) -> int { return std::min(a,b); };

    EXPECT_EQ(liftToOptional(none, none, myMin), none);
    EXPECT_EQ(liftToOptional(one, none, myMin), one);
    EXPECT_EQ(liftToOptional(none, one, myMin), one);
    EXPECT_EQ(liftToOptional(two, one, myMin), one);

    // Do one with an explicit cast instead of an intermediate lambda to
    // resolve the target.
    EXPECT_EQ(liftToOptional(one, none,
      static_cast<int const & (*)(int const &, int const &)>(std::min<int>)),
      one);
  }

  // Maximum.
  {
    auto myMax = [](int a, int b) -> int { return std::max(a,b); };

    EXPECT_EQ(liftToOptional(two, one, myMax), two);
  }

  // Addition.
  {
    EXPECT_EQ(liftToOptional(one, two,
                             [](int a, int b) -> int { return a+b; }),
              std::optional<int>(3));
  }
}


void test_optAccumulateMax()
{
  std::optional<int> n;
  optAccumulateMax(n, 2);
  EXPECT_EQ(*n, 2);
  optAccumulateMax(n, 3);
  EXPECT_EQ(*n, 3);
  optAccumulateMax(n, 1);
  EXPECT_EQ(*n, 3);
}


std::string intToString(int n)
{
  return stringb(n);
}


void test_optInvoke()
{
  std::optional<int> n;
  std::optional<std::string> s = optInvoke(intToString, n);
  EXPECT_EQ(s.has_value(), false);

  n = 3;
  s = optInvoke(intToString, n);
  EXPECT_EQ(s.has_value(), true);
  EXPECT_EQ(*s, "3");
}


struct Data {
  int m_x;

  Data(int x) : m_x(x) {}
  int method() const { return 5; }
};


/* This alternative definition is uses `std::invoke` so it works with
   pointer-to-member (PTM) values, as illustrated below, whereas my
   `optInvoke` does not.

   The main downside is the heavier dependencies, as `std::invoke` is
   in `<functional>` which is quite large (>30kLOC).  Since I don't need
   PTM support, at least not right now, I'll just keep this here in
   reserve.

   Should I need PTM, I think I could just add another overload too.
*/
template <typename FUNC, typename T>
auto optInvokeAlt(FUNC &&f, std::optional<T> const &opt)
  -> std::optional<std::remove_reference_t<std::invoke_result_t<FUNC, T>>>
{
  if (opt) {
    return std::make_optional(std::invoke(f, *opt));
  }
  else {
    return std::nullopt;
  }
}


void test_optInvokeAlt()
{
  // The same things as above work.
  std::optional<int> n;
  std::optional<std::string> s = optInvokeAlt(intToString, n);
  EXPECT_EQ(s.has_value(), false);
  n = 3;
  s = optInvokeAlt(intToString, n);
  EXPECT_EQ(s.has_value(), true);
  EXPECT_EQ(*s, "3");

  // But in addition you can use a pointer-to-member function.
  std::optional<Data> d;
  n = optInvokeAlt(&Data::method, d);
  EXPECT_EQ(n.has_value(), false);
  d = Data{4};
  n = optInvokeAlt(&Data::method, d);
  EXPECT_EQ(n.has_value(), true);
  EXPECT_EQ(*n, 5);

  // This does not work with my definition.
  //n = optInvoke(&Data::method, d);

  // And even pointer-to-member data.
  d = std::nullopt;
  n = optInvokeAlt(&Data::m_x, d);
  EXPECT_EQ(n.has_value(), false);
  d = Data{4};
  n = optInvokeAlt(&Data::m_x, d);
  EXPECT_EQ(n.has_value(), true);
  EXPECT_EQ(*n, 4);

  // This also would not work with mine.
  //n = optInvoke(&Data::m_x, d);
}


void test_optFromOpt()
{
  std::optional<int> i;
  std::optional<Data> d = optFromOpt<Data>(i);
  EXPECT_FALSE(d.has_value());

  i = 3;
  d = optFromOpt<Data>(i);
  EXPECT_EQ(d->m_x, 3);
}


void test_OPT_INVOKE_METHOD()
{
  std::optional<std::string> strOpt = "hi";
  EXPECT_EQ(*OPT_INVOKE_METHOD(strOpt, size), 2);

  EXPECT_EQ(*OPT_INVOKE_METHOD(strOpt, substr, 1), "i");

  std::optional<std::size_t> sizeOpt =
    OPT_INVOKE_METHOD(strOpt, size);
  EXPECT_EQ(*sizeOpt, 2);

  strOpt = std::nullopt;
  sizeOpt = OPT_INVOKE_METHOD(strOpt, size);
  EXPECT_FALSE(sizeOpt.has_value());
}


// This is a different way to define `OPT_INVOKE_METHOD`.  I think this
// is not as good but I'm keeping it here in case I need it later.
#if 0

// If `opt` has a value, invoke zero-arg `method` and wrap the result as
// an `optional`.  Otherwise return `nullopt`.
//
// We remove references from `RESULT` to handle the case where `method`
// returns a reference (e.g.) to a data member.
template <typename RESULT, typename T>
std::optional<std::remove_reference_t<RESULT>> optInvokeMethod(
  std::optional<T> const &opt, RESULT (T::*method)() const)
{
  if (opt) {
    return std::optional<std::remove_reference_t<RESULT>>(
             ((*opt).*method)());
  }
  else {
    return std::nullopt;
  }
}

// Given a `expr` and `method` such that `expr.method()` would be valid,
// get a pointer-to-member for `method` without explicitly mentioning
// the class that `expr` is (or is a reference to).
#define GET_PTR_TO_MEMBER_FROM_EXPR(expr, method) \
  (&std::remove_reference_t<decltype(expr)>::method)

/* Equivalent to:

     (opt? std::make_optional(opt->method()) : std::nullopt)

   but less verbose.
*/
#define OPT_INVOKE_METHOD(opt, method) \
  optInvokeMethod(opt, GET_PTR_TO_MEMBER_FROM_EXPR(*opt, method))

#endif



CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_optional_util()
{
  testOptionalToString();
  testLiftToOptional();
  test_optAccumulateMax();
  test_optInvoke();
  test_optInvokeAlt();
  test_optFromOpt();
  test_OPT_INVOKE_METHOD();
}


// EOF
