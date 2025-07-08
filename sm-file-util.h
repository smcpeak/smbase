// sm-file-util.h
// smbase file utilities.

// There are also file utilities in 'nonport', but that module is a
// haphazard mix of stuff with poor error handling, so I have decided
// to start over.

#ifndef SM_FILE_UTIL_H
#define SM_FILE_UTIL_H

// smbase
#include "array.h"                     // ArrayStack
#include "sm-iostream.h"               // ostream
#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "sm-noexcept.h"               // NOEXCEPT
#include "sm-override.h"               // OVERRIDE
#include "str.h"                       // string
#include "stringset.h"                 // StringSet

// libc++
#include <optional>                    // std::optional
#include <vector>                      // std::vector


// Structured representation of a file name.
//
// Conventionally, file names are represented as strings, but of course
// that creates various problems for reliably interpreting and
// manipulating them.  This class captures the concept of a file name
// in a more abstract, structured way.  It attempts to provide a union
// of the file name features available on POSIX and Windows:
//
//   - optional file system designator (e.g., "c:")
//   - absolute versus relative path indicator ("leading slash")
//   - sequence of path components (non-empty strings)
//   - optional "trailing slash", sometimes used to indicate that the
//     name is intended to refer to a directory rather than file
//
// This class does *not* associate a particular path separator
// character (e.g., forward slash versus backward slash) with each path
// component.  Consequently, it loses some information present in the
// string representation.
//
// Since all of the elements are effectively optional, there is an
// "empty" file name, corresponding to the empty string.
//
// A file name is immutable once constructed.
class SMFileName {
private:     // data
  // Optional file system designator.  Empty if there is none.  POSIX
  // file names always lack this.  For a Windows file name like
  // "C:/Windows", the file system is "C:" (two-character string).
  // For a UNC path like "//server/share", the file system is "/".
  string m_fileSystem;

  // True if this path is absolute, i.e., has a leading slash.  This is
  // true for UNC paths, and for the path "/".  It is false for "".
  bool m_isAbsolute;

  // Possibly empty sequence of path component strings.  It is empty
  // for paths like "", "/", "c:", and "C:/".  But "." has a single
  // path component, ".".  Each path component is a non-empty string.
  // A path like "a//b" is treated the same as "a/b".
  ArrayStack<string> m_pathComponents;

  // True if the file name has at least one component and ends with a
  // directory separator.  That normally means it is intended to
  // designate a directory.
  //
  // Invariant: !m_trailingSlash || hasPathComponents()
  bool m_trailingSlash;

public:      // types
  // Specification of how to convert names to and from strings.
  enum Syntax {
    // POSIX file name.  File systems are not recognized during parsing,
    // so "c:/windows" has two path components, "c:" and "windows".
    // Only forward slash is recognized as a path separator.
    S_POSIX,

    // Windows file name.  Forward slash and backslash are both path
    // separators, and file systems are recognized.  When printing,
    // *forward* slashes are used for path separators.  (Windows
    // recognizes both, and that improves system interoperability.)
    //
    // During parsing letter case is retained, even though most Windows
    // file systems are case-insensitive.
    S_WINDOWS,

    // Equivalent to S_POSIX or S_WINDOWS, depending on the platform
    // this code is running on.
    S_NATIVE,

    NUM_SYNTAXES
  };

private:     // unimplemented
  // This does not exist because file names are immutable.
  SMFileName& operator= (SMFileName const &obj);

public:      // data
  // Construct an empty name.
  SMFileName();

  // Parse a string as a path name.  If 'path' has an embedded NUL,
  // parsing stops there, as if it was the end of the string.  Bytes
  // with values outside [0,126] are regarded as valid but with no
  // special significance.  This is compatible with both Latin-1 and
  // UTF-8 encodings.
  explicit SMFileName(string const &path, Syntax syntax = S_NATIVE);

  // Construct from components.
  SMFileName(string fileSystem, bool isAbsolute,
             ArrayStack<string> const &pathComponents, bool trailingSlash);

  SMFileName(SMFileName const &obj);

  ~SMFileName();

  // Assert invariants.
  void selfCheck() const;

  // True if all components are equal, including letter case.
  bool operator== (SMFileName const &obj) const;
  NOTEQUAL_OPERATOR(SMFileName)

  // Retrieve components.
  string getFileSystem() const { return m_fileSystem; }
  bool isAbsolute() const { return m_isAbsolute; }
  void getPathComponents(ArrayStack<string> /*OUT*/ &pathComponents) const;
  bool hasTrailingSlash() const { return m_trailingSlash; }

  // True if there is at least one path component, which is a
  // requirement for having a trailing slash.
  bool hasPathComponents() const { return m_pathComponents.isNotEmpty(); }

  // Create new names by replacing components.
  SMFileName withFileSystem(string const &newFileSystem) const;
  SMFileName withIsAbsolute(bool newIsAbsolute) const;
  SMFileName withPathComponents(ArrayStack<string> const &newPathComponents) const;
  SMFileName withTrailingSlash(bool newTrailingSlash) const;

  // Render as a string.
  string toString(Syntax syntax = S_NATIVE) const;

  // Get just the path components as a string separated by forward slashes.
  string getPathComponentsString() const;

  // True if the string representation ends with a path separator,
  // either because it is absolute and has no components, or because it
  // has a trailing slash.
  bool endsWithPathSeparator() const;

  // True if 'syntax' is S_NATIVE and we are running under Windows,
  // or is S_WINDOWS.
  static bool isWindowsSyntax(Syntax syntax);

  // True if 'c' is regarded as a path separator in 'syntax'.
  static bool isPathSeparator(unsigned char c, Syntax syntax);
};


// Collection of file system utilities.
//
// These are packaged as a class with virtual methods mainly to allow
// them to be replaced with mock implementations for testing.  Code
// that does not care about that can just make an instance of
// SMFileUtil itself in the ordinary way.
//
// TODO: This class has become a mishmash of several ideas:
//
//   1. Queries about file names themselves, optionally (via virtual
//      functions) applying Windows or POSIX semantics, which overlaps
//      with the SMFileName class, above.
//
//   2. Manipulation of file names, such as joining them, which
//      SMFileName does not do but probably should.
//
//   3. Queries to execute against the local file system via system
//      calls, which is a form of interprocess communication.
//
// I think 3 should be split from 1 and 2, and then something done to
// resolve the tension between (1,2) and SMFileName.
//
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

    // Write as a string for debug purposes.
    string asString() const;
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

  // True if 'name' has at least one character, and the last character
  // 'isDirectorySeparator'.
  bool endsWithDirectorySeparator(string const &name);

  // Given an ostensible directory name, if it does not end with a
  // directory separator, append '/' and return that.
  string ensureEndsWithDirectorySeparator(string const &dir);

  // Remove a trailing separator from a directory unless it is "/" or,
  // on Windows, "<letter>:<separator>".
  string stripTrailingDirectorySeparator(string const &dir);

  // True if the given path is absolute.  On unix, an absolute path
  // starts with '/'.  On Windows, it starts with '//' (UNC path) or
  // "<letter>:/", or the equivalent with backslash.
  virtual bool isAbsolutePath(string const &path);

  // Convert 'path' to an absolute path.  If it is relative, we
  // prepend 'currentDirectory()'.
  virtual string getAbsolutePath(string const &path);

  // Return true if 'path' is absolute and names an existing entity
  // (file, directory, etc.) on disk.  Throws XSysError on permission
  // errors or the like.
  virtual bool absolutePathExists(string const &path);

  // Like above, except it specifically has to be an ordinary file.
  // Throws XSysError on permission errors or the like.
  virtual bool absoluteFileExists(string const &path);

  // True if 'path' names a directory.  Relative paths are relative to
  // the current working directory.  Throws XSysError on permission
  // errors or the like.
  virtual bool directoryExists(string const &path);

  // Get the file kind, or FK_NONE if it does not exist.  Relative paths
  // are relative to the current working directory.  Throws XSysError on
  // permission errors or the like.
  virtual FileKind getFileKind(string const &path);

  // True if 'path' exists, i.e., its file kind is not `FK_NONE`.
  virtual bool pathExists(string const &path);

  // Create 'path' and any needed parents if it does not already exist.
  // If it, or any parent, already exists but is not a directory, throw
  // 'XSysError' with reason R_ALREADY_EXISTS.  Any other problem also
  // causes 'XSysError' to be thrown.  If no exception is thrown, then
  // after this call, the directory exists.  A trailing directory
  // separator on 'path' is ignored.
  virtual void createDirectoryAndParents(string const &path);

  // True if 'path' exists, but the current user does not have write
  // permission for it.  This does not throw; it returns false if the
  // file does not exist or we cannot determine whether it is read-only.
  virtual bool isReadOnly(string const &path) NOEXCEPT;

  // Return prefix+suffix, except if neither is empty, add a directory
  // separator if none is present, and remove an extra trailing
  // directory separator from 'prefix'.
  virtual string joinFilename(string const &prefix,
                                       string const &suffix);

  // Like 'joinFilename', except if 'suffix' is absolute, then return it
  // as-is.  The idea is to treat 'suffix' as being relative to 'prefix'
  // unless it is absolute already.
  string joinIfRelativeFilename(string const &prefix,
                                string const &suffix);

  // Read the contents of 'fname' in binary mode, returning the entire
  // thing as a vector.  Throw XSysError on error.
  virtual std::vector<unsigned char> readFile(string const &fname);

  // Write 'bytes' into 'fname' in binary mode.  Throw XSysError on
  // error.
  virtual void writeFile(string const &fname,
                         std::vector<unsigned char> const &bytes);

  // Read the contents of `fname`, in binary mode, and return the result
  // as a string.
  virtual string readFileAsString(string const &fname);

  // Write `contents` to `fname` in binary mode.
  virtual void writeFileAsString(string const &fname,
                                 string const &contents);

  // Get the names of entries in 'directory'.  If an error is
  // encountered, throw XSysError (syserr.h).  The entries are not
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

  // If 'inputPath' has any occurrences of "." or "..", collapse them
  // as much as possible.  The result may have a sequence of "../" at
  // the start, or consist entirely of ".", or have neither.
  string collapseDots(string const &inputPath);

  // Atomically rename 'oldPath' to 'newPath', replacing the latter if
  // it exists.  This is meant to act like POSIX 'rename' even on
  // Windows using MSVCRT.  It refuses to work on directories.
  void atomicallyRenameFile(string const &oldPath, string const &newPath);

  // Return a process ID suitable for use in file name generation.
  virtual int getProcessID();

  // Create a file name like "$dir/$prefix.$pid.$n.tmp" and does not
  // already exist.
  string createUniqueTemporaryFname(
    string const &dir,
    string const &prefix,
    int maxAttempts = 1000);

  // Write `contents` to `fname` (as binary, i.e., without line ending
  // translation).  Do this atomically.
  void atomicallyWriteFileAsString(
    string const &fname, string const &contents);

  // Delete 'path'.  This is basically POSIX 'remove' except using
  // exceptions to communicate errors.  This includes the case of the
  // file not existing.
  void removeFile(string const &path);

  // Like `removeFile`, but do nothing if `path` does not exist.
  void removeFileIfExists(string const &path);

  // Update the modification time of 'path' or create it if it does not
  // exist.  Return false on error, but there are no details.
  bool touchFile(string const &path);
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
  // For `windowsPathSemantics`.  Initially false for compatibility with
  // older code.
  std::optional<bool> m_windowsPathSemantics;

  // For `pathExists`.  Initially an existing but empty map for
  // compatiblity with older code.
  std::optional<StringSet> m_existingPaths;

  // For `getProcessID`.  Initially unset.
  std::optional<int> m_pid;

  // For `writeFile`.  Initially unset.
  std::optional<std::size_t> m_injectFailureAfterNBytes;

public:      // funcs
  TestSMFileUtil();
  ~TestSMFileUtil();

  // Reset all data members to absent so all functions behave like the
  // superclass versions.
  void resetAll();

  // If `m_windowsPathSemantics` is set, returns that.  Otherwise, calls
  // the superclass function.
  virtual bool windowsPathSemantics() OVERRIDE;

  // If `m_existingPaths` is set, return true iff `path` is in it.
  // Otherwise, call the superclass function.
  virtual bool pathExists(string const &path) OVERRIDE;

  // If `m_pid` is set, return it.  Otherwise, call the superclass
  // function.
  virtual int getProcessID() OVERRIDE;

  // If `m_injectFailureAfterNBytes` is set, and `bytes` is that size or
  // larger, then write that size, then throw `XFatal`, simulating a
  // write failure.  Otherwise, call the superclass function.
  virtual void writeFile(string const &fname,
                         std::vector<unsigned char> const &bytes) OVERRIDE;
};


#endif // SM_FILE_UTIL_H
