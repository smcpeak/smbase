// dni-vector-test.cc
// Tests for `dni-vector`.

#include "dni-vector-ops.h"            // this module

#include "smbase/distinct-number.h"    // smbase::DistinctNumber
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Distinct index type.
class TestTag;
using DNIndex = DistinctNumber<TestTag, unsigned>;

// Index values of the appropriate type.
DNIndex const i0(0);

// Vector of integers.
using DNIV = DNIVector<DNIndex, int>;


void testEmpty()
{
  DNIV vec;
  DNIV const &vec_c = vec;

  xassert(vec.empty());
  xassert(vec.size() == 0);

  xassert(vec.begin() == vec.end());
  xassert(vec.cbegin() == vec.cend());
  xassert(vec_c.begin() == vec_c.end());

  EXPECT_EQ(toGDValue(vec), GDVSequence{});
}


void testOne()
{
  DNIV vec;
  DNIV const &vec_c = vec;

  vec.push_back(11);

  xassert(!vec.empty());
  xassert(vec.size() == 1);

  xassert(vec.at(i0) == 11);
  xassert(vec_c.at(i0) == 11);

  #if ERRNUM == 1
    // Not allowed.
    xassert(vec.at(0) == 11);
  #endif

  xassert(vec[i0] == 11);
  xassert(vec_c[i0] == 11);

  #if ERRNUM == 2
    // Not allowed.
    xassert(vec[0] == 11);
  #endif

  int s = 0;
  for (int &elt : vec) {
    s += elt;
  }
  xassert(s == 11);

  s = 0;
  for (int const &elt : vec_c) {
    s += elt;
  }
  xassert(s == 11);

  EXPECT_EQ(toGDValue(vec), GDVSequence{11});

  vec[i0] = 12;
  EXPECT_EQ(toGDValue(vec), GDVSequence{12});

  DNIV vec2;
  vec.swap(vec2);
  xassert(vec.empty());
  EXPECT_EQ(toGDValue(vec2), GDVSequence{12});

  int thirteen = 13;
  vec.push_back(std::move(thirteen));
  EXPECT_EQ(toGDValue(vec), GDVSequence{13});
}


void testInitList()
{
  DNIV vec{1,2,3};
  EXPECT_EQ(toGDValue(vec), (GDVSequence{1,2,3}));
}


void testMoveInit()
{
  DNIV vec{1,2,3};
  DNIV vec2(std::move(vec));
  EXPECT_EQ(toGDValue(vec2), (GDVSequence{1,2,3}));
}


void testMoveAssign()
{
  DNIV vec{1,2,3};
  DNIV vec2;
  vec2 = std::move(vec);
  EXPECT_EQ(toGDValue(vec2), (GDVSequence{1,2,3}));
}


void testRelational()
{
  DNIV vecEmpty{};
  DNIV vecOne{1};
  DNIV vecAnotherOne{1};
  DNIV vecTwo{2};

  xassert(vecEmpty == vecEmpty);
  xassert(vecEmpty != vecOne);
  xassert(vecAnotherOne == vecOne);
  xassert(vecEmpty < vecOne);
  xassert(vecOne < vecTwo);
  xassert(vecOne <= vecTwo);
  xassert(vecTwo > vecOne);
  xassert(vecTwo >= vecOne);
}


void testWrite()
{
  DNIV vec{1,2,3};
  EXPECT_EQ(stringb(vec), "[1 2 3]");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_dni_vector()
{
  testEmpty();
  testOne();
  testInitList();
  testMoveInit();
  testMoveAssign();
  testRelational();
  testWrite();
}


// EOF
