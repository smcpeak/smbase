// exclusive-write-file-fwd.h
// Forward decls for `exclusive-write-file.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_EXCLUSIVE_WRITE_FILE_FWD_H
#define SMBASE_EXCLUSIVE_WRITE_FILE_FWD_H

namespace smbase {
  class ExclusiveWriteFile;
  class XExclusiveWriteFileConflict;

  // There is also ExclusiveWriteFilePrivate but since that is private
  // to the module, it is not advertised here.
}

#endif // SMBASE_EXCLUSIVE_WRITE_FILE_FWD_H
