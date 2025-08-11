// posix-fd-ostream-test.cc
// Tests for `posix-fd-ostream` module.

#include "smbase/sm-platform.h"        // PLATFORM_IS_POSIX

#if PLATFORM_IS_POSIX

#include "smbase/posix-fd-ostream.h"   // module under test

#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/sm-file-util.h"       // SMFileUtil
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/syserr.h"             // xsyserror

#include <fcntl.h>                     // open, O_RDWR, etc.
#include <unistd.h>                    // close

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Class to open a file for writing and close it in the dtor.
class OpenFileDescriptor {
public:      // data
  // Descriptor for the open file.
  int m_fd;

public:      // methods
  explicit OpenFileDescriptor(std::string const &fname)
    : m_fd(-1)
  {
    EXN_CONTEXT_STRING(doubleQuote(fname));

    m_fd = open(fname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (m_fd < 0) {
      xsyserror("open");
    }
  }

  ~OpenFileDescriptor()
  {
    close(m_fd);
  }
};


char const * const testFileName = "out/pfot.txt";

std::string testFileContents()
{
  SMFileUtil sfu;
  return sfu.readFileAsString(testFileName);
}


// Very simple usage, with autoflush.
void test_simple()
{
  {
    OpenFileDescriptor ofd(testFileName);
    PosixFDOStream os(ofd.m_fd);
    os << "hello simple\n";

    os.selfCheck();
  }
  EXPECT_EQ(testFileContents(), "hello simple\n");
}


void test_error()
{
  {
    OpenFileDescriptor ofd(testFileName);
    PosixFDOStream os(ofd.m_fd);
    os << "hello error\n";
    os.flush();

    close(ofd.m_fd);
    os << "oops\n";
    os.flush();

    EXPECT_EQ(os.fail(), true);
    EXPECT_EQ(*( os.getExceptionMessage() ),
      "write: Bad file descriptor");
  }
  EXPECT_EQ(testFileContents(), "hello error\n");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_posix_fd_ostream()
{
  test_simple();
  test_error();
}


#else // !PLATFORM_IS_POSIX
// The module is essentially empty on other platforms.
void test_posix_fd_ostream()
{}

#endif


// EOF
