// ordered-flat-map-test.cc
// Tests for `ordered-flat-map` module.

// This file is in the public domain.

#include "smbase/ordered-flat-map.h"   // module under test

#include "smbase/exc.h"                // EXN_CONTEXT_EXPR
#include "smbase/ordered-map.h"        // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, DMEMB, MDMEMB, smbase_loopi
#include "smbase/sm-random.h"          // smbase::{RandomChoice, sm_random}
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN_SUBSTR, EnvRandomizedTestIters, TEST_FUNC

#include <string>                      // std::string
#include <utility>                     // std::move

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  OrderedFlatMap<int, std::string> m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.setValueAtNewKey(1, "one");
  m.setValueAtNewKey(3, "three");
  m.setValueAtNewKey(2, "two");

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

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
    OrderedFlatMap<int, std::string> m2(m);
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
  }

  // Move ctor.
  OrderedFlatMap<int, std::string> m2(std::move(m));
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


void test_differentCompare()
{
  OrderedFlatMap<int, std::string, CompareMod10> m;
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


// OrderedFlatMap under test, and OrderedMap for reference.
template <typename KEY, typename VALUE>
class OrderedMapPair {
public:      // types
  using size_type = typename OrderedFlatMap<KEY, VALUE>::size_type;

public:      // data
  // Object under test.
  OrderedFlatMap<KEY, VALUE> m_flatMap;

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

  bool setValueAtKey(KEY const &key, VALUE const &value)
  {
    bool inserted = m_flatMap.setValueAtKey(key, value);
    EXPECT_EQ(inserted, m_refMap.setValueAtKey(key, value));
    return inserted;
  }

  bool setValueAtKey(KEY const &key, VALUE &&value)
  {
    // To exercise the move version of `setValueAtKey`, move a copy of
    // `value`.
    bool inserted = m_flatMap.setValueAtKey(key, VALUE(value));

    // Move the original into the reference map.
    EXPECT_EQ(inserted, m_refMap.setValueAtKey(key, std::move(value)));

    return inserted;
  }

  void setValueAtNewKey(KEY const &key, VALUE const &value)
  {
    m_flatMap.setValueAtNewKey(key, value);
    m_refMap.setValueAtNewKey(key, value);
  }

  void setValueAtNewKey(KEY const &key, VALUE &&value)
  {
    m_flatMap.setValueAtNewKey(key, VALUE(value));
    m_refMap.setValueAtNewKey(key, std::move(value));
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
void test_mapPairBasics()
{
  OrderedMapPair<int, std::string> m;
  m.selfCheck();

  EXPECT_EQ(m.empty(), true);
  EXPECT_EQ(m.size(), 0);

  m.setValueAtNewKey(1, "one");
  m.setValueAtNewKey(3, "three");
  m.setValueAtNewKey(2, "two");

  EXPECT_EQ(m.empty(), false);
  EXPECT_EQ(m.size(), 3);

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
    OrderedMapPair<int, std::string> m2(m);
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
    std::string &valueRef = m.m_flatMap.valueRefAtKey(4);
    EXPECT_EQ(valueRef, "");

    // The reference map does not have `valueRefAtKey`, so simulate the
    // relevant part of its effect using a different method.
    m.m_refMap.setValueAtNewKey(4, "");

    EXPECT_EQ(m.valueAtKey(4), "");

    // And ability to update via the reference.
    valueRef = "four";
    m.m_refMap.setValueAtKey(4, "four");
    EXPECT_EQ(m.valueAtKey(4), "four");
  }

  // Move ctor.
  OrderedMapPair<int, std::string> m2(std::move(m));
  EXPECT_EQ(m2.size(), 4);
  EXPECT_EQ(m2.valueAtKey(2), "two");
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
void test_randomOps()
{
  TEST_FUNC();

  using MP = OrderedMapPair<int, std::string>;

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
        for (MP::size_type index=0; index < m.size(); ++index) {
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
        bool inserted = m.setValueAtKey(int(key), std::string(value));
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
            m.setValueAtNewKey(int(key), std::string(value));
          }
          EXPECT_EQ(m.valueAtKey(key), value);
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
          MP::size_type index = sm_random(m.size());
          m.keyAtIndex(index);
          m.valueAtIndex(index);
        }
        EXPECT_EXN(m.m_flatMap.keyAtIndex(m.size()), XAssert);
      }

      m.selfCheck();
    }
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_flat_map()
{
  test_basics();
  test_differentCompare();
  test_mapPairBasics();
  test_randomOps();
}


// EOF
