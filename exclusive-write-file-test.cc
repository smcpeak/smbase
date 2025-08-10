// exclusive-write-file-test.cc
// Tests for `exclusive-write-file` module.

#include "smbase/exclusive-write-file.h"         // module under test

#include "smbase/nonport.h"                      // getProcessId
#include "smbase/sm-env.h"                       // smbase::envAsBool
#include "smbase/sm-file-util.h"                 // SMFileUtil
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/syserr.h"                       // smbase::XSysError

#include <iostream>                              // std::{cin, cout, endl}

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


char const * const testFileName = "out/ewft.txt";
SMFileUtil sfu;

std::string testFileContents()
{
  return sfu.readFileAsString(testFileName);
}


void test_simple()
{
  sfu.removeFileIfExists(testFileName);

  // Open when does not exist.
  {
    ExclusiveWriteFile ewf(testFileName);
    ewf.stream() << "hello ewf simple\n";
    ewf.selfCheck();
  }
  EXPECT_EQ(testFileContents(), "hello ewf simple\n");

  // Open when exists.  Write a smaller string to verify truncation.
  {
    ExclusiveWriteFile ewf(testFileName);
    ewf.stream() << "hi\n";
    ewf.selfCheck();
  }
  EXPECT_EQ(testFileContents(), "hi\n");
}


void test_close()
{
  // TODO
}


// Open the file and hold it open.  This is for interactively testing
// that the mutual exclusion works.
void test_wait()
{
  std::cout << "I am PID " << getProcessId() << "\n";
  std::cout << "Trying to open " << testFileName << " ...\n";

  try {
    ExclusiveWriteFile ewf(testFileName);
    ewf.stream() << "opened by PID " << getProcessId() << "\n";
    ewf.stream().flush();

    std::cout << "Opened " << testFileName << "\n";

    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
  }

  catch (XSysError &x) {
    DIAG("Got XSysError:");
    PVAL(x.reason);
    PVAL(x.reasonString);
    PVAL(x.sysErrorCode);
    PVAL(x.sysReasonString);
    PVAL(x.syscallName);
    PVAL(x.context);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_exclusive_write_file()
{
  if (envAsBool("EWFT_WAIT")) {
    test_wait();
    return;
  }

  test_simple();
  test_close();

  // TODO: more tests
}


// EOF
