// gdvtuple-test.cc
// Tests for `gdvtuple` module.

#include "gdvtuple.h"                  // module under test

#include "gdvalue.h"                   // gdv::GDValue
#include "sm-test.h"                   // EXPECT_EQ
#include "xassert.h"                   // xassert

using namespace gdv;
using namespace smbase;


static void testConstructors()
{
  GDVTuple t1;
  xassert(t1.empty());
  EXPECT_EQ(t1.size(), 0);

  GDVTuple t2(3, GDValue());
  EXPECT_EQ(t2.size(), 3);

  GDVTuple t3(3);
  xassert(t2 == t3);
  xassert(t2 > t1);

  GDVTuple t4(t3);
  xassert(t4 == t3);

  GDVTuple t5((GDVTuple()));
  xassert(t5 == t1);

  GDVTuple t6{1,2,3,4};
  EXPECT_EQ(t6.size(), 4);

  GDVTuple t7(GDVTuple{1,2,3});
  EXPECT_EQ(t7.size(), 3);

  // Having trouble getting the move ctor to run.
  GDVTuple t8(std::move(t7));
  EXPECT_EQ(t8.size(), 3);
}


static void testAssignment()
{
  GDVTuple t1{1};
  GDVTuple t2{1,2};

  GDVTuple t3 = t1;
  EXPECT_EQ(t3.size(), 1);

  t3 = t2;
  EXPECT_EQ(t3.size(), 2);

  t3 = GDVTuple{1,2,3,4};
  EXPECT_EQ(t3.size(), 4);

  t3 = {1,2,3,4,5};
  EXPECT_EQ(t3.size(), 5);
}


static void testElementAccess()
{
  GDVTuple t1{1,2};
  EXPECT_EQ(t1.at(0), GDValue(1));
  EXPECT_EQ(t1.at(1), GDValue(2));

  GDVTuple const &ct1 = t1;
  EXPECT_EQ(ct1.at(0), GDValue(1));
  EXPECT_EQ(ct1.at(1), GDValue(2));

  EXPECT_EQ(t1[0], GDValue(1));
  EXPECT_EQ(t1[1], GDValue(2));

  EXPECT_EQ(ct1[0], GDValue(1));
  EXPECT_EQ(ct1[1], GDValue(2));
}


static void testIterators()
{
  GDVTuple t1{1,2,3};
  GDVTuple const &ct1 = t1;

  int sum = 0;
  for (GDValue &v : t1) {
    sum += v.smallIntegerGet();
  }
  EXPECT_EQ(sum, 6);

  sum = 0;
  for (GDValue const &v : ct1) {
    sum += v.smallIntegerGet();
  }
  EXPECT_EQ(sum, 6);

  sum = 0;
  for (auto it = t1.cbegin(); it != t1.cend(); ++it) {
    sum += (*it).smallIntegerGet();
  }
  EXPECT_EQ(sum, 6);
}


static void testModifiers()
{
  GDVTuple t;
  t.insert(t.begin(), GDValue(1));
  EXPECT_EQ(t[0], GDValue(1));

  t.push_back(2);
  EXPECT_EQ(t[1], GDValue(2));

  t.erase(t.begin());
  EXPECT_EQ(t[0], GDValue(2));

  GDValue three(3);
  t.push_back(three);
  EXPECT_EQ(t[1], three);

  GDVTuple t2(t);
  t2.resize(3);
  EXPECT_EQ(t2.at(2), GDValue());

  t2.clear();
  EXPECT_EQ(t2.size(), 0);

  t.swap(t2);
  EXPECT_EQ(t2.size(), 2);
}


// Called by unit-tests.cc.
void test_gdvtuple()
{
  testConstructors();
  testAssignment();
  testElementAccess();
  testIterators();
  testModifiers();
}


// EOF
