// sm-file-util.h
// smbase file utilities.

// There are also file utilities in 'nonport', but that module is a
// haphazard mix of stuff with poor error handling, so I have decided
// to start over.

// At some point I may introduce a structured file name class, but for
// now I will just use strings to hold file names.

#ifndef SM_FILE_UTIL_H
#define SM_FILE_UTIL_H

#include "array.h"                     // ArrayStack
#include "macros.h"                    // NO_OBJECT_COPIES
#include "sm-override.h"               // OVERRIDE
#include "str.h"                       // string
#include "stringset.h"                 // StringSet


// Collection of file system utilities.
//
// These are packaged as a class with virtual methods mainly to allow
// them to be replaced with mock implementations for testing.  Code
// that does not care about that can just make an instance of
// SMFileUtil itself in the ordinary way.
class SMFileUtil {
  NO_OBJECT_COPIES(SMFileUtil);

public:      // funcs
  SMFileUtil();
  virtual ~SMFileUtil();

  // True if the target platform uses Windows path semantics, for
  // example using backslash as a possible path separator.  False
  // if using POSIX paths.
  virtual bool windowsPathSemantics();

  // Return a string with all path separators as forward slashes.
  virtual string normalizePathSeparators(string const &s);

  // Return the current directory as an absolute path name.
  virtual string currentDirectory();

  // True if 'c' is considered a directory separator for the platform.
  virtual bool isDirectorySeparator(char c);

  // True if the given path is absolute.  On unix, an absolute path
  // starts with '/'.  On Windows, it starts with '//' (UNC path) or
  // "<letter>:/", or the equivalent with backslash.
  virtual bool isAbsolutePath(string const &path);

  // Convert 'path' to an absolute path.  If it is relative, we
  // prepend 'currentDirectory()'.
  virtual string getAbsolutePath(string const &path);

  // Return true if 'path' is absolute and names an existing entity
  // (file, directory, etc.) on disk.
  virtual bool absolutePathExists(string const &path);

  // Like above, except it specifically has to be an ordinary file
  // (or a symlink? not sure).
  virtual bool absoluteFileExists(string const &path);

  // Return prefix+suffix, except if neither is empty, add a directory
  // separator if none is present, and remove an extra trailing
  // directory separator from 'prefix'.
  virtual string joinFilename(string const &prefix,
                              string const &suffix);

  // Get the entries in 'directory'.  If an error is encountered, throw
  // xSysError (syserr.h).  The entries are not guaranteed to be
  // returned in any particular order.  They may include "." and ".." if
  // they exist in the given directory.
  virtual void getDirectoryEntries(ArrayStack<string> /*OUT*/ &entries,
                                   string const &directory);

  // Split 'inputPath' into two strings, 'dir' and 'base', such that:
  //
  //   * 'dir' + 'base' == 'inputPath'.
  //   * 'base' has no characters for which 'isDirectorySeparator()'
  //     is true.
  //   * 'base' is the longest string such that the above are true.
  //
  void splitPath(string /*OUT*/ &dir, string /*OUT*/ &base,
                 string const &inputPath);

  // Get the 'dir' output of 'splitPath'.
  string splitPathDir(string const &inputPath);

  // Get the 'base' output of 'splitPath'.
  string splitPathBase(string const &inputPath);
};


// Variant of SMFileUtil that returns specific values in response to
// certain queries.  This is only meant for use in test code.
class TestSMFileUtil : public SMFileUtil {
public:      // data
  // For 'absolutePathExists'.
  StringSet m_existingPaths;

public:      // funcs
  TestSMFileUtil() {}
  ~TestSMFileUtil() {}

  // Returns false.
  virtual bool windowsPathSemantics() OVERRIDE;

  // Returns true iff 'path' is in 'm_existingPaths'.
  virtual bool absolutePathExists(string const &path) OVERRIDE;
};


#endif // SM_FILE_UTIL_H
