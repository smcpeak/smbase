// exclusive-write-file.h
// `ExclusiveWriteFile`, to open a file with exclusion for writing.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_EXCLUSIVE_WRITE_FILE_H
#define SMBASE_EXCLUSIVE_WRITE_FILE_H

#include "smbase/exc.h"                          // smbase::XBase
#include "smbase/sm-macros.h"                    // NO_OBJECT_COPIES, OPEN_NAMESPACE, NULLABLE
#include "smbase/std-string-view-fwd.h"          // std::string_view
#include "smbase/system-error-code.h"            // smbase::SystemErrorCode

#include <iosfwd>                                // std::ostream
#include <memory>                                // std::unique_ptr


OPEN_NAMESPACE(smbase)


// Platform-specific private implementation details, defined in the .cc
// file.
class ExclusiveWriteFilePrivate;


/* Open a file for writing with a discretionary write lock.

   While this object exists, other processes are allowed to read the
   file contents.  However, no other process can write to the file *if*
   it uses this class to do so (this is a "discretionary" lock).  An
   attempt to do so will fail immediately (not block).

   Whether other processes can write to the file *without* using this
   class is unspecified, and in fact depends on the platform.

   The initial intended purpose is to be able to open a log file without
   having other instances of the same program stomp on the log.
*/
class ExclusiveWriteFile {
  NO_OBJECT_COPIES(ExclusiveWriteFile);

private:     // data
  // Lock data.  Never null.
  std::unique_ptr<ExclusiveWriteFilePrivate> m_private;

public:      // methods
  // Open `fname` for writing.  Create the file if needed, and truncate
  // it if it already exists.
  //
  // If the file is already locked, this does *not* block, instead
  // throwing `XExclusiveWriteFileConflict`.
  //
  // Other failures throw `XSysError`.
  explicit ExclusiveWriteFile(std::string_view fname);

  // This will try to flush, close the file and release the lock, but
  // does not have a way to communicate failures.
  ~ExclusiveWriteFile() noexcept;

  // Flush, close and unlock the file if it is currently locked.  This
  // will throw an exception on error.
  void close();

  // Get the stream to write to that goes to the file.  The file is
  // effectively open in binary mode, so this does not do CRLF
  // translation.
  std::ostream &stream();

  // Assert invariants.
  void selfCheck() const;
};


// Thrown when `ExclusiveWriteFile` cannot open the file for the
// specific reason that another process has it open.
//
// We do not rely on interpreting `XSysError` because the meaning of the
// possible error codes in this context is potentially ambiguous with
// their meaning in other contexts.  Thus, we use a distinct exception
// class.
//
class XExclusiveWriteFileConflict : public XBase {
public:      // data
  // The platform-specific code resulting from the lock attempt.
  SystemErrorCode m_systemErrorCode;

  // The name of the file we were trying to lock.
  std::string m_fname;

public:      // methods
  // ---- create-tuple-class: declarations for XExclusiveWriteFileConflict
  /*AUTO_CTC*/ explicit XExclusiveWriteFileConflict(SystemErrorCode const &systemErrorCode, std::string const &fname);
  /*AUTO_CTC*/ XExclusiveWriteFileConflict(XExclusiveWriteFileConflict const &obj) noexcept;
  /*AUTO_CTC*/ XExclusiveWriteFileConflict &operator=(XExclusiveWriteFileConflict const &obj) noexcept;

  // XBase methods.
  //
  // This message does not include the file name because I expect the
  // file name to already be in the exception context stack, making
  // including it here redundant.  However, the class still stores a
  // copy of the name so that calling code can unambiguously retrieve
  // it.
  virtual std::string getConflict() const override;
};


// Attempt to open, with exclusive write access, a file with a name
// based on `fname`.  Keep trying up to (by default) 100 variations.
// Upon success, return an owner pointer to the file object and set
// `fname` to the adjusted name.  Throw an exception if all attempts
// fail.
//
// Setting the envvar `EXCLUSIVE_FILE_MAX_SUFFIX` will adjust the number
// of attempts, and setting it to 0 disables such file creation
// entirely, causing this function to return null.
ExclusiveWriteFile * NULLABLE tryCreateExclusiveWriteFile(
  std::string &fname /*INOUT*/);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_EXCLUSIVE_WRITE_FILE_H
