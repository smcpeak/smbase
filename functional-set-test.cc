// functional-set-test.cc
// Tests for functional-set module.

#include "functional-set.h"            // module under test

#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout


class FSEInteger : public FSElement {
public:      // data
  int const m_i;

public:      // methods
  explicit FSEInteger(int i)
    : m_i(i)
  {}

  char const *fseKind() const override
  {
    return "FSEInteger";
  }

  StrongOrdering compareTo(FSElement const &obj_) const override
  {
    FSELEMENT_COMPARETO_PRELUDE(FSEInteger);

    return strongOrder(m_i, obj.m_i);
  }

  void print(std::ostream &os) const override
  {
    os << m_i;
  }
};


static void testBasics()
{
  FunctionalSetManager fsm;

  RCPtr<FunctionalSet> empty = fsm.emptySet();
  std::cout << "empty: " << *empty << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s1 = fsm.singleton(rcptr(new FSEInteger(1)));
  std::cout << "s1: " << *s1 << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s2 = fsm.singleton(rcptr(new FSEInteger(2)));
  std::cout << "s2: " << *s2 << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s12 = fsm.unionSet(s1, s2);
  std::cout << "s12: " << *s12 << "\n";
  fsm.checkInvariants();

  auto s3 = fsm.singleton(rcptr(new FSEInteger(3)));
  std::cout << "s3: " << *s3 << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s23 = fsm.unionSet(s2, s3);
  std::cout << "s23: " << *s23 << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s123a = fsm.unionSet(s1, s23);
  std::cout << "s123a: " << *s123a << "\n";
  fsm.checkInvariants();

  RCPtr<FunctionalSet> s123b = fsm.unionSet(s3, s12);
  std::cout << "s123b: " << *s123a << "\n";
  fsm.checkInvariants();

  xassert(s123a == s123b);

  xassert(fsm.intersection(s12, s23) == s2);
  xassert(fsm.intersection(s123a, s23) == s23);
  xassert(fsm.intersection(s12, s123b) == s12);
  xassert(fsm.intersection(s12, s3) == empty);

  fsm.checkInvariants();
}


void test_functional_set()
{
  testBasics();
}


// EOF
