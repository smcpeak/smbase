// pp-file-line-test.cc
// Tests for `pp-file-line` module.

#include "smbase/pp-file-line.h"       // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


PreprocFileLine passThrough(PreprocFileLine ppfl)
{
  return ppfl;
}


void test_basics()
{
  PreprocFileLine ppfl = HERE_PREPROC_FILE_LINE;
  EXPECT_EQ(stringb(ppfl), "pp-file-line-test.cc:23");

  // Check that it works to pass the macro as a function argument,
  // since that is how I mainly plan to use it.
  ppfl = passThrough(HERE_PREPROC_FILE_LINE);
  EXPECT_EQ(stringb(ppfl), "pp-file-line-test.cc:28");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_pp_file_line()
{
  test_basics();
}


// EOF
