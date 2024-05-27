// unit-tests.cc
// Unit test driver program.

// The general structure is that a module called $MOD has its tests in a
// file called $MOD-test.cc, which defines a function called
// test_$MOD(), which is declared and called below by the 'RUN_TEST'
// macro.

#include "nonport.h"                   // getMilliseconds
#include "sm-test.h"                   // ARGS_TEST_MAIN
#include "str.h"                       // streq

#include <cstdlib>                     // std::getenv
#include <iostream>                    // std::{cout, cerr}

#include <stdio.h>                     // fflush, stdout, stderr


extern "C" {
  void test_cycles();                  // cycles-test.c
  void test_d2vector();                // d2vector-test.c
  void test_gprintf();                 // gprintf-test.c
  void test_mypopen();                 // mypopen-test.c
}


static void printTiming(char const *testName, long elapsed)
{
   std::cout <<
     "TIMING: " <<
     std::setw(4) << elapsed << " ms  " <<
     testName <<
     "\n";
}


static void entry(int argc, char **argv)
{
  char const *testName = NULL;
  if (argc >= 2) {
    testName = argv[1];
  }

  bool ranOne = false;

  bool enableTimes = !!std::getenv("UNIT_TESTS_TIMES");

  // Run the test if it is enabled without declaring the test function.
  #define RUN_TEST_NO_DECL(name)                        \
    if (testName == NULL || streq(testName, #name)) {   \
      std::cout << "---- " #name " ----" << std::endl;  \
      long start = getMilliseconds();                   \
      test_##name();                                    \
      long stop = getMilliseconds();                    \
      if (enableTimes) {                                \
        printTiming(#name, stop-start);                 \
      }                                                 \
      /* Flush all output streams so that the output */ \
      /* from different tests cannot get mixed up. */   \
      /* Note that some tests are written in C, so */   \
      /* the C streams require flushing too. */         \
      fflush(stdout);                                   \
      std::cout.flush();                                \
      fflush(stderr);                                   \
      std::cerr.flush();                                \
      ranOne = true;                                    \
    }

  // Run the named test if enabled, also declaring the test function.
  #define RUN_TEST(name)       \
    extern void test_##name(); \
    RUN_TEST_NO_DECL(name)

  RUN_TEST(array);
  RUN_TEST(array2d);
  RUN_TEST(arrayqueue);
  RUN_TEST(astlist);
  RUN_TEST(autofile);
  RUN_TEST(bdffont);
  RUN_TEST(bflatten);
  RUN_TEST(bit2d);
  RUN_TEST(bitarray);
  RUN_TEST(boxprint);
  RUN_TEST(codepoint);
  RUN_TEST(counting_ostream);
  RUN_TEST(crc);
  RUN_TEST_NO_DECL(cycles);
  RUN_TEST_NO_DECL(d2vector);
  RUN_TEST(datablok);
  RUN_TEST(datetime);
  RUN_TEST(dict);
  RUN_TEST(exc);
  RUN_TEST(functional_set);
  RUN_TEST(gcc_options);
  RUN_TEST(gdvalue);
  RUN_TEST_NO_DECL(gprintf);
  RUN_TEST(growbuf);
  RUN_TEST(hashline);
  RUN_TEST(map_utils);
  RUN_TEST_NO_DECL(mypopen);
  RUN_TEST(mysig);
  RUN_TEST(nonport);
  RUN_TEST(objlist);
  RUN_TEST(objpool);
  RUN_TEST(overflow);
  RUN_TEST(owner);
  RUN_TEST(parsestring);
  RUN_TEST(pprint);
  RUN_TEST(refct_serf);
  RUN_TEST(run_process);
  RUN_TEST(sm_ap_int);
  RUN_TEST(sm_ap_uint);
  RUN_TEST(sm_file_util);
  RUN_TEST(sm_pp_util);
  RUN_TEST(sm_rc_ptr);
  RUN_TEST(smregexp);
  RUN_TEST(sobjlist);
  RUN_TEST(srcloc);
  RUN_TEST(str);
  RUN_TEST(strdict);
  RUN_TEST(strhash);
  RUN_TEST(string_utils);
  RUN_TEST(stringf);
  RUN_TEST(stringset);
  RUN_TEST(strutil);
  RUN_TEST(svdict);
  RUN_TEST(syserr);
  RUN_TEST(taillist);
  RUN_TEST(trdelete);
  RUN_TEST(tree_print);
  RUN_TEST(utf8);
  RUN_TEST(vdtllist);
  RUN_TEST(vector_utils);
  RUN_TEST(voidlist);
  RUN_TEST(vptrmap);

  #undef RUN_TEST

  if (!ranOne) {
    xfatal(stringb("unrecogized module name: " << testName));
  }
  else if (testName) {
    cout << "tests for module " << testName << " PASSED\n";
  }
  else {
    cout << "unit tests PASSED" << endl;
  }
}


ARGS_TEST_MAIN

// EOF
