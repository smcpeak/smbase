// windows-handle-ostream.cc
// Code for `windows-handle-ostream` module.

#include "windows-handle-ostream.h"    // this module

#include "smbase/overflow.h"           // convertNumber
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-windows.h"         // HANDLE
#include "smbase/syserr.h"             // xsyserror
#include "smbase/xassert.h"            // xassert


OPEN_NAMESPACE(smbase)


// ---------------------- WindowsHandleStreambuf -----------------------
std::streamsize WindowsHandleStreambuf::writeToDestination(
  const char *src, std::streamsize count)
{
  DWORD numWritten = 0;
  if (!WriteFile(
         m_handle,
         src,
         static_cast<DWORD>(count),
         &numWritten,
         nullptr)) {
    // This gets caught and turned into a string in the base class.
    xsyserror("WriteFile");
  }

  return convertNumber<std::streamsize>(numWritten);
}


WindowsHandleStreambuf::~WindowsHandleStreambuf() noexcept
{
  autoflush();
}


WindowsHandleStreambuf::WindowsHandleStreambuf(
  HANDLE handle, std::size_t bufSize)
  : BufferedStreambuf(bufSize),
    m_handle(handle)
{
  selfCheck();
}


void WindowsHandleStreambuf::selfCheck() const
{
  xassert(m_handle != INVALID_HANDLE_VALUE);
}


// ----------------------- WindowsHandleOStream ------------------------
WindowsHandleOStream::~WindowsHandleOStream()
{
  if (fail()) {
    m_streambuf.m_enableAutoflush = false;
  }
}


WindowsHandleOStream::WindowsHandleOStream(
  HANDLE handle, std::size_t bufSize)
  : std::ostream(nullptr),
    m_streambuf(handle, bufSize)
{
  // We cannot pass `&m_streambuf` to the `ostream` constructor in the
  // initializer list because `m_streambuf` would not yet be
  // constructed, which would technically be undefined behavior (even
  // though it seemingly works everywhere).  So we call `init` after the
  // members are initialized.
  init(&m_streambuf);
}


std::optional<std::string>
WindowsHandleOStream::getExceptionMessage() const
{
  return m_streambuf.m_exceptionMessage;
}


void WindowsHandleOStream::clearExceptionMessage()
{
  m_streambuf.m_exceptionMessage.reset();
}


CLOSE_NAMESPACE(smbase)


// EOF
