// unit-tests.cc
// Unit test driver program.

// The general structure is that a module called $MOD has its tests in a
// file called $MOD-test.cc, which defines a function called
// test_$MOD(), which is declared and called below by the 'RUN_TEST'
// macro.

#include "smbase/exc.h"                // xfatal
#include "smbase/nonport.h"            // getMilliseconds
#include "smbase/sm-test.h"            // g_argv0
#include "smbase/str.h"                // streq
#include "smbase/strutil.h"            // test_strutil

#include <cstdlib>                     // std::getenv
#include <exception>                   // std::exception
#include <iomanip>                     // std::setw
#include <iostream>                    // std::{cout, cerr}
#include <typeinfo>                    // typeid.name

#include <stdio.h>                     // fflush, stdout, stderr

using namespace smbase;


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
  g_argv0 = argv[0];

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
  RUN_TEST(astlist_gdvalue);
  RUN_TEST(autofile);
  RUN_TEST(bdffont);
  RUN_TEST(bflatten);
  RUN_TEST(bit2d);
  RUN_TEST(bitarray);
  RUN_TEST(boxprint);
  RUN_TEST(c_string_reader);
  RUN_TEST(codepoint);
  RUN_TEST(counting_ostream);
  RUN_TEST(crc);
  RUN_TEST_NO_DECL(cycles);
  RUN_TEST_NO_DECL(d2vector);
  RUN_TEST(datablok);
  RUN_TEST(datetime);
  RUN_TEST(dict);
  RUN_TEST(distinct_number);
  RUN_TEST(dni_vector);
  RUN_TEST(exc);
  RUN_TEST(functional_set);
  RUN_TEST(gcc_options);
  RUN_TEST(gdvalue);
  RUN_TEST(gdvalue_json);
  RUN_TEST(gdvalue_parser);
  RUN_TEST(gdvsymbol);
  RUN_TEST(gdvtuple);
  RUN_TEST(get_type_name);
  RUN_TEST_NO_DECL(gprintf);
  RUN_TEST(growbuf);
  RUN_TEST(hashline);
  RUN_TEST(indexed_string_table);
  RUN_TEST(map_util);
  RUN_TEST_NO_DECL(mypopen);
  RUN_TEST(mysig);
  RUN_TEST(nonport);
  RUN_TEST(objlist);
  RUN_TEST(objpool);
  RUN_TEST(optional_util);
  RUN_TEST(ordered_map);
  RUN_TEST(overflow);
  RUN_TEST(owner);
  RUN_TEST(parsestring);
  RUN_TEST(pprint);
  RUN_TEST(rack_allocator);
  RUN_TEST(reader);
  RUN_TEST(refct_serf);
  RUN_TEST(run_process);
  RUN_TEST(save_restore);
  RUN_TEST(set_util);
  RUN_TEST(sm_ap_int);
  RUN_TEST(sm_ap_uint);
  RUN_TEST(sm_env);
  RUN_TEST(sm_file_util);
  RUN_TEST(sm_integer);
  RUN_TEST(sm_is_equal);
  RUN_TEST(sm_pp_util);
  RUN_TEST(sm_rc_ptr);
  RUN_TEST(sm_regex);
  RUN_TEST(sm_stristr);
  RUN_TEST(sm_trace);
  RUN_TEST(sm_unique_ptr);
  RUN_TEST(sobjlist);
  RUN_TEST(srcloc);
  RUN_TEST(std_list_fwd);
  RUN_TEST(std_map_fwd);
  RUN_TEST(std_optional_fwd);
  RUN_TEST(std_set_fwd);
  RUN_TEST(std_string_fwd);
  RUN_TEST(std_string_view_fwd);
  RUN_TEST(std_variant_fwd);
  RUN_TEST(std_vector_fwd);
  RUN_TEST(str);
  RUN_TEST(strdict);
  RUN_TEST(strhash);
  RUN_TEST(string_hash);
  RUN_TEST(string_util);
  RUN_TEST(stringf);
  RUN_TEST(stringset);
  RUN_TEST(strutil);
  RUN_TEST(svdict);
  RUN_TEST(syserr);
  RUN_TEST(taillist);
  RUN_TEST(temporary_file);
  RUN_TEST(trdelete);
  RUN_TEST(tree_print);
  RUN_TEST(type_name_and_size);
  RUN_TEST(utf8);
  RUN_TEST(vdtllist);
  RUN_TEST(vector_push_pop);
  RUN_TEST(vector_util);
  RUN_TEST(voidlist);
  RUN_TEST(vptrmap);
  RUN_TEST(xassert);

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


int main(int argc, char *argv[])
{
  g_abortUponDevWarning = true;
  try {
    entry(argc, argv);
    return 0;
  }
  catch (XBase &x) {
    cerr << x.what() << endl;
    return 2;
  }
  catch (std::exception &x) {
    // Some of the std exceptions are not very self-explanatory without
    // also seeing the exception type.  This is ugly because `name()`
    // returns a mangled name, so I'd like to avoid ever allowing such
    // exceptions to propagate.
    cerr << typeid(x).name() << ": " << x.what() << endl;
    return 2;
  }
}


// EOF
