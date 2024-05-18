// unit-tests.cc
// Unit test driver program.

// The general structure is that a module called $MOD has its tests in a
// file called $MOD-test.cc, which defines a function called
// test_$MOD(), which is declared and called below by the 'RUN_TEST'
// macro.

#include "sm-test.h"                   // ARGS_TEST_MAIN
#include "str.h"                       // streq


static void entry(int argc, char **argv)
{
  char const *testName = NULL;
  if (argc >= 2) {
    testName = argv[1];
  }

  bool ranOne = false;

  #define RUN_TEST(name)                               \
    if (testName == NULL || streq(testName, #name)) {  \
      std::cout << "---- " #name " ----" << std::endl; \
      extern void test_##name();                       \
      test_##name();                                   \
      ranOne = true;                                   \
    }

  RUN_TEST(array);
  RUN_TEST(astlist);
  RUN_TEST(autofile);
  RUN_TEST(bflatten);
  RUN_TEST(bit2d);
  RUN_TEST(counting_ostream);
  RUN_TEST(datablok);
  RUN_TEST(dict);
  RUN_TEST(functional_set);
  RUN_TEST(gcc_options);
  RUN_TEST(gdvalue);
  RUN_TEST(growbuf);
  RUN_TEST(map_utils);
  RUN_TEST(objlist);
  RUN_TEST(overflow);
  RUN_TEST(parsestring);
  RUN_TEST(sm_pp_util);
  RUN_TEST(sm_rc_ptr);
  RUN_TEST(sobjlist);
  RUN_TEST(str);
  RUN_TEST(strdict);
  RUN_TEST(string_utils);
  RUN_TEST(svdict);
  RUN_TEST(taillist);
  RUN_TEST(vdtllist);
  RUN_TEST(vector_utils);
  RUN_TEST(voidlist);

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
