// datablok-test.cc
// Unit tests for datablok module.

#include "datablok.h"                  // module under test

#include "nonport.h"                   // removeFile
#include "save-restore.h"              // SET_RESTORE
#include "sm-test.h"                   // verbose
#include "xassert.h"                   // xfailure

#include <stdio.h>                     // printf


static bool detectedCorruption = false;

static void corruptionHandler()
{
  detectedCorruption = true;
}

static void testMemoryCorruption()
{
  SET_RESTORE(DataBlock::s_memoryCorruptionOverrideHandler,
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
        b.getData()[i] = (unsigned char)i;
      }
      b.setDataLen(260);
      if (verbose) {
        b.print("all bytes plus 4 extra");
      }
    }

    DataBlock block("yadda smacker");
    xassert(block.getDataLen() == 14);

    // Full: Includes NUL.
    xassert(block.toFullString() == string("yadda smacker", 14));

    // Null-term: Does not.
    xassert(block.toNTString() == string("yadda smacker"));

    // Legacy compatibility: Does not.
    xassert(block.toString() == string("yadda smacker"));

    DataBlock block2((unsigned char*)"yadda smacker", 13, 14);
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

    // This particular test is annoying because it prints an alarming
    // message that is easily misinterpreted.  If I'm not actively
    // developing DataBlock then I don't need this on.
    if (false) {
      testMemoryCorruption();
    }
  }
}


// EOF
