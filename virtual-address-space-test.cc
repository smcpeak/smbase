// virtual-address-space-test.cc
// Tests for `virtual-address-space` module.

#include "smbase/virtual-address-space.h"        // module under test

#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ, TEST_FUNC_EXPRS

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


using VASID = VirtualASManager::VASID;
using LocalOffset = VirtualASManager::LocalOffset;
using GlobalOffset = VirtualASManager::GlobalOffset;
using IdOffset = std::pair<VASID, LocalOffset>;


// Check that `globalOffset` maps to/from `(vas, localOffst)` in `mgr`.
void checkGlobalLocal(
  VirtualASManager const &mgr,
  GlobalOffset globalOffset,
  VASID vas,
  LocalOffset localOffset)
{
  TEST_FUNC_EXPRS(globalOffset, vas, localOffset);

  EXPECT_EQ_GDVSER(mgr.globalToLocal(globalOffset),
                   IdOffset(vas, localOffset));
  EXPECT_EQ(mgr.localToGlobal(vas, localOffset), globalOffset);
}


// This builds the address space described in the comments above the
// declaration of `VirtualASManager`.
void test_basics()
{
  // Start empty.
  VirtualASManager mgr;

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 0);
  EXPECT_EQ(mgr.numLocalSpaces(), 0);
  EXPECT_FALSE(mgr.validLocalSpace(0));
  EXPECT_FALSE(mgr.validLocalSpace(1));
  EXPECT_FALSE(mgr.validLocalSpace(-1));

  // Allocate A.
  VASID vasA = mgr.allocateLocalSpace();

  mgr.selfCheck();
  EXPECT_EQ(vasA, 0);
  EXPECT_EQ(mgr.globalSpaceSize(), 0);
  EXPECT_EQ(mgr.numLocalSpaces(), 1);
  EXPECT_TRUE(mgr.validLocalSpace(vasA));
  EXPECT_FALSE(mgr.validLocalSpace(1));
  EXPECT_EQ(mgr.localSpaceSize(vasA), 0);

  // Allocate space in A.
  mgr.extendLocalSpace(vasA, 10);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 10);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 10);
  checkGlobalLocal(mgr, 0, vasA, 0);
  checkGlobalLocal(mgr, 5, vasA, 5);
  checkGlobalLocal(mgr, 9, vasA, 9);

  // Allocate B.
  VASID vasB = mgr.allocateLocalSpace();

  mgr.selfCheck();
  EXPECT_EQ(vasB, 1);
  EXPECT_EQ(mgr.globalSpaceSize(), 10);
  EXPECT_EQ(mgr.numLocalSpaces(), 2);
  EXPECT_TRUE(mgr.validLocalSpace(vasA));
  EXPECT_TRUE(mgr.validLocalSpace(vasB));
  EXPECT_EQ(mgr.localSpaceSize(vasA), 10);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 0);

  // Allocate space in B.
  mgr.extendLocalSpace(vasB, 50);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 60);
  EXPECT_EQ(mgr.numLocalSpaces(), 2);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 10);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  checkGlobalLocal(mgr,  0, vasA,  0);
  checkGlobalLocal(mgr,  9, vasA,  9);
  checkGlobalLocal(mgr, 10, vasB,  0);
  checkGlobalLocal(mgr, 59, vasB, 49);

  // Allocate more space in A.
  mgr.extendLocalSpace(vasA, 70);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 130);
  EXPECT_EQ(mgr.numLocalSpaces(), 2);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  checkGlobalLocal(mgr,   0, vasA,  0);
  checkGlobalLocal(mgr,   9, vasA,  9);
  checkGlobalLocal(mgr,  10, vasB,  0);
  checkGlobalLocal(mgr,  59, vasB, 49);
  checkGlobalLocal(mgr,  60, vasA, 10);
  checkGlobalLocal(mgr, 129, vasA, 79);

  // Allocate C.
  VASID vasC = mgr.allocateLocalSpace();

  mgr.selfCheck();
  EXPECT_EQ(vasC, 2);
  EXPECT_EQ(mgr.globalSpaceSize(), 130);
  EXPECT_EQ(mgr.numLocalSpaces(), 3);
  EXPECT_TRUE(mgr.validLocalSpace(vasA));
  EXPECT_TRUE(mgr.validLocalSpace(vasB));
  EXPECT_TRUE(mgr.validLocalSpace(vasC));
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  EXPECT_EQ(mgr.localSpaceSize(vasC), 0);

  // Allocate space in C; start with just half.
  mgr.extendLocalSpace(vasC, 500);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 630);
  EXPECT_EQ(mgr.numLocalSpaces(), 3);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  EXPECT_EQ(mgr.localSpaceSize(vasC), 500);
  checkGlobalLocal(mgr,   0, vasA,   0);
  checkGlobalLocal(mgr,   9, vasA,   9);
  checkGlobalLocal(mgr,  10, vasB,   0);
  checkGlobalLocal(mgr,  59, vasB,  49);
  checkGlobalLocal(mgr,  60, vasA,  10);
  checkGlobalLocal(mgr, 129, vasA,  79);
  checkGlobalLocal(mgr, 130, vasC,   0);
  checkGlobalLocal(mgr, 629, vasC, 499);

  // Extend C again to get the rest.
  mgr.extendLocalSpace(vasC, 500);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 1130);
  EXPECT_EQ(mgr.numLocalSpaces(), 3);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  EXPECT_EQ(mgr.localSpaceSize(vasC), 1000);
  checkGlobalLocal(mgr,    0, vasA,   0);
  checkGlobalLocal(mgr,    9, vasA,   9);
  checkGlobalLocal(mgr,   10, vasB,   0);
  checkGlobalLocal(mgr,   59, vasB,  49);
  checkGlobalLocal(mgr,   60, vasA,  10);
  checkGlobalLocal(mgr,  129, vasA,  79);
  checkGlobalLocal(mgr,  130, vasC,   0);
  checkGlobalLocal(mgr,  629, vasC, 499);
  checkGlobalLocal(mgr,  630, vasC, 500);
  checkGlobalLocal(mgr, 1129, vasC, 999);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_virtual_address_space()
{
  test_basics();
}


// EOF
