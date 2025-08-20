// ast-switch-test.cc
// Tests for `ast-switch` module.

#include "smbase/ast-switch.h"         // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EXN_SUBSTR
#include "smbase/xassert.h"            // xassert, xassertPtr

#include <memory>                      // std::unique_ptr

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


class Sub1;
class Sub2;
class Sub3;


class Super {
public:
  enum Kind { K_SUB1, K_SUB2, K_SUB3 };
  virtual Kind kind() const = 0;

  DECL_AST_DOWNCASTS(Sub1, K_SUB1)
  DECL_AST_DOWNCASTS(Sub2, K_SUB2)
  DECL_AST_DOWNCASTS(Sub3, K_SUB3)
};

DEFN_AST_DOWNCASTS(Super, Sub1, K_SUB1)
DEFN_AST_DOWNCASTS(Super, Sub2, K_SUB2)
DEFN_AST_DOWNCASTS(Super, Sub3, K_SUB3)


class Sub1 : public Super {
public:
  static Kind constexpr TYPE_TAG = K_SUB1;
  virtual Kind kind() const override { return TYPE_TAG; }

  int m_x;

  Sub1(int x)
    : m_x(x)
  {}

  // Get `m_x`, but with a non-const method.
  int xnc() { return m_x; }
};


class Sub2 : public Super {
public:
  static Kind constexpr TYPE_TAG = K_SUB2;
  virtual Kind kind() const override { return TYPE_TAG; }

  int m_y;
  int m_z;

  Sub2(int y, int z)
    : m_y(y),
      m_z(z)
  {}

  int ync() { return m_y; }
  int znc() { return m_z; }
};


class Sub3 : public Super {
public:
  static Kind constexpr TYPE_TAG = K_SUB3;
  virtual Kind kind() const override { return TYPE_TAG; }

  float m_q;

  Sub3(float q)
    : m_q(q)
  {}

  float qnc() { return m_q; }
};


int exhaustiveC(Super const *super)
{
  ASTSWITCHC(Super, super) {
    ASTCASEC(Sub1, s1) {
      return s1->m_x;
    }

    ASTNEXTC(Sub2, s2) {
      return s2->m_y + s2->m_z;
    }

    ASTNEXTC(Sub3, s3) {
      return int(s3->m_q);
    }

    ASTENDCASEC
  }

  // Not reached.
  return 0;
}


int nonExhaustiveC(Super const *super)
{
  ASTSWITCHC(Super, super) {
    ASTCASEC(Sub1, s1) {
      return s1->m_x;
    }

    ASTNEXTC(Sub2, s2) {
      return s2->m_y + s2->m_z;
    }

    ASTENDCASECD
  }

  // Used for Sub3.
  return 33;
}


int withDefaultC(Super const *super)
{
  ASTSWITCHC(Super, super) {
    ASTCASEC(Sub1, s1) {
      return s1->m_x;
    }

    ASTNEXTC(Sub2, s2) {
      return s2->m_y + s2->m_z;
    }

    ASTDEFAULTC {
      return 44;
    }

    ASTENDCASEC
  }

  // Not reached.
  return 0;
}


int oneArgC(Super const *super)
{
  ASTSWITCHC(Super, super) {
    ASTCASEC1(Sub1) {
      return 111;
    }

    ASTNEXTC1(Sub2) {
      return 222;
    }

    ASTNEXTC1(Sub3) {
      return 333;
    }

    ASTENDCASEC
  }

  // Not reached.
  return 0;
}


int exhaustiveNC(Super *super)
{
  ASTSWITCH(Super, super) {
    ASTCASE(Sub1, s1) {
      return s1->xnc();
    }

    ASTNEXT(Sub2, s2) {
      return s2->ync() + s2->znc();
    }

    ASTNEXT(Sub3, s3) {
      return int(s3->qnc());
    }

    ASTENDCASE
  }

  // Not reached.
  return 0;
}


int nonExhaustiveNC(Super *super)
{
  ASTSWITCH(Super, super) {
    ASTCASE(Sub1, s1) {
      return s1->xnc();
    }

    ASTNEXT(Sub2, s2) {
      return s2->ync() + s2->znc();
    }

    ASTENDCASED
  }

  // Used for Sub3.
  return 33;
}


int withDefaultNC(Super *super)
{
  ASTSWITCH(Super, super) {
    ASTCASE(Sub1, s1) {
      return s1->xnc();
    }

    ASTNEXT(Sub2, s2) {
      return s2->ync() + s2->znc();
    }

    ASTDEFAULT {
      return 44;
    }

    ASTENDCASE
  }

  // Not reached.
  return 0;
}


int oneArgNC(Super *super)
{
  ASTSWITCH(Super, super) {
    ASTCASE1(Sub1) {
      return 111;
    }

    ASTNEXT1(Sub2) {
      return 222;
    }

    ASTNEXT1(Sub3) {
      return 333;
    }

    ASTENDCASE
  }

  // Not reached.
  return 0;
}


void test_basics()
{
  std::unique_ptr<Super> p1(new Sub1(1));
  std::unique_ptr<Super> p2(new Sub2(2, 3));
  std::unique_ptr<Super> p3(new Sub3(4.0f));

  Super const *cp1 = p1.get();
  Super const *cp2 = p2.get();
  Super const *cp3 = p3.get();

  EXPECT_EQ(exhaustiveC(cp1), 1);
  EXPECT_EQ(exhaustiveC(cp2), 5);
  EXPECT_EQ(exhaustiveC(cp3), 4);

  EXPECT_EQ(nonExhaustiveC(cp1), 1);
  EXPECT_EQ(nonExhaustiveC(cp2), 5);
  EXPECT_EQ(nonExhaustiveC(cp3), 33);

  EXPECT_EQ(withDefaultC(cp1), 1);
  EXPECT_EQ(withDefaultC(cp2), 5);
  EXPECT_EQ(withDefaultC(cp3), 44);

  EXPECT_EQ(oneArgC(cp1), 111);
  EXPECT_EQ(oneArgC(cp2), 222);
  EXPECT_EQ(oneArgC(cp3), 333);

  xassert(xassertPtr(cp1->ifSub1C())->m_x == 1);
  xassert(cp1->asSub1C()->m_x == 1);
  xassert(cp1->isSub1());

  xassert(cp1->ifSub2C() == nullptr);
  EXPECT_EXN_SUBSTR(cp1->asSub2C(),
    XAssert, "kind() == K_SUB2");
  xassert(!cp1->isSub2());

  xassert(cp2->ifSub1C() == nullptr);
  EXPECT_EXN_SUBSTR(cp2->asSub1C(),
    XAssert, "kind() == K_SUB1");
  xassert(!cp2->isSub1());

  xassert(xassertPtr(cp2->ifSub2C())->m_y == 2);
  xassert(cp2->asSub2C()->m_y == 2);
  xassert(cp2->isSub2());

  Super *ncp1 = p1.get();
  Super *ncp2 = p2.get();
  Super *ncp3 = p3.get();

  EXPECT_EQ(exhaustiveNC(ncp1), 1);
  EXPECT_EQ(exhaustiveNC(ncp2), 5);
  EXPECT_EQ(exhaustiveNC(ncp3), 4);

  EXPECT_EQ(nonExhaustiveNC(ncp1), 1);
  EXPECT_EQ(nonExhaustiveNC(ncp2), 5);
  EXPECT_EQ(nonExhaustiveNC(ncp3), 33);

  EXPECT_EQ(withDefaultNC(ncp1), 1);
  EXPECT_EQ(withDefaultNC(ncp2), 5);
  EXPECT_EQ(withDefaultNC(ncp3), 44);

  EXPECT_EQ(oneArgNC(ncp1), 111);
  EXPECT_EQ(oneArgNC(ncp2), 222);
  EXPECT_EQ(oneArgNC(ncp3), 333);
}


bool equal(Super const *pa, Super const *pb)
{
  if (pa->kind() != pb->kind()) {
    return false;
  }

  ASTSWITCH2C(Super, pa, pb) {
    ASTCASE2C(Sub1, s1a, s1b) {
      return s1a->m_x == s1b->m_x;
    }

    ASTNEXT2C(Sub2, s2a, s2b) {
      return s2a->m_y + s2a->m_z ==
             s2b->m_y + s2b->m_z;
    }

    ASTNEXT2C(Sub3, s3a, s3b) {
      return s3a->m_q == s3b->m_q;
    }

    ASTENDCASE2C
  }

  // Not reached.
  return false;
}

void test_parallel()
{
  std::unique_ptr<Super> p1(new Sub1(1));
  std::unique_ptr<Super> p2(new Sub2(2, 3));
  std::unique_ptr<Super> p3(new Sub3(4.0f));

  Super const *cp1 = p1.get();
  Super const *cp2 = p2.get();
  Super const *cp3 = p3.get();

  xassert(equal(cp1, cp1));
  xassert(equal(cp2, cp2));
  xassert(equal(cp3, cp3));

  xassert(!equal(cp1, cp2));
  xassert(!equal(cp2, cp3));

  std::unique_ptr<Super> p1b(new Sub1(11));

  Super const *cp1b = p1b.get();

  xassert(equal(cp1b, cp1b));
  xassert(!equal(cp1, cp1b));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ast_switch()
{
  test_basics();
  test_parallel();
}


// EOF
