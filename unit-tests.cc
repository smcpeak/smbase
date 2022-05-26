// unit-tests.cc
// Unit test driver program.

// Most modules in smbase are tested in their own separate test
// executables, but I would like to start consolidating them into a
// single unit-test program.  Since the 'overflow' module, imported from
// the 'chess' repo, already works that way, it will be the first module
// to be tested in the combined unit test program.

#include "datablok.h"                  // test_datablok
#include "overflow.h"                  // test_overflow
#include "parsestring.h"               // test_parsestring
#include "sm-test.h"                   // ARGS_TEST_MAIN
#include "str.h"                       // streq

void test_dict();                      // test-dict.cc
void test_functional_set();            // functional-set-test.cc
void test_gcc_options();               // gcc-options-test.cc
void test_sm_rc_ptr();                 // sm-rc-ptr-test.cc
void test_string_utils();              // string-utils-test.cc
void test_vector_utils();              // test-vector-utils.cc


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
      test_##name();                                   \
      ranOne = true;                                   \
    }

  RUN_TEST(datablok);
  RUN_TEST(dict);
  RUN_TEST(functional_set);
  RUN_TEST(gcc_options);
  RUN_TEST(overflow);
  RUN_TEST(parsestring);
  RUN_TEST(sm_rc_ptr);
  RUN_TEST(string_utils);
  RUN_TEST(vector_utils);

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
