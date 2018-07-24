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
#include "sm-iostream.h"               // ostream
#include "sm-noexcept.h"               // NOEXCEPT
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

public:      // types
  // Kinds of files.
  enum FileKind {
    FK_NONE,                 // Not a file.
    FK_REGULAR,              // Regular file.
    FK_DIRECTORY,            // Directory.
    FK_OTHER,                // Something else I haven't categorized.

    NUM_FILE_KINDS
  };

  // Information about a directory entry.
  class DirEntryInfo {
  public:    // data
    // Name of the directory entry, not including any path.
    string m_name;

    // What sort of file it is.
    FileKind m_kind;

  public:
    DirEntryInfo(string const &name, FileKind kind);
    DirEntryInfo(DirEntryInfo const &obj);
    DirEntryInfo();      // Empty name, FK_NONE.
    ~DirEntryInfo();

    DirEntryInfo& operator= (DirEntryInfo const &obj);

    // strcmp-style result.  Lexicographic comparison, name first.
    int compareTo(DirEntryInfo const &obj) const;
    static int compare(DirEntryInfo const *a, DirEntryInfo const *b);
  };

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

  // Given an ostensible directory name, if it does not end with a
  // directory separator, append '/' and return that.
  string ensureEndsWithDirectorySeparator(string const &dir);

  // Remove a trailing separator from a directory unless it is "/" or,
  // on Windows, "<letter>:<separator".
  string stripTrailingDirectorySeparator(string const &dir);

  // True if the given path is absolute.  On unix, an absolute path
  // starts with '/'.  On Windows, it starts with '//' (UNC path) or
  // "<letter>:/", or the equivalent with backslash.
  virtual bool isAbsolutePath(string const &path);

  // Convert 'path' to an absolute path.  If it is relative, we
  // prepend 'currentDirectory()'.
  virtual string getAbsolutePath(string const &path);

  // Return true if 'path' is absolute and names an existing entity
  // (file, directory, etc.) on disk.  Throws xSysError on permission
  // errors or the like.
  virtual bool absolutePathExists(string const &path);

  // Like above, except it specifically has to be an ordinary file.
  // Throws xSysError on permission errors or the like.
  virtual bool absoluteFileExists(string const &path);

  // True if 'path' names a directory.  Relative paths are relative to
  // the current working directory.  Throws xSysError on permission
  // errors or the like.
  virtual bool directoryExists(string const &path);

  // Get the file kind, or FK_NONE if it does not exist.  Relative paths
  // are relative to the current working directory.  Throws xSysError on
  // permission errors or the like.
  virtual FileKind getFileKind(string const &path);

  // True if 'path' exists, but the current user does not have write
  // permission for it.  This does not throw; it returns false for
  // conditions that prevent determining the file's existence.
  virtual bool isReadOnly(string const &path) NOEXCEPT;

  // Return prefix+suffix, except if neither is empty, add a directory
  // separator if none is present, and remove an extra trailing
  // directory separator from 'prefix'.
  virtual string joinFilename(string const &prefix,
                              string const &suffix);

  // Get the names of entries in 'directory'.  If an error is
  // encountered, throw xSysError (syserr.h).  The entries are not
  // guaranteed to be returned in any particular order.  They may
  // include "." and ".." if they exist in the given directory.
  virtual void getDirectoryNames(ArrayStack<string> /*OUT*/ &entries,
                                 string const &directory);

  // Get names and file kinds.  This may be more expensive than just
  // getting the names.
  virtual void getDirectoryEntries(
    ArrayStack<DirEntryInfo> /*OUT*/ &entries, string const &directory);

  // Same, but return in alphabetical order.
  void getSortedDirectoryEntries(
    ArrayStack<DirEntryInfo> /*OUT*/ &entries, string const &directory);

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


// Return a string like "FK_REGULAR".
char const *toString(SMFileUtil::FileKind kind);

inline ostream& operator<< (ostream &os, SMFileUtil::FileKind kind)
{
  return os << toString(kind);
}

inline stringBuilder& operator<< (stringBuilder &sb, SMFileUtil::FileKind kind)
{
  return sb << toString(kind);
}


// Variant of SMFileUtil that returns specific values in response to
// certain queries.  This is only meant for use in test code.
class TestSMFileUtil : public SMFileUtil {
public:      // data
  // For 'windowsPathSemantics'.
  bool m_windowsPathSemantics;

  // For 'absolutePathExists'.
  StringSet m_existingPaths;

public:      // funcs
  TestSMFileUtil() : m_windowsPathSemantics(false) {}
  ~TestSMFileUtil() {}

  // Returns 'm_windowsPathSemantics'.
  virtual bool windowsPathSemantics() OVERRIDE;

  // Returns true iff 'path' is in 'm_existingPaths'.
  virtual bool absolutePathExists(string const &path) OVERRIDE;
};


#endif // SM_FILE_UTIL_H
