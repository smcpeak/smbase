// windows-handle-ostream-test.cc
// Tests for `windows-handle-ostream` module.

#include "smbase/windows-handle-ostream.h"       // module under test

#include "smbase/exc.h"                          // EXN_CONTEXT
#include "smbase/sm-file-util.h"                 // SMFileUtil
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/syserr.h"                       // xsyserror

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Class to open a file for writing and close it in the dtor.
class OpenFileHandle {
public:      // data
  // Handle to the open file.
  HANDLE m_handle;

public:      // methods
  explicit OpenFileHandle(std::string const &fname)
    : m_handle(INVALID_HANDLE_VALUE)
  {
    EXN_CONTEXT(fname);

    m_handle = CreateFileA(
      fname.c_str(),
      GENERIC_READ | GENERIC_WRITE,    // I can r/w (though I only write).
      0,                               // No sharing.
      nullptr,
      CREATE_ALWAYS,                   // Create or truncate.
      FILE_ATTRIBUTE_NORMAL,
      nullptr);
    if (m_handle == INVALID_HANDLE_VALUE) {
      xsyserror("CreateFileA");
    }
  }

  ~OpenFileHandle()
  {
    CloseHandle(m_handle);
  }
};


char const * const testFileName = "out/whot.txt";

std::string testFileContents()
{
  SMFileUtil sfu;
  return sfu.readFileAsString(testFileName);
}


// Very simple usage, with autoflush.
void test_simple()
{
  {
    OpenFileHandle ofh(testFileName);
    WindowsHandleOStream os(ofh.m_handle);
    os << "hello simple\n";

    os.selfCheck();
  }
  EXPECT_EQ(testFileContents(), "hello simple\n");
}


void test_error()
{
  {
    OpenFileHandle ofh(testFileName);
    WindowsHandleOStream os(ofh.m_handle);
    os << "hello error\n";
    os.flush();

    CloseHandle(ofh.m_handle);
    os << "oops\n";
    os.flush();

    EXPECT_EQ(os.fail(), true);
    EXPECT_HAS_SUBSTRING(*( os.getExceptionMessage() ),
      "WriteFile: The handle is invalid.");
  }
  EXPECT_EQ(testFileContents(), "hello error\n");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_windows_handle_ostream()
{
  test_simple();
  test_error();
}


// EOF
