// ordered-flat-map-test.cc
// Tests for `ordered-flat-map` module.

// This file is in the public domain.

#include "smbase/ordered-flat-map.h"             // module under test
#include "smbase/ordered-mru-flat-map.h"         // module under test

#include "smbase/exc.h"                          // EXN_CONTEXT_EXPR
#include "smbase/fasttime.h"                     // smbase::{fastTimeInitialize, fastTimeMilliseconds}
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/ordered-map.h"                  // smbase::OrderedMap
#include "smbase/save-restore.h"                 // SAVE_RESTORE
#include "smbase/sm-env.h"                       // smbase::envAsIntOr
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE, DMEMB, MDMEMB, smbase_loopi
#include "smbase/sm-random.h"                    // smbase::{RandomChoice, sm_random}
#include "smbase/sm-test.h"                      // EXPECT_EQ, EXPECT_EXN_SUBSTR, EnvRandomizedTestIters, TEST_FUNC, TEST_CASE

#include <string>                                // std::string
#include <utility>                               // std::move

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// The tests in this file exercise both OrderedFlatMap and
// OrderedMRUFlatMap, with the template parameter ORDERED_FLAT_MAP bound
// to one or the other.
template <typename ORDERED_FLAT_MAP>
void test_basics()
{
  ORDERED_FLAT_MAP m;
  m.selfCheck();

  EXPECT_EQ_GDV(m, GDVOrderedMap{});
  EXPECT_EQ_GDV(m, fromGDVN("[:]"));

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.setValueAtNewKey(1, "one");
  m.setValueAtNewKey(3, "three");
  m.setValueAtNewKey(2, "two");

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

  EXPECT_EQ_GDV(m, fromGDVN(R"([1:"one" 3:"three" 2:"two"])"));

  // Key-based lookup uses the intrinsic order.
  EXPECT_EQ(m.valueAtKey(1), "one");
  EXPECT_EQ(m.valueAtKey(2), "two");
  EXPECT_EQ(m.valueAtKey(3), "three");

  // Index-based lookup uses the extrinsic order.
  EXPECT_EQ(m.keyAtIndex(0), 1);
  EXPECT_EQ(m.valueAtIndex(0), "one");

  EXPECT_EQ(m.keyAtIndex(1), 3);
  EXPECT_EQ(m.valueAtIndex(1), "three");

  EXPECT_EQ(m.keyAtIndex(2), 2);
  EXPECT_EQ(m.valueAtIndex(2), "two");

  EXPECT_EQ(m.contains(0), false);
  EXPECT_EQ(m.contains(1), true);
  EXPECT_EQ(m.contains(2), true);
  EXPECT_EQ(m.contains(3), true);
  EXPECT_EQ(m.contains(4), false);
  m.selfCheck();

  // Copy ctor.
  {
    ORDERED_FLAT_MAP m2(m);
    EXPECT_EQ(m2.size(), 3);
    EXPECT_EQ(m2.valueAtKey(2), "two");
    m2.selfCheck();
  }

  // Invalid access.
  EXPECT_EXN(m.valueAtKey(4), XAssert);
  EXPECT_EXN(m.keyAtIndex(3), XAssert);
  EXPECT_EXN(m.valueAtIndex(3), XAssert);

  // Check default-constructed value.
  {
    std::string &valueRef = m.valueRefAtKey(4);
    EXPECT_EQ(valueRef, "");
    EXPECT_EQ(m.valueAtKey(4), "");

    // And ability to update via the reference.
    valueRef = "four";
    EXPECT_EQ(m.valueAtKey(4), "four");

    EXPECT_EQ_GDV(m, fromGDVN(R"([1:"one" 3:"three" 2:"two" 4:"four"])"));
  }

  // Move ctor.
  ORDERED_FLAT_MAP m2(std::move(m));
  EXPECT_EQ(m2.size(), 4);
  EXPECT_EQ(m2.valueAtKey(2), "two");
  m2.selfCheck();

  // The invariants of `m` should hold, even if its state is otherwise
  // indeterminate.
  m.selfCheck();
}


class CompareMod10 {
public:
  bool operator()(int a, int b) const
  {
    return (a % 10) < (b % 10);
  }
};


template <typename ORDERED_FLAT_MAP>
void test_differentCompare()
{
  ORDERED_FLAT_MAP m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.valueRefAtKey(1) = "one";
  m.valueRefAtKey(3) = "three";
  m.valueRefAtKey(2) = "two";

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

  EXPECT_EQ(m.valueAtKey(1), "one");
  EXPECT_EQ(m.valueAtKey(2), "two");
  EXPECT_EQ(m.valueAtKey(3), "three");

  // mod 10 behavior
  EXPECT_EQ(m.valueAtKey(13), "three");
}


// Ordered[MRU]FlatMap under test, and OrderedMap for reference.
template <typename KEY, typename VALUE, typename ORDERED_FLAT_MAP>
class OrderedMapPair {
public:      // types
  using size_type = typename ORDERED_FLAT_MAP::size_type;
  using FLAT_MAP = ORDERED_FLAT_MAP;

public:      // data
  // Object under test.
  ORDERED_FLAT_MAP m_flatMap;

  // Reference implementation.
  OrderedMap<KEY, VALUE> m_refMap;

public:
  ~OrderedMapPair() = default;

  void selfCheck() const
  {
    m_flatMap.selfCheck();
    m_refMap.selfCheck();

    // Verify queries agree.
    this->empty();
    size_type sz = this->size();

    for (size_type index=0; index < sz; ++index) {
      KEY key = this->keyAtIndex(index);
      this->valueAtIndex(index);
      this->valueAtKey(key);
    }
  }

  OrderedMapPair()
  {
    this->selfCheck();
  }

  OrderedMapPair(OrderedMapPair const &obj)
    : DMEMB(m_flatMap),
      DMEMB(m_refMap)
  {
    this->selfCheck();
  }

  OrderedMapPair(OrderedMapPair &&obj)
    : MDMEMB(m_flatMap),
      MDMEMB(m_refMap)
  {
    // Ensure the elements of `obj` remain equal.
    obj.clear();

    this->selfCheck();
    obj.selfCheck();
  }

  VALUE const &valueAtKey(KEY const &key) const
  {
    VALUE const &ret = m_flatMap.valueAtKey(key);
    EXPECT_EQ(ret, m_refMap.valueAtKey(key));
    return ret;
  }

  KEY const &keyAtIndex(size_type index) const
  {
    KEY const &ret = m_flatMap.keyAtIndex(index);
    EXPECT_EQ(ret, m_refMap.entryAtIndex(index).first);
    return ret;
  }

  VALUE const &valueAtIndex(size_type index) const
  {
    VALUE const &ret = m_flatMap.valueAtIndex(index);
    EXPECT_EQ(ret, m_refMap.entryAtIndex(index).second);
    return ret;
  }

  bool empty() const
  {
    bool ret = m_flatMap.empty();
    EXPECT_EQ(ret, m_refMap.empty());
    return ret;
  }

  size_type size() const
  {
    size_type ret = m_flatMap.size();
    EXPECT_EQ(ret, m_refMap.size());
    return ret;
  }

  template <typename K, typename V>
  bool setValueAtKey(K &&key, V &&value)
  {
    // Do the reference map update first, using just lvalue references.
    // I don't need to exercise the reference map's handling of other
    // types since it's not the system under test.
    bool inserted = m_refMap.setValueAtKey(key, value);

    EXPECT_EQ(
      m_flatMap.setValueAtKey(
        std::forward<K>(key), std::forward<V>(value)),
      inserted);

    return inserted;
  }

  template <typename K, typename V>
  void setValueAtNewKey(K &&key, V &&value)
  {
    m_refMap.setValueAtNewKey(key, value);

    m_flatMap.setValueAtNewKey(
      std::forward<K>(key), std::forward<V>(value));
  }

  // This class does not expose `valueRefAtKey` because that wouldn't
  // work due to the interface.  Instead, the test harness has to call
  // that directly on the flat map.

  void clear()
  {
    m_flatMap.clear();
    m_refMap.clear();
  }

  bool contains(KEY const &key) const
  {
    bool ret = m_flatMap.contains(key);
    EXPECT_EQ(ret, m_refMap.contains(key));
    return ret;
  }
};


// Like `test_basics()`, but using `OrderedMapPair`.
template <typename ORDERED_MAP_PAIR>
void test_mapPairBasics()
{
  ORDERED_MAP_PAIR m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  std::string one("one");
  std::string two("two");
  std::string three("three");

  m.setValueAtNewKey(1, one);
  m.setValueAtNewKey(3, three);
  m.setValueAtNewKey(2, two);

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

  // Key-based lookup uses the intrinsic order.
  EXPECT_EQ(m.valueAtKey(1), one);
  EXPECT_EQ(m.valueAtKey(2), two);
  EXPECT_EQ(m.valueAtKey(3), three);

  // Index-based lookup uses the extrinsic order.
  EXPECT_EQ(m.keyAtIndex(0), 1);
  EXPECT_EQ(m.valueAtIndex(0), one);

  EXPECT_EQ(m.keyAtIndex(1), 3);
  EXPECT_EQ(m.valueAtIndex(1), three);

  EXPECT_EQ(m.keyAtIndex(2), 2);
  EXPECT_EQ(m.valueAtIndex(2), two);

  EXPECT_EQ(m.contains(0), false);
  EXPECT_EQ(m.contains(1), true);
  EXPECT_EQ(m.contains(2), true);
  EXPECT_EQ(m.contains(3), true);
  EXPECT_EQ(m.contains(4), false);
  m.selfCheck();

  // Copy ctor.
  {
    ORDERED_MAP_PAIR m2(m);
    EXPECT_EQ(m2.size(), 3);
    EXPECT_EQ(m2.valueAtKey(2), two);
    m2.selfCheck();
  }

  // Invalid access.
  EXPECT_EXN(m.valueAtKey(4), XAssert);
  EXPECT_EXN(m.keyAtIndex(3), XAssert);
  EXPECT_EXN(m.valueAtIndex(3), XAssert);

  // Check default-constructed value.
  {
    std::string &valueRef = m.m_flatMap.valueRefAtKey(4);
    EXPECT_EQ(valueRef, "");

    // The reference map does not have `valueRefAtKey`, so simulate the
    // relevant part of its effect using a different method.
    m.m_refMap.setValueAtNewKey(4, "");

    EXPECT_EQ(m.valueAtKey(4), "");

    // And ability to update via the reference.
    std::string four("four");
    valueRef = four;
    m.m_refMap.setValueAtKey(4, four);
    EXPECT_EQ(m.valueAtKey(4), four);
  }

  // Move ctor.
  ORDERED_MAP_PAIR m2(std::move(m));
  EXPECT_EQ(m2.size(), 4);
  EXPECT_EQ(m2.valueAtKey(2), two);
  m2.selfCheck();

  // The invariants of `m` should hold, even if its state is otherwise
  // indeterminate.
  m.selfCheck();
}


EnvRandomizedTestIters const OUTER_ITERS
  {20, "OFMT_RANDOM_OUTER_ITERS", 2};
EnvRandomizedTestIters const INNER_ITERS
  {100, "OFMT_RANDOM_INNER_ITERS", 2};


// Keys are drawn from [0, KEY_RANGE-1].  The range is kept small so
// that keys collide often, exercising both the "new key" and "existing
// key" paths.
int const KEY_RANGE = 30;


int randomKey()
{
  return sm_random(KEY_RANGE);
}


// Return a random value.  It is never empty, so it is distinguishable
// from the default-constructed value that `valueRefAtKey` inserts.
std::string randomValue()
{
  return "v" + std::to_string(sm_random(100));
}


// Apply a random sequence of operations to an `OrderedMapPair`, which
// checks that the flat map agrees with the reference map after each.
template <typename ORDERED_MAP_PAIR>
void test_randomOps()
{
  TEST_FUNC();

  using MP = ORDERED_MAP_PAIR;
  using size_type = typename MP::size_type;
  using FLAT_MAP = typename MP::FLAT_MAP;

  smbase_loopi(OUTER_ITERS) {
    EXN_CONTEXT_EXPR(i);

    MP m;

    smbase_loopj(INNER_ITERS) {
      EXN_CONTEXT_EXPR(j);

      RandomChoice choice(100);

      if (choice.check(1)) {
        m.clear();
      }

      else if (choice.check(2)) {
        // Copy, then move, and check that the results agree with the
        // original.  (`selfCheck` compares flat and reference maps.)
        MP copy(m);
        EXPECT_EQ(copy.size(), m.size());

        MP moved(std::move(copy));
        EXPECT_EQ(moved.size(), m.size());
        for (size_type index=0; index < m.size(); ++index) {
          EXPECT_EQ(moved.keyAtIndex(index), m.keyAtIndex(index));
          EXPECT_EQ(moved.valueAtIndex(index), m.valueAtIndex(index));
        }

        // The move ctor clears its source.
        EXPECT_EQ(copy.empty(), true);
      }

      else if (choice.check(20)) {
        // Lvalue `setValueAtKey`.
        int key = randomKey();
        std::string value = randomValue();
        bool wasPresent = m.contains(key);
        bool inserted = m.setValueAtKey(key, value);
        EXPECT_EQ(inserted, !wasPresent);
        EXPECT_EQ(m.valueAtKey(key), value);
      }

      else if (choice.check(15)) {
        // Rvalue `setValueAtKey`.
        int key = randomKey();
        std::string value = randomValue();
        bool wasPresent = m.contains(key);
        // This exercises lvalue `key` reference.
        bool inserted = m.setValueAtKey(key, std::string(value));
        EXPECT_EQ(inserted, !wasPresent);
        EXPECT_EQ(m.valueAtKey(key), value);
      }

      else if (choice.check(15)) {
        // `setValueAtNewKey`, lvalue or rvalue, using a key that is not
        // already mapped if there is one.
        int key = randomKey();
        if (!m.contains(key)) {
          std::string value = randomValue();
          if (sm_random(2)) {
            m.setValueAtNewKey(key, value);
          }
          else {
            // This exercises rvalue `key` reference.
            m.setValueAtNewKey(int(key), std::string(value));
          }
          EXPECT_EQ(m.valueAtKey(key), value);
        }

        else {
          // Make a copy of the map first, since it will be altered by
          // the offending insertion.
          FLAT_MAP m2(m.m_flatMap);

          // Confirm that `setValueAtNewKey` will complain if I insert
          // an existing key.
          EXPECT_EXN(
            m2.setValueAtNewKey(key, std::string("x")),
            XAssert);
        }
      }

      else if (choice.check(20)) {
        // `valueRefAtKey`, which is only on the flat map.  As in
        // `test_mapPairBasics`, simulate its effect on the reference
        // map using other methods.
        int key = randomKey();
        bool wasPresent = m.contains(key);

        std::string &valueRef = m.m_flatMap.valueRefAtKey(key);
        if (!wasPresent) {
          // It was just created with a default-constructed value.
          EXPECT_EQ(valueRef, "");
          m.m_refMap.setValueAtNewKey(key, "");
        }
        EXPECT_EQ(m.valueAtKey(key), valueRef);

        // Usually, update the value through the reference.
        if (sm_random(4) != 0) {
          std::string value = randomValue();
          valueRef = value;
          m.m_refMap.setValueAtKey(key, value);
          EXPECT_EQ(m.valueAtKey(key), value);
        }
      }

      // The sum of `choice` calls above is 73, so 27% of the time we
      // end up here.
      else {
        // Queries.  (Each wrapper method compares the two maps.)
        int key = randomKey();
        if (m.contains(key)) {
          m.valueAtKey(key);
        }
        else {
          EXPECT_EXN(m.m_flatMap.valueAtKey(key), XAssert);
        }

        if (!m.empty()) {
          size_type index = sm_random(m.size());
          m.keyAtIndex(index);
          m.valueAtIndex(index);
        }
        EXPECT_EXN(m.m_flatMap.keyAtIndex(m.size()), XAssert);
      }

      m.selfCheck();
    }
  }
}


// Use std::string as a key.
template <typename ORDERED_FLAT_MAP>
void test_stringKey()
{
  ORDERED_FLAT_MAP m;

  // This fails (as desired) due to the `enable_if_t`:
  //m.setValueAtNewKey("one", 1);
  // So we do this instead:
  m.setValueAtNewKey(std::string("one"), 1);

  EXPECT_EQ(m.valueAtKey("one"), 1);

  m.selfCheck();
}


// Do repeated lookups of the same key to test performance of MRU.
template <typename ORDERED_FLAT_MAP>
void test_performanceLoop()
{
  int const numKeys = envAsIntOr(10, "OFMT_PERF_KEYS");
  int const numIters = envAsIntOr(1000000, "OFMT_PERF_ITERS");

  ORDERED_FLAT_MAP m;

  smbase_loopi(numKeys) {
    m.setValueAtNewKey(i, 0);
  }

  fastTimeInitialize();
  unsigned start = fastTimeMilliseconds;

  // Repeatedly look up 5.
  smbase_loopi(numIters) {
    // Increment the value returned so the optimizer does not remove
    // this entire loop.
    ++(m.valueRefAtKey(5));
  }

  unsigned elapsedMS = fastTimeMilliseconds - start;
  VPVAL(elapsedMS);

  // Check its value afterward, again to prevent optimizing away.
  EXPECT_EQ(m.valueRefAtKey(5), numIters);
}


/*
  Measurements (-O2, x86_64, AMD Ryzen 9 7900X):

    OFMT_PERF_KEYS OFMT_PERF_ITERS FlatMap(ms) MRUFlatMap(ms)
    -------------- --------------- ----------- --------------
                 1       100000000         172             78
                10       100000000         219             78
               100       100000000         375             78

  So for a probably-typical map size of 10 elements, the MRU lookup is
  close to 3x faster, and (as expected) its run time stays constant as
  the size of the map increases.

  It's interesting that the lookup is still more than twice as fast when
  there is only one child.
*/
void test_performance()
{
  // Enable verbose output just for the performance tests.
  SAVE_RESTORE(verbose);
  if (envAsBool("OFMT_PERF")) {
    verbose = 1;
  }

  {
    TEST_CASE("OrderedFlatMap performance");
    test_performanceLoop<OrderedFlatMap<int, int>>();
  }

  {
    TEST_CASE("OrderedMRUFlatMap performance");
    test_performanceLoop<OrderedMRUFlatMap<int, int>>();
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_flat_map()
{
  using FlatMap = OrderedFlatMap<int, std::string>;
  using MRUFlatMap = OrderedMRUFlatMap<int, std::string>;

  test_basics<FlatMap>();
  test_basics<MRUFlatMap>();

  test_differentCompare<OrderedFlatMap<int, std::string, CompareMod10>>();
  test_differentCompare<OrderedMRUFlatMap<int, std::string, CompareMod10>>();

  using MapPair = OrderedMapPair<int, std::string, FlatMap>;
  using MRUMapPair = OrderedMapPair<int, std::string, MRUFlatMap>;

  test_mapPairBasics<MapPair>();
  test_mapPairBasics<MRUMapPair>();

  test_randomOps<MapPair>();
  test_randomOps<MRUMapPair>();

  test_stringKey<OrderedFlatMap<std::string, int>>();
  test_stringKey<OrderedMRUFlatMap<std::string, int>>();

  test_performance();
}


// EOF
