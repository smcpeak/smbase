// virtual-address-space-test.cc
// Tests for `virtual-address-space` module.

#include "smbase/virtual-address-space.h"        // module under test

#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// This builds the address space described in the comments above the
// declaration of `VirtualASManager`.
void test_basics()
{
  using VASID = VirtualASManager::VASID;
  using LocalOffset = VirtualASManager::LocalOffset;
  using IdOffset = std::pair<VASID, LocalOffset>;

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
  EXPECT_EQ_GDVSER(mgr.globalToLocal(0), IdOffset(vasA,0));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(5), IdOffset(vasA,5));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(9), IdOffset(vasA,9));

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
  EXPECT_EQ_GDVSER(mgr.globalToLocal(9), IdOffset(vasA,9));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(10), IdOffset(vasB,0));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(59), IdOffset(vasB,49));

  // Allocate more space in A.
  mgr.extendLocalSpace(vasA, 70);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 130);
  EXPECT_EQ(mgr.numLocalSpaces(), 2);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  EXPECT_EQ_GDVSER(mgr.globalToLocal(9), IdOffset(vasA,9));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(10), IdOffset(vasB,0));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(59), IdOffset(vasB,49));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(60), IdOffset(vasA,10));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(129), IdOffset(vasA,79));

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
  EXPECT_EQ_GDVSER(mgr.globalToLocal(59), IdOffset(vasB,49));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(60), IdOffset(vasA,10));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(129), IdOffset(vasA,79));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(130), IdOffset(vasC,0));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(629), IdOffset(vasC,499));

  // Extend C again to get the rest.
  mgr.extendLocalSpace(vasC, 500);

  mgr.selfCheck();
  EXPECT_EQ(mgr.globalSpaceSize(), 1130);
  EXPECT_EQ(mgr.numLocalSpaces(), 3);
  EXPECT_EQ(mgr.localSpaceSize(vasA), 80);
  EXPECT_EQ(mgr.localSpaceSize(vasB), 50);
  EXPECT_EQ(mgr.localSpaceSize(vasC), 1000);
  EXPECT_EQ_GDVSER(mgr.globalToLocal(59), IdOffset(vasB,49));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(60), IdOffset(vasA,10));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(129), IdOffset(vasA,79));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(130), IdOffset(vasC,0));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(629), IdOffset(vasC,499));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(630), IdOffset(vasC,500));
  EXPECT_EQ_GDVSER(mgr.globalToLocal(1129), IdOffset(vasC,999));
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_virtual_address_space()
{
  test_basics();
}


// EOF
