// posix-fd-ostream.cc
// Code for `posix-fd-ostream` module.

#include "smbase/sm-platform.h"        // PLATFORM_IS_POSIX

#if PLATFORM_IS_POSIX

#include "posix-fd-ostream.h"          // this module

#include "smbase/overflow.h"           // convertNumber
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/syserr.h"             // xsyserror
#include "smbase/xassert.h"            // xassert

#include <unistd.h>                    // write, ssize_t, size_t

OPEN_NAMESPACE(smbase)


// ------------------------- PosixFDStreambuf --------------------------
std::streamsize PosixFDStreambuf::writeToDestination(
  const char *src, std::streamsize count)
{
  ssize_t numWritten = write(m_fd, src, convertNumber<size_t>(count));
  if (numWritten < 0) {
    // This gets caught and turned into a string in the base class.
    xsyserror("write");
  }

  return convertNumber<std::streamsize>(numWritten);
}


PosixFDStreambuf::~PosixFDStreambuf() noexcept
{
  autoflush();
}


PosixFDStreambuf::PosixFDStreambuf(
  int fd, std::size_t bufSize)
  : BufferedStreambuf(bufSize),
    m_fd(fd)
{
  selfCheck();
}


void PosixFDStreambuf::selfCheck() const
{
  xassert(m_fd >= 0);
}


// -------------------------- PosixFDOStream ---------------------------
PosixFDOStream::~PosixFDOStream()
{
  if (fail()) {
    m_streambuf.m_enableAutoflush = false;
  }
}


PosixFDOStream::PosixFDOStream(
  int fd, std::size_t bufSize)
  : std::ostream(nullptr),
    m_streambuf(fd, bufSize)
{
  // We cannot pass `&m_streambuf` to the `ostream` constructor in the
  // initializer list because `m_streambuf` would not yet be
  // constructed, which would technically be undefined behavior (even
  // though it seemingly works everywhere).  So we call `init` after the
  // members are initialized.
  init(&m_streambuf);

  selfCheck();
}


std::optional<std::string>
PosixFDOStream::getExceptionMessage() const
{
  return m_streambuf.m_exceptionMessage;
}


void PosixFDOStream::clearExceptionMessage()
{
  m_streambuf.m_exceptionMessage.reset();
}


void PosixFDOStream::selfCheck() const
{
  m_streambuf.selfCheck();
}


CLOSE_NAMESPACE(smbase)


#else // !PLATFORM_IS_POSIX
// The module is empty on other platforms.

#endif

// EOF
