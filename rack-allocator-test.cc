// rack-allocator-test.cc
// Tests for `rack-allocator` module.

#include "rack-allocator.h"            // module under test

#include "sm-random.h"                 // sm_random
#include "sm-test.h"                   // DIAG, EXPECT_EQ, verbose

#include <cstddef>                     // std::size_t
#include <cstdint>                     // std::uintptr_t

using namespace smbase;


static void printStats(RackAllocator const &ra)
{
  ra.printStats(tout);
}


static void checkAlloc(RackAllocator &ra, std::size_t size)
{
  unsigned char *p = ra.allocate(size);

  // Verify that the pointer is aligned.
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % sizeof(void*), 0);

  ra.selfCheck();
}


static void testFixedSizes()
{
  DIAG("---- testFixedSizes ----");

  RackAllocator ra;
  printStats(ra);

  checkAlloc(ra, 10);
  printStats(ra);

  checkAlloc(ra, 100);
  printStats(ra);

  checkAlloc(ra, 1000);
  printStats(ra);

  checkAlloc(ra, 10000);
  printStats(ra);

  checkAlloc(ra, 100000);
  printStats(ra);

  checkAlloc(ra, 1000000);
  printStats(ra);

  ra.clear();
  printStats(ra);
}


static void testRandomSizes()
{
  DIAG("---- testRandomSizes ----");

  RackAllocator ra;
  smbase_loopi(100) {
    checkAlloc(ra, sm_random(2000));
  }
  printStats(ra);
}


// Called from unit-tests.cc.
void test_rack_allocator()
{
  testFixedSizes();
  testRandomSizes();
}


// EOF
