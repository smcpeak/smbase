// system-error-code.cc
// Code for `system-error-code` module.

#include "system-error-code.h"         // this module

#include "smbase/compare-util.h"       // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DEFINE_ENUMERATION_TO_STRING_OR, RETURN_ENUMERATION_STRING_OR, PRETEND_USED
#include "smbase/sm-span.h"            // smbase::Span
#include "smbase/sm-windows.h"         // PLATFORM_IS_WINDOWS, GetLastError(), etc.
#include "smbase/stringb.h"            // stringb

#include <algorithm>                   // std::sort
#include <iostream>                    // std::hex


OPEN_NAMESPACE(smbase)


OPEN_ANONYMOUS_NAMESPACE


// Entry in a map from code to name and optional portable code.
class NameEntry {
public:      // data
  // System-specific code.
  std::uint32_t m_systemCode;

  // System-specific name for this code.
  char const *m_codeName;

  // Portable counterpart, or PEC_UNKNOWN if there is none.
  PortableErrorCode m_portableCode;

public:      // methods
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(NameEntry)

  // For `lower_bound` we need to be able to compare just the
  // system code.
  bool operator<(std::uint32_t sc) const
    { return m_systemCode < sc; }
};


// Macros to help with building the table.

// Name and PortableErrorCode.
#define NAME_ENTRY_PEC(c, r) { c, #c, PortableErrorCode::r }

// Name without PCE, for when I want to be able to map a code to a name
// without specifying a portable equivalent.
#define NAME_ENTRY(c) { c, #c, PortableErrorCode::PEC_UNKNOWN }


int NameEntry::compareTo(NameEntry const &b) const
{
  auto const &a = *this;

  // First compare by `m_systemCode` since that is what we search on.
  RET_IF_COMPARE_MEMBERS(m_systemCode);

  // In at least one case below (for `EINVFMT`), I may have more than
  // one entry for a given system code.  Prefer the one with the smaller
  // portable code in such a case.
  RET_IF_COMPARE_MEMBERS(m_portableCode);

  return 0;
}


// Sort `entries` in place by `m_systemCode`.  This should be done once
// before using `lookupCode`.
void sortNameEntries(Span<NameEntry> entries)
{
  std::sort(entries.begin(), entries.end());

  // GCC complains I haven't used all of my comparison operators...
  NameEntry const &e = entries[0];
  PRETEND_USED(e <= e);
  PRETEND_USED(e <  e);
  PRETEND_USED(e >= e);
  PRETEND_USED(e >  e);
  PRETEND_USED(e == e);
  PRETEND_USED(e != e);
}


// Look up `systemCode` in the sorted `entries` array.  Return a pointer
// to the element with the same code if found, and null otherwise.
NameEntry const * NULLABLE lookupCodeInEntries(
  Span<NameEntry const> entries, std::uint32_t systemCode)
{
  auto it = std::lower_bound(entries.begin(), entries.end(), systemCode);
  if (it != entries.end() &&
      (*it).m_systemCode == systemCode) {
    return &*it;
  }
  else {
    return nullptr;
  }
}


std::string decAndHex(std::uint32_t n)
{
  return stringb(n << " (0x" << std::hex << n << ")");
}


CLOSE_ANONYMOUS_NAMESPACE



std::string SystemErrorCode::codeName() const
{
  if (char const *name = codeNameOpt()) {
    return name;
  }
  else {
    return stringb("Error code " << decAndHex(m_systemCode));
  }
}


// ------------------------------ Windows ------------------------------
#if PLATFORM_IS_WINDOWS

// Note: Not `const` since we sort this array.
NameEntry nameEntries[] = {
  NAME_ENTRY_PEC(ERROR_SUCCESS,            PEC_NO_ERROR),
  NAME_ENTRY_PEC(ERROR_FILE_NOT_FOUND,     PEC_FILE_NOT_FOUND),
  NAME_ENTRY_PEC(ERROR_PATH_NOT_FOUND,     PEC_FILE_NOT_FOUND),
  NAME_ENTRY_PEC(ERROR_ACCESS_DENIED,      PEC_ACCESS_DENIED),
  NAME_ENTRY_PEC(ERROR_NOT_ENOUGH_MEMORY,  PEC_OUT_OF_MEMORY),
  NAME_ENTRY_PEC(ERROR_OUTOFMEMORY,        PEC_OUT_OF_MEMORY),
  NAME_ENTRY_PEC(ERROR_INVALID_BLOCK,      PEC_SEGFAULT),
  NAME_ENTRY_PEC(ERROR_BAD_FORMAT,         PEC_FORMAT),
  NAME_ENTRY_PEC(ERROR_INVALID_DATA,       PEC_INVALID_ARGUMENT),
  NAME_ENTRY_PEC(ERROR_WRITE_PROTECT,      PEC_READ_ONLY),
  NAME_ENTRY_PEC(ERROR_ALREADY_EXISTS,     PEC_ALREADY_EXISTS),
  NAME_ENTRY_PEC(ERROR_BUSY,               PEC_BUSY),

  /* I considered various options for how to map this.  On Linux, a
     failure of `fcntl(F_SETLK)` yields `EAGAIN`, but POSIX specifies
     that `EACCES` is also a possibility in that situation.  Both are
     quite vague though.

     Map to PEC_AGAIN:
       - Pro: Agrees with Linux.
       - Pro: Code exists, isn't yet used on Windows.
       - Con: Not necessarily aligned with POSIX, so possibly confusing.
       - Con: Not specific to the situation, so possibly ambiguous.

     Map to PEC_ACCESS_DENIED:
       - Con: Quite misleading.  IMO, "access denied" indicates that an
         action conflicted with a *security* policy, whereas mutual
         exclusion is primarily about data *integrity*.

     Map to a new PEC_SHARING_VIOLATION (e.g.):
       - Pro: Specific.
       - Con: Definitely misaligned with Linux and POSIX.
       - Con: Yet a third PEC_* code for this situation.

     This experience seems to show that trying to map all OS-specific
     codes into a coarse-grained "portable" code space is going to be,
     at best, very difficult, and thus the specific choice here is
     perhaps not very important since it's part of an overall system
     that does not work.

     Instead, I may need to rely on the approach taken with
     `XExclusiveWriteFile`, where a highly-specific exception type is
     thrown from the spot where I know the context.  It carries the
     `SystemErrorCode` but we don't have to look at it to know what
     happened, and thus have no need for a "portable" interpretation.

     Therefore, I went with the least-friction option of `PEC_AGAIN`.
  */
  NAME_ENTRY_PEC(ERROR_SHARING_VIOLATION,  PEC_AGAIN),
};


/*static*/ SystemErrorCode SystemErrorCode::getCurrent()
{
  return SystemErrorCode(GetLastError());
}


std::string SystemErrorCode::codeDescription() const
{
  // method to translate an error code into a string on win32; this
  // code is copied+modified from the win32 SDK docs for FormatMessage

  // get the string
  LPVOID lpMsgBuf = nullptr;
  DWORD msgLen = FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    m_systemCode,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
  );

  // The docs say `FormatMessage` returns 0 when it fails, but I'll
  // double-check `lpMsgBuf` for safety.
  if (msgLen == 0 || lpMsgBuf == nullptr) {
    DWORD secondCode = GetLastError();
    if (secondCode == ERROR_MR_MID_NOT_FOUND) {
      // The error code is invalid.
      return stringb(
        "Error with unrecognized code " << decAndHex(m_systemCode));
    }
    else {
      return stringb(
        "Unknown error; the attempt to look up error code " <<
        decAndHex(m_systemCode) <<
        " using FormatMessage led to error code " <<
        decAndHex(secondCode));
    }
  }

  // now the SDK says: "Process any inserts in lpMsgBuf."
  //
  // I think this means that lpMsgBuf might have "%" escape
  // sequences in it... oh well, I'm just going to keep them

  // Remove any newline characters from the end.
  char *msgBuf = static_cast<char*>(lpMsgBuf);
  for (int i = msgLen-1; i >= 0; --i) {
    if (msgBuf[i] == '\r' || msgBuf[i] == '\n') {
      msgBuf[i] = 0;
    }
  }

  // make a copy of the string
  std::string sysMsg = msgBuf;

  // Free the buffer.
  LocalFree( lpMsgBuf );

  return sysMsg;
}


// ------------------------------- POSIX -------------------------------
#else // !PLATFORM_IS_WINDOWS

#include <errno.h>                     // errno
#include <string.h>                    // strerror

// Deal with inconsistent names across platforms.
#ifndef EZERO
#  define EZERO 0
#endif
#ifndef ENOPATH
#  define ENOPATH ENOENT
#endif
#ifndef EINVMEM
#  define EINVMEM EFAULT
#endif
#ifndef EINVFMT
#  define EINVFMT 0     // Won't be used because EZERO takes priority.
#endif


// Note: Not `const` since we sort this array.
//
// Generally I prefer to use names documented in POSIX, hence I use
// ENOENT here instead of ENOFILE.
NameEntry nameEntries[] = {
  NAME_ENTRY_PEC(EZERO,         PEC_NO_ERROR),
  NAME_ENTRY_PEC(ENOENT,        PEC_FILE_NOT_FOUND),
  NAME_ENTRY_PEC(ENOPATH,       PEC_FILE_NOT_FOUND),
  NAME_ENTRY_PEC(EACCES,        PEC_ACCESS_DENIED),
  NAME_ENTRY_PEC(ENOMEM,        PEC_OUT_OF_MEMORY),
  NAME_ENTRY_PEC(EINVMEM,       PEC_SEGFAULT),
  NAME_ENTRY_PEC(EINVFMT,       PEC_FORMAT),
  NAME_ENTRY_PEC(EINVAL,        PEC_INVALID_ARGUMENT),
  NAME_ENTRY_PEC(EROFS,         PEC_READ_ONLY),
  NAME_ENTRY_PEC(EEXIST,        PEC_ALREADY_EXISTS),
  NAME_ENTRY_PEC(EAGAIN,        PEC_AGAIN),
  NAME_ENTRY_PEC(EBUSY,         PEC_BUSY),
  NAME_ENTRY_PEC(ENAMETOOLONG,  PEC_INVALID_FILENAME),
};


/*static*/ SystemErrorCode SystemErrorCode::getCurrent()
{
  return SystemErrorCode(errno);
}


std::string SystemErrorCode::codeDescription() const
{
  // The POSIX docs for `strerror` say it always generates a string
  // and that there is no way to tell when it has failed.
  return strerror(m_systemCode);
}


#endif


// -------------------------- Both platforms ---------------------------
static NameEntry const * NULLABLE lookupCode(std::uint32_t systemCode)
{
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    sortNameEntries(nameEntries);
  }

  return lookupCodeInEntries(nameEntries, systemCode);
}


char const * NULLABLE SystemErrorCode::codeNameOpt() const
{
  if (NameEntry const *entry = lookupCode(m_systemCode)) {
    return entry->m_codeName;
  }
  else {
    return nullptr;
  }
}


PortableErrorCode SystemErrorCode::portableCode() const
{
  if (NameEntry const *entry = lookupCode(m_systemCode)) {
    return entry->m_portableCode;
  }
  else {
    return PortableErrorCode::PEC_UNKNOWN;
  }
}


void SystemErrorCode::write(std::ostream &os) const
{
  os << codeName();
}


CLOSE_NAMESPACE(smbase)


// EOF
