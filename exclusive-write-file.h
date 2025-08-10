// exclusive-write-file.h
// `ExclusiveWriteFile`, to open a file with exclusion for writing.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_EXCLUSIVE_WRITE_FILE_H
#define SMBASE_EXCLUSIVE_WRITE_FILE_H

#include "smbase/exc.h"                          // smbase::XBase
#include "smbase/sm-macros.h"                    // NO_OBJECT_COPIES
#include "smbase/std-string-view-fwd.h"          // std::string_view

#include <iosfwd>                                // std::ostream
#include <memory>                                // std::unique_ptr


// Platform-specific private implementation details, defined in the .cc
// file.
class ExclusiveWriteFilePrivate;


/* Open a file for writing, creating it if needed, and truncating it if
   it already exists.

   Writing does not do any line ending translation.

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
  // Open `fname` for writing with a discretionary write lock.  Throw an
  // exception on failure; if another process has it open, throw
  // `XExclusiveWriteFileConflict` specifically.
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
DEFINE_XMESSAGE_SUBCLASS(XExclusiveWriteFileConflict);


#endif // SMBASE_EXCLUSIVE_WRITE_FILE_H
