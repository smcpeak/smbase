// ordered-flat-map-test.cc
// Tests for `ordered-flat-map` module.

// This file is in the public domain.

#include "smbase/ordered-flat-map.h"   // module under test

#include "smbase/ordered-map.h"        // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, DMEMB, MDMEMB
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN_SUBSTR

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


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ordered_flat_map()
{
  test_basics();
  test_differentCompare();
  test_mapPairBasics();
}


// EOF
