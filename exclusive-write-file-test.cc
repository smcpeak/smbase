// exclusive-write-file-test.cc
// Tests for `exclusive-write-file` module.

#include "smbase/exclusive-write-file.h"         // module under test

#include "smbase/nonport.h"                      // getProcessId
#include "smbase/sm-env.h"                       // smbase::envAsBool
#include "smbase/sm-file-util.h"                 // SMFileUtil
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/syserr.h"                       // smbase::XSysError

#include <cstdlib>                               // std::exit
#include <iostream>                              // std::{cin, cout, endl}
#include <memory>                                // std::unique_ptr

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
  sfu.removeFileIfExists(testFileName);

  {
    ExclusiveWriteFile ewf(testFileName);
    ewf.stream() << "hello test_close\n";
    ewf.selfCheck();

    // No data should be written yet.
    EXPECT_EQ(testFileContents(), "");

    // Closing flushes.
    ewf.close();
    ewf.selfCheck();
    EXPECT_EQ(testFileContents(), "hello test_close\n");

    // Redundant close is fine.
    ewf.close();
    ewf.selfCheck();

    // Destructor should also be fine.
  }

  // Double-check the contents.
  EXPECT_EQ(testFileContents(), "hello test_close\n");
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

  catch (XExclusiveWriteFileConflict &x) {
    std::cout << "Conflict: " << x << "\n";
    std::cout << "Code: " << x.m_systemErrorCode.codeName() << "\n";
    std::exit(1);
  }

  catch (XSysError &x) {
    std::cout << "XSysError: " << x << "\n";
    std::cout << "Code: " << x.getSystemErrorCode().codeName() << "\n";
    std::exit(2);
  }

  catch (XBase &x) {
    std::cout << "XBase: " << x << "\n";
    std::exit(2);
  }
}


void test_createExclusive()
{
  std::string fname1(testFileName);
  std::unique_ptr<ExclusiveWriteFile> file1(
    tryCreateExclusiveWriteFile(fname1));
  VPVAL(fname1);
  file1->stream() << "write to " << fname1 << "\n";
  EXPECT_EQ(fname1, testFileName);

  std::string fname2(testFileName);
  std::unique_ptr<ExclusiveWriteFile> file2(
    tryCreateExclusiveWriteFile(fname2));
  VPVAL(fname2);
  file2->stream() << "write to " << fname2 << "\n";

  // On Windows, we get exclusion within the process.  What will Linux
  // do?
  EXPECT_EQ(fname2, stringb(testFileName << ".2"));
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
  test_createExclusive();
}


// EOF
