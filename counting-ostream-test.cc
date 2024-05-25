// counting-ostream-test.cc
// Tests for counting-ostream.

#include "counting-ostream.h"          // this module

#include <cassert>                     // assert
#include <sstream>                     // std::ostringstream

using std::cout;
using std::endl;


static bool verbose = false;


// Run a single test.  'writeOperands' is a chain of things to be
// written using 'operator<<'.
#define TEST_WITH(writeOperands)                        \
{                                                       \
  CountingOStream cos;                                  \
  cos << writeOperands;                                 \
                                                        \
  std::ostringstream oss;                               \
  oss << writeOperands;                                 \
                                                        \
  size_t actual = cos.getCount();                       \
  size_t expect = oss.str().size();                     \
                                                        \
  /* Print each test case and flush before checking. */ \
  if (verbose) {                                        \
    cout                                                \
      << "actual=" << actual                            \
      << " expect=" << expect                           \
      << " string: " << oss.str()                       \
      << endl;                                          \
  }                                                     \
  assert(actual == expect);                             \
}


// This is called from unit-test.cc.
void test_counting_ostream()
{
  TEST_WITH("Hello, world!\n");
  TEST_WITH("Look a number: " << std::hex << 29 << std::endl);

  // These work, but printing them to stdout makes tools (grep) think
  // the output is binary data.
  //TEST_WITH('\0');
  //TEST_WITH('\377');

  // Larger test to exercise any buffering, resizing, etc.
  {
    CountingOStream cos;
    std::ostringstream oss;

    for (int i=0; i < 10000; ++i) {
      cos << "string" << 'x' << 123;
      oss << "string" << 'x' << 123;
    }

    size_t actual = cos.getCount();
    size_t expect = oss.str().size();

    if (verbose) {
      cout << "larger: actual=" << actual
           << " expect=" << expect
           << endl;
    }
    assert(actual == expect);
  }
}


// EOF
