// temporary-file.h
// `TemporaryFile` class that creates a temporary file holding a string.

#ifndef SMBASE_TEMPORARY_FILE_H
#define SMBASE_TEMPORARY_FILE_H

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// Create and populate a temporary file, deleting it on exit.
class TemporaryFile {
public:      // class data
  // Counter for created file names, used to ensure uniqueness.
  static int s_fileNameCounter;

public:      // instance data
  // The temporary file name.
  std::string m_fname;

public:      // methods
  // This deletes the temporary file.
  ~TemporaryFile();

  // Make a temporary file in the current directory whose name has
  // `fnamePrefix` as a prefix, `fnameSuffix` as a suffix, and contains
  // `contents`.
  //
  // This has a race condition due to separating the file test from
  // creation.  It's fine for tests, and probably ok for non-adversarial
  // production scenarios, but could be exploited adversarially.
  //
  TemporaryFile(std::string const &fnamePrefix,
                std::string const &fnameSuffix,
                std::string const &contents);

  std::string const &getFname() const
    { return m_fname; }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_TEMPORARY_FILE_H
