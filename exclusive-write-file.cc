// exclusive-write-file.cc
// Code for `exclusive-write-file` module.

#include "smbase/sm-platform.h"                  // PLATFORM_IS_WINDOWS

#include "exclusive-write-file.h"                // this module

#include "smbase/exc.h"                          // EXN_CONTEXT, GENERIC_CATCH_BEGIN, OPEN_NAMESPACE
#include "smbase/sm-env.h"                       // smbase::envAsIntOr
#include "smbase/syserr.h"                       // xsyserror

#include <iostream>                              // std::ostream
#include <memory>                                // std::unique_ptr
#include <string>                                // std::string
#include <string_view>                           // std::string_view

#if PLATFORM_IS_WINDOWS

  #include "smbase/sm-windows.h"                 // CreateFileA, etc.
  #include "smbase/windows-handle-ostream.h"     // smbase::WindowsHandleOStream

#else

  #include "smbase/posix-fd-ostream.h"           // smbase::PosixFDOStream

  #include <fstream>                             // std::filebuf

  #include <errno.h>                             // errno
  #include <fcntl.h>                             // fcntl, struct flock
  #include <unistd.h>                            // open, close, ftruncate

#endif


OPEN_NAMESPACE(smbase)


// -------------------- XExclusiveWriteFileConflict --------------------
// ---- create-tuple-class: definitions for XExclusiveWriteFileConflict
/*AUTO_CTC*/ XExclusiveWriteFileConflict::XExclusiveWriteFileConflict(
/*AUTO_CTC*/   SystemErrorCode const &systemErrorCode,
/*AUTO_CTC*/   std::string const &fname)
/*AUTO_CTC*/   : XBase(),
/*AUTO_CTC*/     m_systemErrorCode(systemErrorCode),
/*AUTO_CTC*/     m_fname(fname)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XExclusiveWriteFileConflict::XExclusiveWriteFileConflict(XExclusiveWriteFileConflict const &obj) noexcept
/*AUTO_CTC*/   : XBase(obj),
/*AUTO_CTC*/     DMEMB(m_systemErrorCode),
/*AUTO_CTC*/     DMEMB(m_fname)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XExclusiveWriteFileConflict &XExclusiveWriteFileConflict::operator=(XExclusiveWriteFileConflict const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XBase::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_systemErrorCode);
/*AUTO_CTC*/     CMEMB(m_fname);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XExclusiveWriteFileConflict::getConflict() const
{
  return "File is locked by another process.";
}


// --------------------- ExclusiveWriteFilePrivate ---------------------
#if PLATFORM_IS_WINDOWS


class ExclusiveWriteFilePrivate {
public:      // data
  // Owning handle to the open and locked file.
  HANDLE m_hFile;

  // Stream wrapped around the handle.  null if the file is closed.
  std::unique_ptr<WindowsHandleOStream> m_stream;

public:      // methods
  explicit ExclusiveWriteFilePrivate(std::string_view fname)
    : m_hFile(INVALID_HANDLE_VALUE),
      m_stream()
  {
    EXN_CONTEXT(doubleQuote(fname));

    // `CreateFileA` requires a NUL-terminated string.  (It's possible,
    // even likely, that the caller had a `string` object already, but
    // the cost of an extra copy here is negligible, so I use
    // `string_view` in the interface for flexibility.)
    std::string fnameString(fname);

    // Open and acquire write lock.
    m_hFile = CreateFileA(
      fnameString.c_str(),
      GENERIC_READ | GENERIC_WRITE,      // I can r/w (though I only write).
      FILE_SHARE_READ,                   // Allow others to read the file.
      nullptr,
      CREATE_ALWAYS,                     // Create or truncate.
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

    if (m_hFile == INVALID_HANDLE_VALUE) {
      SystemErrorCode sec = SystemErrorCode::getCurrent();
      if (sec.systemCode() == ERROR_SHARING_VIOLATION) {
        THROW(XExclusiveWriteFileConflict(sec, fnameString));
      }
      else {
        // The file name is already on the context stack.
        THROW(XSysError(sec, "CreateFileA", ""));
      }
    }

    // Make the stream.  Does not take ownership of the handle.
    m_stream = std::make_unique<WindowsHandleOStream>(m_hFile);
  }

  ~ExclusiveWriteFilePrivate() noexcept
  {
    GENERIC_CATCH_BEGIN

    close();

    GENERIC_CATCH_END
  }

  void close()
  {
    if (m_stream) {
      m_stream->flush();
      m_stream.reset();
    }

    if (m_hFile != INVALID_HANDLE_VALUE) {
      // Closing the handle unlocks the file.
      CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
    }
  }

  void selfCheck() const
  {
    if (m_stream) {
      m_stream->selfCheck();
    }
  }
};


#else // not windows


// Close a file descriptor in destructor.
class AutoCloseFD {
public:      // data
  // File descriptor to close, or -1 to disable.
  int m_fd;

public:
  AutoCloseFD()
    : m_fd(-1)
  {}

  ~AutoCloseFD() noexcept
  {
    GENERIC_CATCH_BEGIN

    close();

    GENERIC_CATCH_END
  }

  // Close the descriptor if it is still open.  Throws on error.
  void close()
  {
    if (m_fd >= 0) {
      if (::close(m_fd) < 0) {
        xsyserror("close");
      }
      m_fd = -1;
    }
  }
};


class ExclusiveWriteFilePrivate : public AutoCloseFD {
public:      // data
  // Stream wrapped around the descriptor.  null if the file is closed.
  std::unique_ptr<PosixFDOStream> m_stream;

public:      // methods
  explicit ExclusiveWriteFilePrivate(std::string_view fname)
    : AutoCloseFD(),
      m_stream()
  {
    EXN_CONTEXT_STRING(doubleQuote(fname));

    // `open` requires a NUL-terminated string.
    std::string fnameString(fname);

    // Do not truncate yet, since this call ignores the lock.
    m_fd = open(fnameString.c_str(), O_RDWR | O_CREAT, 0666);
    if (m_fd < 0) {
      // I don't pass `fname` because it's already on the context stack.
      xsyserror("open");
    }

    struct flock fl{};
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;            // Means to lock all bytes.

    if (fcntl(m_fd, F_SETLK, &fl) < 0) {
      // Note: `AutoCloseFD` will close `m_fd`.

      SystemErrorCode sec = SystemErrorCode::getCurrent();

      // POSIX explains that both are possible, so we have to check for
      // both.
      if (sec.systemCode() == EAGAIN || sec.systemCode() == EACCES) {
        THROW(XExclusiveWriteFileConflict(sec, fnameString));
      }
      else {
        THROW(XSysError(sec, "fcntl", ""));
      }
    }

    // Successfully opened and locked, so truncate now.
    if (ftruncate(m_fd, 0)) {
      xsyserror("ftruncate");
    }

    m_stream = std::make_unique<PosixFDOStream>(m_fd);
  }

  ~ExclusiveWriteFilePrivate() noexcept
  {
    GENERIC_CATCH_BEGIN

    close();

    GENERIC_CATCH_END
  }

  // Close everything.  Throw on error.
  void close()
  {
    if (m_stream) {
      m_stream->flush();
      m_stream.reset();
    }

    // Simply closing the file releases the lock.
    AutoCloseFD::close();
  }

  void selfCheck() const
  {
    if (m_stream) {
      m_stream->selfCheck();
    }
  }
};


#endif // !PLATFORM_IS_WINDOWS


// ------------------------ ExclusiveWriteFile -------------------------
ExclusiveWriteFile::ExclusiveWriteFile(std::string_view fname)
  : m_private(new ExclusiveWriteFilePrivate(fname))
{}


ExclusiveWriteFile::~ExclusiveWriteFile() noexcept
{
  m_private.reset();
}


void ExclusiveWriteFile::close()
{
  m_private->close();
}


std::ostream &ExclusiveWriteFile::stream()
{
  return *( m_private->m_stream );
}


void ExclusiveWriteFile::selfCheck() const
{
  m_private->selfCheck();
}


// -------------------- tryCreateExclusiveWriteFile --------------------
ExclusiveWriteFile * NULLABLE tryCreateExclusiveWriteFile(
  std::string &fname /*INOUT*/)
{
  int const maxSuffix = envAsIntOr(100, "EXCLUSIVE_FILE_MAX_SUFFIX");
  for (int suffix = 1; suffix <= maxSuffix; ++suffix) {
    std::string attemptName = fname;
    if (suffix > 1) {
      attemptName = stringb(fname << "." << suffix);
    }

    try {
      ExclusiveWriteFile *ret = new ExclusiveWriteFile(attemptName);
      fname = attemptName;
      return ret;
    }
    catch (XExclusiveWriteFileConflict &x) {
      if (suffix == maxSuffix) {
        xmessage(stringb(
          "Could not create a write-exclusive file based on " <<
          doubleQuote(fname) << " despite trying " << maxSuffix <<
          " suffixes.  The final attempt yielded the error: " << x));
      }
    }
  }

  // This can happen if the envvar is set to 0, effectively disabling
  // creation of a file this way.  But a value of 1 or greater will
  // cause an exception to be thrown if we can't open the file.
  return nullptr;
}


CLOSE_NAMESPACE(smbase)


// EOF
