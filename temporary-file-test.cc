// temporary-file-test.cc
// Tests for `temporary-file` module.

#include "temporary-file.h"            // module under test

#include "sm-file-util.h"              // SMFileUtil
#include "sm-test.h"                   // EXPECT_EQ

using namespace smbase;


// Called from unit-tests.cc.
void test_temporary_file()
{
  SMFileUtil sfu;
  std::string fname;
  {
    char const *contents = "line1\nline2\n";
    TemporaryFile temp("tmp", "txt", contents);

    fname = temp.getFname();
    xassert(sfu.pathExists(fname));

    EXPECT_EQ(sfu.readFileAsString(temp.getFname()), contents);
  }
  xassert(!sfu.pathExists(fname));
}


// EOF
