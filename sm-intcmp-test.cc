// sm-intcmp-test.cc
// Tests for `sm-intcmp` module.

#include "smbase/sm-intcmp.h"          // module under test

#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <cstdint>                     // std::int32_t, etc.
#include <limits>                      // std::numeric_limits

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


#define EXN_CONTEXT_TYPE(T) \
  EXN_CONTEXT(#T "=" << GetTypeName<T>::value) /* user ; */


template <typename T, typename U>
void check(T t, U u, int expect)
{
  TEST_FUNC();
  EXN_CONTEXT_TYPE(T);
  EXN_CONTEXT_TYPE(U);

  EXN_CONTEXT_EXPR(t);
  EXN_CONTEXT_EXPR(u);

  int actual = intcmp_compare<T,U>(t, u);
  EXPECT_EQ(actual, expect);

  EXPECT_EQ((intcmp_equal        <T,U>(t, u)), (expect == 0));
  EXPECT_EQ((intcmp_not_equal    <T,U>(t, u)), (expect != 0));
  EXPECT_EQ((intcmp_less         <T,U>(t, u)), (expect <  0));
  EXPECT_EQ((intcmp_greater      <T,U>(t, u)), (expect >  0));
  EXPECT_EQ((intcmp_less_equal   <T,U>(t, u)), (expect <= 0));
  EXPECT_EQ((intcmp_greater_equal<T,U>(t, u)), (expect >= 0));
}


// Smallest and largest.
#define SM(T) (std::numeric_limits<T>::min())
#define LG(T) (std::numeric_limits<T>::max())

// "Primitive" comparison in that it does not use `intcmp_compare`,
// which is part of the system under test.  This requires that the
// values have the same signedness.
#define PRIM_COMPARE(a, b) (((a)<(b))? -1 : ((b)<(a))? +1 : 0)


template <typename T, typename U>
void checkForTypes()
{
  TEST_FUNC();
  EXN_CONTEXT_TYPE(T);
  EXN_CONTEXT_TYPE(U);

  // The following code performs all 25 comparisons of:
  //
  //   t in { SM(T), -1, 0, +1, LG(T) }
  //
  // versus
  //
  //   u in { SM(U), -1, 0, +1, LG(U) }
  //
  // except it only does a subset when `T` and/or `U` is unsigned.

  //              t      u      expect
  //              -----  -----  ------

  if (std::is_signed_v<T>) {
    if (std::is_signed_v<U>) {
      // Whichever is wider has the smaller minimum.
      check<T, U>(SM(T), SM(U), -PRIM_COMPARE(sizeof(T), sizeof(U)));

      check<T, U>(SM(T),    -1, -1);

      check<T, U>(   -1, SM(U), +1);
      check<T, U>(   -1,    -1,  0);
    }

    // T is signed, U is signed or unsigned.
    check<T, U>  (SM(T),     0, -1);
    check<T, U>  (SM(T),     1, -1);
    check<T, U>  (SM(T), LG(U), -1);

    check<T, U>  (   -1,     0, -1);
    check<T, U>  (   -1,     1, -1);
    check<T, U>  (   -1, LG(U), -1);
  }

  else /* T is unsigned */ {
    if (std::is_signed_v<U>) {
      check<T, U>(    0, SM(U), +1);
      check<T, U>(    1, SM(U), +1);
      check<T, U>(LG(T), SM(U), +1);

      check<T, U>(    0,    -1, +1);
      check<T, U>(    1,    -1, +1);
      check<T, U>(LG(T),    -1, +1);
    }
  }

  check<T, U>    (    0,     0,  0);
  check<T, U>    (    1,     0, +1);
  check<T, U>    (LG(T),     0, +1);

  check<T, U>    (    0,     1, -1);
  check<T, U>    (    1,     1,  0);
  check<T, U>    (LG(T),     1, +1);

  check<T, U>    (    0, LG(U), -1);
  check<T, U>    (    1, LG(U), -1);
  check<T, U>    (LG(T), LG(U),
    // Whichever is wider has the larger maximum.  If they have the
    // same size, whichever is unsigned if larger.
    PRIM_COMPARE(sizeof(T)*2 + std::is_unsigned_v<T>,
                 sizeof(U)*2 + std::is_unsigned_v<U>));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_sm_intcmp()
{
  using std::int32_t;
  using std::uint32_t;
  using std::int64_t;
  using std::uint64_t;

  checkForTypes<int32_t, int32_t>();
  checkForTypes<uint32_t, int32_t>();
  checkForTypes<int32_t, uint32_t>();
  checkForTypes<uint32_t, uint32_t>();

  checkForTypes<int32_t, int64_t>();
  checkForTypes<uint32_t, int64_t>();
  checkForTypes<int32_t, uint64_t>();
  checkForTypes<uint32_t, uint64_t>();

  checkForTypes<int64_t, int32_t>();
  checkForTypes<uint64_t, int32_t>();
  checkForTypes<int64_t, uint32_t>();
  checkForTypes<uint64_t, uint32_t>();

  checkForTypes<int64_t, int64_t>();
  checkForTypes<uint64_t, int64_t>();
  checkForTypes<int64_t, uint64_t>();
  checkForTypes<uint64_t, uint64_t>();
}


// EOF
