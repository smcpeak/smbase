// portable-error-code.h
// Portable enumeration of system call error code categories.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_PORTABLE_ERROR_CODE_H
#define SMBASE_PORTABLE_ERROR_CODE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <iosfwd>                      // std::ostream


OPEN_NAMESPACE(smbase)


// Portable categories of system call errors.
//
// The basic idea is we map platform-specific error codes to elements of
// this enumeration in order to allow code to portably distinguish among
// various error causes.
//
// It is anticipated that, as certain errors become important on
// certain platforms, that this list will be extended as necessary.
enum class PortableErrorCode : int {
  // No error occurred.
  //
  // POSIX: 0
  // Windows: ERROR_SUCCESS (0)
  PEC_NO_ERROR,

  // Specified file does not exist.
  //
  // POSIX: ENOENT
  // Windows: ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND
  PEC_FILE_NOT_FOUND,

  // 2025-08-09: There was `PEC_PATH_NOT_FOUND` here, but I removed it
  // because it is not usefully different from `PEC_FILE_NOT_FOUND` and
  // (relatedly) not consistently distinguished in system error codes.

  // Permission error.
  //
  // POSIX: EACCESS
  // Windows: ERROR_ACCESS_DENIED
  PEC_ACCESS_DENIED,

  // Out of memory, or not enough space.
  //
  // POSIX: ENOMEM
  // Windows: ERROR_NOT_ENOUGH_MEMORY, ERROR_OUTOFMEMORY
  PEC_OUT_OF_MEMORY,

  // Invalid address or pointer.
  //
  // POSIX: EFAULT
  // Windows: ERROR_INVALID_BLOCK
  PEC_SEGFAULT,

  // Bad data format.
  //
  // POSIX: EINVFMT
  // Windows: ERROR_BAD_FORMAT
  PEC_FORMAT,

  // POSIX: EINVAL
  // Windows: ERROR_INVALID_DATA
  PEC_INVALID_ARGUMENT,

  // Attempt to write to a read-only resource.
  //
  // POSIX: EROFS
  // Windows: ERROR_WRITE_PROTECT
  PEC_READ_ONLY,

  // File exists already.
  //
  // POSIX: EEXIST
  // Windows: ERROR_ALREADY_EXISTS
  PEC_ALREADY_EXISTS,

  // Resource temporarily unavailable.
  //
  // POSIX: EAGAIN
  // Windows: none
  PEC_AGAIN,

  // Resource busy.
  //
  // POSIX: EBUSY
  // Windows: ERROR_BUSY
  PEC_BUSY,

  // File name too long, bad chars, etc.
  //
  // POSIX: ENAMETOOLONG
  // Windows: none
  PEC_INVALID_FILENAME,

  // System error code that isn't mapped to one of the above.
  PEC_UNKNOWN,

  // Last item in the list, total number of reason codes.
  NUM_REASONS
};

// Return a string like "PEC_NO_ERROR", or "<invalid>" if `r` is out of
// bounds.
char const *toString(PortableErrorCode r);

// Write `toString(r)`.
std::ostream &operator<<(std::ostream &os, PortableErrorCode r);

// I sometimes use unary `+` when printing values to ensure I get an
// integer instead of a char.  Allow this enumeration to pass through
// that idiom.
inline PortableErrorCode operator+(PortableErrorCode r)
  { return r; }

// Human-readable string like "File not found".
char const *portableCodeDescription(PortableErrorCode r);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_PORTABLE_ERROR_CODE_H
