// system-error-code.h
// Portable wrapper around platform-specific error codes.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SYSTEM_ERROR_CODE_H
#define SMBASE_SYSTEM_ERROR_CODE_H

#include "smbase/portable-error-code.h"          // PortableErrorCode
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NULLABLE
#include "smbase/std-string-fwd.h"               // std::string

#include <cstdint>                               // std::uint32_t
#include <iosfwd>                                // std::ostream


OPEN_NAMESPACE(smbase)


// Encapsulates a platform-specific error code.
class SystemErrorCode {
private:     // data
  // Platform-specific error code numeric value.  0 means "none".
  std::uint32_t m_systemCode;

public:      // methods
  explicit SystemErrorCode(std::uint32_t systemCode = 0)
    : m_systemCode(systemCode)
  {}

  SystemErrorCode(SystemErrorCode const &) = default;
  SystemErrorCode &operator=(SystemErrorCode const &) = default;

  // Retrieve the current platform-specific error, i.e., `errno` on
  // POSIX and `GetLastError()` on Windows.
  static SystemErrorCode getCurrent();

  // True if we are carrying system error code.
  bool hasSystemCode() const { return m_systemCode != 0; }

  // Platform-specific error code value.
  std::uint32_t systemCode() const { return m_systemCode; }

  // Return the platform-specific name for this code, e.g., "EACCESS",
  // or null if we don't know.
  char const * NULLABLE codeNameOpt() const;

  // Like `codeNameOpt`, except if that returns null, return a string
  // like "Error code 1234 (0x4d2)"
  std::string codeName() const;

  // Return a human-readable platform-specific description of the code.
  // This string does not end with a newline.  It may or may not end
  // with sentence-ending punctuation.
  std::string codeDescription() const;

  // Map the system error code to its portable counterpart.  If we do
  // not have a system code, return PEC_NO_ERROR.  If there is not a
  // portable counterpart, return PEC_UNKNOWN.
  PortableErrorCode portableCode() const;

  // Write `codeName()` to `os`.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, SystemErrorCode code)
    { code.write(os); return os; }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SYSTEM_ERROR_CODE_H
