// mysig-test.cc
// Tests for mysig.

#include "mysig.h"                     // module under test

#include "sm-test.h"                   // dummy_printf, verbose

#include <setjmp.h>                    // setjmp
#include <stdint.h>                    // uintptr_t
#include <stdio.h>                     // printf
#include <stdlib.h>                    // strtoul, exit
#include <string.h>                    // strcmp


#define printf (verbose? printf : dummy_printf)


static void infiniteRecursion()
{
  char volatile buf[1024];
  buf[0] = 4;
  buf[1] = buf[0];     // silence an icc warning
  buf[1023] = 6;

  // This useless test is here to suppress a Clang infinite loop
  // warning.
  if (buf[0]) {
    infiniteRecursion();
  }
}


static void runTest()
{
  if (char const *segfaultAddr = getenv("MYSIG_SEGFAULT_ADDR")) {
    // segfault at a given addr
    printSegfaultAddrs();

    if (0==strcmp(segfaultAddr, "inf")) {
      // die by stack overflow.. interesting, I can't catch it..
      printf("going into infinite recursion...\n");
      infiniteRecursion();
    }

    uintptr_t addr = strtoul(segfaultAddr, NULL /*endp*/, 0 /*radix*/);
    printf("about to access 0x%lX ...\n", (long)addr);
    *((int volatile*)addr) = 0;
    return;     // won't be reached for most values of 'addr'
  }

  if (setjmp(sane_state) == 0) {   // normal flow
    setHandler(SIGINT, printHandler);
    setHandler(SIGTERM, printHandler);
    setHandler(SIGSEGV, jmpHandler);
    #ifdef __WIN32__
      // Windows does not seem to have SIGUSR1 or SIGBUS definitions.
      // I'm not going to run this code on Windows, but compiling it is
      // good for detecting syntax errors, so I'll just avoid the
      // problematic symbols.
    #else
      setHandler(SIGUSR1, jmpHandler);
      setHandler(SIGBUS, jmpHandler);   // osx gives SIBGUS instead of SIGSEGV
    #endif

    //printf("I'm pid %d waiting to be killed...\n", getpid());
    //sleep(10);
    printf("about to deliberately cause a segfault ...\n");
    printf("(Note: 'gcc -fsanitize=undefined' will report a "
           "\"runtime error\" here too, which can be ignored.)\n");
    *((int volatile*)0) = 0;    // segfault!

    printf("didn't segfault??\n");
    exit(2);
  }

  else {         // from longjmp
    printf("came back from a longjmp!\n");
    printf("\nmysig works\n");
  }
}


// Called from unit-tests.cc.
void test_mysig()
{
  if (mysigModuleWorks()) {
    runTest();
  }
  else {
    printf("mysig does not work on this platform, skipping test\n");
  }
}


// EOF
