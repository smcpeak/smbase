// system-error-code-test.cc
// Tests for `system-error-code` module.

#include "smbase/system-error-code.h"  // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-platform.h"        // PLATFORM_IS_WINDOWS
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/sm-windows.h"         // ERROR_FILE_NOT_FOUND

#include <errno.h>                     // ENOENT
#include <stdio.h>                     // FILE ops

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_noError()
{
  SystemErrorCode none;
  EXPECT_EQ(none.hasSystemCode(), false);
  EXPECT_EQ(none.systemCode(), 0);

  if (PLATFORM_IS_WINDOWS) {
    EXPECT_EQ(none.codeNameOpt(), "ERROR_SUCCESS");
    EXPECT_EQ(none.codeDescription(),
      "The operation completed successfully.");
  }
  else {
    EXPECT_EQ(none.codeNameOpt(), "EZERO");

    // This is what I see on Ubuntu 22.04.
    EXPECT_EQ(none.codeDescription(), "Success");
  }
  EXPECT_EQ(none.codeName(), none.codeNameOpt());
  EXPECT_EQ(none.portableCode(), PortableErrorCode::PEC_NO_ERROR);
}


void test_fileNotFound()
{
  // Use `fopen` to minimize the layers between me and the OS (without
  // resorting to non-portable calls).
  FILE *fp = fopen("some-nonexistent-file", "r");
  SystemErrorCode sec = SystemErrorCode::getCurrent();
  xassert(!fp);

  EXPECT_EQ(sec.hasSystemCode(), true);
  if (PLATFORM_IS_WINDOWS) {
    EXPECT_EQ(sec.systemCode(), ERROR_FILE_NOT_FOUND);
    EXPECT_EQ(sec.codeNameOpt(), "ERROR_FILE_NOT_FOUND");
    EXPECT_EQ(sec.codeName(), "ERROR_FILE_NOT_FOUND");
    EXPECT_EQ(sec.codeDescription(),
      "The system cannot find the file specified.");
  }
  else {
    EXPECT_EQ(sec.systemCode(), ENOENT);
    EXPECT_EQ(sec.codeNameOpt(), "ENOENT");
    EXPECT_EQ(sec.codeName(), "ENOENT");
    EXPECT_EQ(sec.codeDescription(),
      "No such file or directory");
  }
  EXPECT_EQ(sec.portableCode(), PortableErrorCode::PEC_FILE_NOT_FOUND);
}


void test_unknownCode()
{
  SystemErrorCode sec(0x12345678);
  EXPECT_EQ(sec.hasSystemCode(), true);
  EXPECT_EQ(sec.systemCode(), 0x12345678);
  xassert(sec.codeNameOpt() == nullptr);
  EXPECT_EQ(sec.codeName(), "Error code 305419896 (0x12345678)");
  if (PLATFORM_IS_WINDOWS) {
    // This is what FormatMessage does.
    EXPECT_EQ(sec.codeDescription(),
      "Error with unrecognized code 305419896 (0x12345678)");
  }
  else {
    // This is what I see on Ubuntu 22.04.
    EXPECT_EQ(sec.codeDescription(),
      "Unknown error 305419896");
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_system_error_code()
{
  test_noError();
  test_fileNotFound();
  test_unknownCode();
}


// EOF
