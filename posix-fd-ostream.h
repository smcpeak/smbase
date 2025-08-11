// posix-fd-ostream.h
// Subclass of `std::ostream` that wraps a Windows `HANDLE`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_POSIX_FD_OSTREAM_H
#define SMBASE_POSIX_FD_OSTREAM_H

#include "smbase/buffered-streambuf.h" // BufferedStreambuf
#include "smbase/sm-macros.h"          // NO_OBJECT_COPIES, OPEN_NAMESPACE

#include <ios>                         // std::streamsize
#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Stream buffer that writes to a POSIX file descriptor, buffering the
// data before sending it to `write`.
class PosixFDStreambuf : public BufferedStreambuf {
  NO_OBJECT_COPIES(PosixFDStreambuf);

private:     // data
  // File descriptor we are writing to.  Never invalid.
  int m_fd;

protected:   // methods
  // BufferedStreambuf methods.
  virtual std::streamsize writeToDestination(
    const char *src, std::streamsize count) override;

public:      // methods
  virtual ~PosixFDStreambuf() noexcept override;

  // Arrange to write to `fd`.  Requires that it be valid initially and
  // for the lifetime of this object.  This class does *not* take
  // ownership of the descriptor.
  explicit PosixFDStreambuf(
    int fd, std::size_t bufSize = 0x1000);

  // Assert invariants.
  void selfCheck() const;
};


// Output stream that writes to a POSIX file descriptor.  It does *not*
// take ownership of the descriptor.
class PosixFDOStream : public std::ostream {
  NO_OBJECT_COPIES(PosixFDOStream);

private:     // data
  // Intermediate buffer that sends data to the descriptor.
  PosixFDStreambuf m_streambuf;

public:      // methods
  // If `fail()`, this will disable autoflush on `m_streambuf`.
  virtual ~PosixFDOStream() override;

  // Wrap an ostream around a descriptor.
  PosixFDOStream(int fd, std::size_t bufSize = 0x1000);

  // Return the `what()` string of an exception thrown by
  // `PosixFDStreambuf::writeToDevice` on error, if there is one.
  std::optional<std::string> getExceptionMessage() const;

  // Remove the error message (if there is one).
  void clearExceptionMessage();

  // Assert invariants.
  void selfCheck() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_POSIX_FD_OSTREAM_H
