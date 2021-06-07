// test-datablok.cc
// Unit tests for datablok module.

#include "datablok.h"                  // module under test

#include "macros.h"                    // Restorer
#include "nonport.h"                   // removeFile
#include "xassert.h"                   // xfailure

#include <stdio.h>                     // printf


static bool detectedCorruption = false;

static void corruptionHandler()
{
  detectedCorruption = true;
}

static void testMemoryCorruption()
{
  Restorer< void (*)() > restorer(DataBlock::s_memoryCorruptionOverrideHandler,
    &corruptionHandler);

  {
    DataBlock b("some test data");
    b.getData()[b.getAllocated()] = 0;   // overrun

    printf("This should cause a corruption detection:\n");
    fflush(stdout);
    // invoke selfcheck in destructor
  }

  if (!detectedCorruption) {
    xfailure("failed to detect overrun");
  }
}

void test_datablok()
{
  // nest everything so the dtors are inside
  {
    // test printing function
    {
      DataBlock b(260);
      for (int i=0; i<260; i++) {
        b.getData()[i] = (byte)i;
      }
      b.setDataLen(260);
      b.print("all bytes plus 4 extra");
    }

    DataBlock block("yadda smacker");
    xassert(block.getDataLen() == 14);

    DataBlock block2((byte*)"yadda smacker", 13, 14);
    block2.addNull();
    xassert(block == block2);

    DataBlock block3;
    block3 = block2;
    xassert(block3 == block);

    block3.setAllocated(5);       // truncates
    block2.setAllocated(25);
    xassert(block3 != block2);

    // test file save/load
    block.writeToFile("tempfile.blk");
    DataBlock block4;
    block4.readFromFile("tempfile.blk");
    xassert(block == block4);
    removeFile("tempfile.blk");

    testMemoryCorruption();
  }

  printf("test_datablok: PASSED\n");
}


// EOF
