// windows-handle-ostream.h
// Subclass of `std::ostream` that wraps a Windows `HANDLE`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_WINDOWS_HANDLE_OSTREAM_H
#define SMBASE_WINDOWS_HANDLE_OSTREAM_H

#include "smbase/sm-windows.h"         // HANDLE

#include "smbase/buffered-streambuf.h" // BufferedStreambuf
#include "smbase/sm-macros.h"          // NO_OBJECT_COPIES, OPEN_NAMESPACE

#include <ios>                         // std::streamsize
#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Stream buffer that writes to a Windows `HANDLE`, buffering the data
// before sending it to `WriteFile`.
class WindowsHandleStreambuf : public BufferedStreambuf {
  NO_OBJECT_COPIES(WindowsHandleStreambuf);

private:     // data
  // Handle we are writing to.  Never invalid.
  HANDLE m_handle;

protected:   // methods
  // BufferedStreambuf methods.
  virtual std::streamsize writeToDestination(
    const char *src, std::streamsize count) override;

public:      // methods
  virtual ~WindowsHandleStreambuf() noexcept override;

  // Arrange to write to `handle`.  Requires that it be valid initially
  // and for the lifetime of this object.  This class does *not* take
  // ownership of the handle.
  explicit WindowsHandleStreambuf(
    HANDLE handle, std::size_t bufSize = 0x1000);

  // Assert invariants.
  void selfCheck() const;
};


// Output stream that writes to a Windows `HANDLE`.  It does *not* take
// ownership of the handle.
class WindowsHandleOStream : public std::ostream {
  NO_OBJECT_COPIES(WindowsHandleOStream);

private:     // data
  // Intermediate buffer that sends data to the handle.
  WindowsHandleStreambuf m_streambuf;

public:      // methods
  // If `fail()`, this will disable autoflush on `m_streambuf`.
  // Consequently, if an error happens, then to re-enable autoflush, a
  // client must both call `clear()` to clear the stream error bits, and
  // also call `clearExceptionMessage()` if the failure resulted in an
  // exception message being stored, which it normally will for
  // `WindowsHandleStreambuf`. (BufferedStreambuf itself disables
  // autoflush if the message has not been cleared.)
  virtual ~WindowsHandleOStream() override;

  // Wrap an ostream around a handle.
  WindowsHandleOStream(HANDLE handle, std::size_t bufSize = 0x1000);

  // Return the `what()` string of an exception thrown by
  // `WindowsHandleStreambuf::writeToDevice` on error, if there is one.
  std::optional<std::string> getExceptionMessage() const;

  // Remove the error message (if there is one).
  void clearExceptionMessage();

  // Assert invariants.
  void selfCheck() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_WINDOWS_HANDLE_OSTREAM_H
