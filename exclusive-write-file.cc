// exclusive-write-file.cc
// Code for `exclusive-write-file` module.

#include "exclusive-write-file.h"                // this module

#include "smbase/exc.h"                          // EXN_CONTEXT
#include "smbase/sm-windows.h"                   // CreateFileA, etc.
#include "smbase/syserr.h"                       // xsyserror
#include "smbase/windows-handle-ostream.h"       // smbase::WindowsHandleOstream

#include <iostream>                              // std::ostream
#include <memory>                                // std::unique_ptr
#include <string>                                // std::string
#include <string_view>                           // std::string_view

using namespace smbase;


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
    EXN_CONTEXT(fname);

    // `CreateFileA` requires a NUL-terminated string.  (It's possible,
    // even likely, that the caller had a `string` object already, but
    // the cost of an extra copy here is negligible, so I use
    // `string_view` in the interface for flexibility.)
    std::string fnameString(fname);

    // Open.
    m_hFile = CreateFileA(
      fnameString.c_str(),
      #if 1
        GENERIC_READ | GENERIC_WRITE,      // I can r/w (though I only write).
        FILE_SHARE_READ,                   // Allow others to read the file.
      #else
        GENERIC_WRITE,
        0,
      #endif
      nullptr,
      CREATE_ALWAYS,                     // Create or truncate.
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

    if (m_hFile == INVALID_HANDLE_VALUE) {
      xsyserror("Failed to open for locking");
    }

    #if 0
    // Lock.
    OVERLAPPED ov = {};
    if (!LockFileEx(
           m_hFile,
           // TODO: Experiment with this.
           LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
           //LOCKFILE_FAIL_IMMEDIATELY,
           //LOCKFILE_EXCLUSIVE_LOCK,
           0,                  // Reserved.
           MAXDWORD,           // Low 32 bits of number of bytes to lock.
           MAXDWORD,           // High 32 bits of number of bytes to lock.
           &ov)) {
      std::string msg = GetLastErrorAsString();

      CloseHandle(m_hFile);
      throw std::runtime_error(fname + ": Failed to lock file: " + msg);
    }
    #endif

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
      #if 0
      OVERLAPPED ov = {};
      if (!UnlockFileEx(
             m_hFile,
             0,                  // Reserved.
             MAXDWORD,           // Low 32 bits.
             MAXDWORD,           // High 32 bits.
             &ov)) {
        std::string msg = GetLastErrorAsString();

        // If we failed to unlock it once, reset the handle anyway so we
        // do not try again during the dtor (if we are not in it now).
        m_hFile = INVALID_HANDLE_VALUE;

        throw std::runtime_error("Failed to unlock file: " + msg);
      }
      #endif

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


#endif


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


// EOF
