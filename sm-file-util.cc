// sm-file-util.cc
// code for sm-file-util.h

#include "sm-file-util.h"              // this module

// smbase
#include "array.h"                     // Array
#include "strtokp.h"                   // StrtokParse
#include "syserr.h"                    // xsyserror

// libc++
#include <algorithm>                   // std::max

// libc
#include <errno.h>                     // errno
#include <string.h>                    // strlen

// POSIX fragment evidently portable enough to unconditionally include,
// at least for Windows (mingw, cygwin) and Linux.
#include <dirent.h>                    // opendir, readdir, etc.
#include <sys/stat.h>                  // stat

// Use the Windows API?  I want an easy way to switch it so I can test
// both ways under cygwin, although my intent *is* to use it normally
// whenever running on Windows.  (Using the POSIX API on cygwin is
// basically a quick, moderately accurate way to test if the code will
// work on Linux.)
//#define SM_FILE_UTIL_USE_WINDOWS_API 0
#ifndef SM_FILE_UTIL_USE_WINDOWS_API
#  ifdef __WIN32__
#    define SM_FILE_UTIL_USE_WINDOWS_API 1
#  else
#    define SM_FILE_UTIL_USE_WINDOWS_API 0
#  endif
#endif

#if SM_FILE_UTIL_USE_WINDOWS_API
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>                 // GetCurrentDirectoryA
#else
#  include <limits.h>                  // PATH_MAX
#  include <stdlib.h>                  // getcwd
#  include <unistd.h>                  // pathconf
#endif


static char const * const s_fileKindNames[] = {
  "FK_NONE",
  "FK_REGULAR",
  "FK_DIRECTORY",
  "FK_OTHER",
};

char const *toString(SMFileUtil::FileKind kind)
{
  ASSERT_TABLESIZE(s_fileKindNames, SMFileUtil::NUM_FILE_KINDS);
  if ((unsigned)kind < TABLESIZE(s_fileKindNames)) {
    return s_fileKindNames[kind];
  }
  else {
    return "unknown FileKind";
  }
}



// ---------------- SMFileUtil::DirEntryInfo -----------------
SMFileUtil::DirEntryInfo::DirEntryInfo(string const &name, FileKind kind)
  : m_name(name),
    m_kind(kind)
{}


SMFileUtil::DirEntryInfo::DirEntryInfo(DirEntryInfo const &obj)
  : DMEMB(m_name),
    DMEMB(m_kind)
{}


SMFileUtil::DirEntryInfo::DirEntryInfo()
  : m_name(""),
    m_kind(FK_NONE)
{}


SMFileUtil::DirEntryInfo::~DirEntryInfo()
{}


SMFileUtil::DirEntryInfo&
SMFileUtil::DirEntryInfo::operator= (SMFileUtil::DirEntryInfo const &obj)
{
  CMEMB(m_name);
  CMEMB(m_kind);
  return *this;
}


// ----------------------- SMFileUtil ------------------------
bool SMFileUtil::windowsPathSemantics()
{
#ifdef __WIN32__
  return true;
#else
  return false;
#endif
}


SMFileUtil::SMFileUtil()
{}

SMFileUtil::~SMFileUtil()
{}


string SMFileUtil::normalizePathSeparators(string const &s)
{
  if (!windowsPathSemantics()) {
    return s;
  }

  int len = s.length();
  Array<char> arr(len+1);
  for (int i=0; i < len; i++) {
    if (s[i] == '\\') {
      arr[i] = '/';
    }
    else {
      arr[i] = s[i];
    }
  }
  arr[len] = 0;
  return string(arr.ptr());
}


string SMFileUtil::currentDirectory()
{
#if SM_FILE_UTIL_USE_WINDOWS_API
  // Get length of current directory, in characters.
  DWORD length = GetCurrentDirectoryA(0, NULL);
  if (!length) {
    xsyserror("GetCurrentDirectoryA(0)");
  }
  xassert(length > 0);

  // Get the actual string.
  for (int attempts=0; attempts < 10; attempts++) {
    Array<char> a(length+1);
    DWORD res = GetCurrentDirectoryA(length+1, a.ptr());
    if (!res) {
      xsyserror("GetCurrentDirectoryA");
    }
    xassert(res > 0);
    if (res > length) {
      // Somehow it's still not enough, try again.
      length = res;
      continue;
    }

    string ret(a.ptr());
    xassert(isAbsolutePath(ret));
    return ret;
  }

  xfailure("GetCurrentDirectory kept returning larger values!");
  return string("");       // silence warning

#else
  errno = 0;
  long maxSize = pathconf(".", _PC_PATH_MAX);
  if (maxSize < 0) {
    if (errno != 0) {
      xsyserror("pathconf(_PC_PATH_MAX)");
    }
    // There is no system limit.  Let's just try something big.
    maxSize = std::max(20000, PATH_MAX);
  }

  Array<char> a(maxSize+1);
  if (!getcwd(a.ptr(), maxSize+1)) {
    xsyserror("getcwd");
  }

  return string(a.ptr());

#endif
}


bool SMFileUtil::isDirectorySeparator(char c)
{
  if (c == '/') {
    return true;
  }
  if (windowsPathSemantics() && c == '\\') {
    return true;
  }
  return false;
}


string SMFileUtil::ensureEndsWithDirectorySeparator(string const &dir)
{
  if (dir.empty() || !isDirectorySeparator(dir[dir.length()-1])) {
    return stringb(dir << '/');
  }
  else {
    return dir;
  }
}


string SMFileUtil::stripTrailingDirectorySeparator(string const &dir)
{
  int len = dir.length();
  if (len <= 1) {
    // Empty or "/" or just some letter.
    return dir;
  }

  if (this->windowsPathSemantics() &&
      len == 3 &&
      dir[1] == ':' &&
      isDirectorySeparator(dir[2])) {
    // Windows absolute path.
    return dir;
  }

  if (isDirectorySeparator(dir[len-1])) {
    // Strip final separator.
    return dir.substring(0, len-1);
  }

  return dir;
}


bool SMFileUtil::isAbsolutePath(string const &path)
{
  if (path[0] == 0) {
    return false;
  }

  if (windowsPathSemantics()) {
    if (isDirectorySeparator(path[0]) && isDirectorySeparator(path[1])) {
      // Absolute UNC path.
      return true;
    }

    if (path[1] == ':' && isDirectorySeparator(path[2])) {
      // Drive letter and absolute path.
      return true;
    }
  }
  else {
    if (isDirectorySeparator(path[0])) {
      return true;
    }
  }

  return false;
}


string SMFileUtil::getAbsolutePath(string const &path)
{
  if (isAbsolutePath(path)) {
    return path;
  }

  // There is a bug here.  On Windows, a path like "d:foo" is legal,
  // being composed of a drive letter and a relative path.  Every
  // process has a current working directory for each drive letter.
  // However, I don't know how to get ahold of it using the Windows
  // API!  GetCurrentDirectory just returns one thing.
  //
  // If 'path' is "d:foo", we will return something like
  // "d:/some/path/d:foo", which is wrong, but oh well.

  string cwd = currentDirectory();

  if (windowsPathSemantics()) {
    if (isDirectorySeparator(path[0])) {
      // We have a path that is absolute except it is missing the
      // drive letter or UNC share.  Get that from 'cwd'.
      if (cwd[1] == ':') {
        return cwd.substring(0, 2) & path;
      }

      if (isDirectorySeparator(cwd[0]) && isDirectorySeparator(cwd[1])) {
        // Get the UNC share name.
        StrtokParse tok(string(cwd.c_str()+2), "\\/");
        if (tok.tokc() >= 2) {
          stringBuilder sb;
          sb << "//" << tok.tokv(0) << '/' << tok.tokv(1) << path;
          return sb;
        }
      }

      // Not sure what it is, just fall through.
    }
  }

  return cwd & "/" & path;
}


bool SMFileUtil::absolutePathExists(string const &path)
{
  if (!isAbsolutePath(path)) {
    return false;
  }

  return this->getFileKind(path) != FK_NONE;
}


bool SMFileUtil::absoluteFileExists(string const &path)
{
  if (!isAbsolutePath(path)) {
    return false;
  }

  return this->getFileKind(path) == FK_REGULAR;
}


bool SMFileUtil::directoryExists(string const &path)
{
  return this->getFileKind(path) == FK_DIRECTORY;
}


SMFileUtil::FileKind SMFileUtil::getFileKind(string const &path)
{
  if (path.empty()) {
    return FK_NONE;
  }

  struct stat st;
  if (0!=stat(path.c_str(), &st)) {
    if (errno != ENOENT) {
      xsyserror("stat", path);
    }
    return FK_NONE;
  }
  else if (S_ISDIR(st.st_mode)) {
    return FK_DIRECTORY;
  }
  else if (S_ISREG(st.st_mode)) {
    return FK_REGULAR;
  }
  else {
    return FK_OTHER;
  }
}


string SMFileUtil::joinFilename(string const &prefix,
                                string const &suffix)
{
  if (prefix.isempty()) {
    return suffix;
  }
  if (suffix.isempty()) {
    return prefix;
  }

  if (!this->isDirectorySeparator(suffix[0]) &&
      !this->isDirectorySeparator(prefix[prefix.length()-1])) {
    // Add a separator.
    return stringb(prefix << '/' << suffix);
  }

  if (this->isDirectorySeparator(suffix[0]) &&
      this->isDirectorySeparator(prefix[prefix.length()-1])) {
    // Remove a separator.
    return stringb(prefix.substring(0, prefix.length()-1) << suffix);
  }

  return stringb(prefix << suffix);
}


// Call 'closedir' in the destructor.
struct CallCloseDir {
  DIR *m_dirp;

  CallCloseDir(DIR *d) : m_dirp(d) {}

  ~CallCloseDir()
  {
    if (0 != closedir(m_dirp)) {
      DEV_WARNING_SYSERROR("closedir");
    }
  }
};


// Rationale for not filtering out "." and "..": the fact that those
// names are special is a POSIX and Windows convention.  It is my intent
// that this module's interface be free of system-specific assumptions.
// Filtering those two names would consistitute such an assumption.
void SMFileUtil::getDirectoryNames(ArrayStack<string> /*OUT*/ &entries,
                                   string const &directory)
{
  entries.clear();

  DIR *dirp = opendir(directory.c_str());
  if (!dirp) {
    xsyserror("opendir", directory);
  }
  CallCloseDir callCloseDir(dirp);

  // I am not using readdir_r here because mingw does not provide it.
  errno = 0;
  struct dirent *ent = readdir(dirp);
  while (ent) {
    entries.push(ent->d_name);

    errno = 0;
    ent = readdir(dirp);
  }

  if (errno) {
    xsyserror("readdir", directory);
  }

  // End of stream.

  // 'dirp' is closed by 'callCloseDir'.
}


// This is defined as an external function so it can be called by
// the test code.  It works on all platforms but is relatively slow
// on Windows.
void getDirectoryEntries_scanThenStat(SMFileUtil &sfu,
  ArrayStack<SMFileUtil::DirEntryInfo> /*OUT*/ &entries, string const &directory)
{
  entries.clear();

  // First get the names.
  ArrayStack<string> names;
  sfu.getDirectoryNames(names, directory);

  // Probe each one to get its file type.
  for (int i=0; i < names.length(); i++) {
    string const &name = names[i];

    SMFileUtil::FileKind kind =
      sfu.getFileKind(sfu.joinFilename(directory, name));
    if (kind == SMFileUtil::FK_NONE) {
      // The file disappeared between when we scanned the directory and
      // when we checked the particular file.
    }
    else {
      entries.push(SMFileUtil::DirEntryInfo(name, kind));
    }
  }
}


void SMFileUtil::getDirectoryEntries(
  ArrayStack<DirEntryInfo> /*OUT*/ &entries, string const &directory)
{
#ifdef __MINGW32__
  struct CallFindClose {
    HANDLE m_hFind;
    string const &m_dir;

    CallFindClose(HANDLE h, string const &d)
      : m_hFind(h),
        m_dir(d)
    {}

    ~CallFindClose()
    {
      if (!FindClose(m_hFind)) {
        DEV_WARNING_SYSERROR_CTXT("FindClose", m_dir.c_str());
      }
    }
  };

  entries.clear();

  // Path to search.  FindFirstFileA has an odd interface, demanding
  // an explicit wildcard to search a directory.
  string searchPath = this->joinFilename(directory, "*");

  // Begin iterating over directory entries.
  WIN32_FIND_DATA fileData;
  HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fileData);
  if (hFind == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      // The directory is empty.  (Although, we never get here on
      // Windows and Linux since there is always "." and "..".)
      return;
    }
    xsyserror("FindFirstFileA", directory);
  }

  // Call 'FindClose' when we return or throw.
  CallFindClose cfc(hFind, directory);

  do {
    // Add the information in 'fileData' to 'entries'.
    FileKind kind =
      (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? FK_DIRECTORY :
      (fileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)? FK_OTHER :
      /*else*/ FK_REGULAR;
    entries.push(DirEntryInfo(fileData.cFileName, kind));

    // Get next entry.
  } while (FindNextFileA(hFind, &fileData));

  if (GetLastError() != ERROR_NO_MORE_FILES) {
    xsyserror("FindNextFileA", directory);
  }

#else
  // The POSIX API does not have a faster way to do this.
  getDirectoryEntries_scanThenStat(*this, entries, directory);

#endif // !__MINGW32__
}


void SMFileUtil::splitPath(string /*OUT*/ &dir, string /*OUT*/ &base,
                           string const &inputPath)
{
  if (inputPath.isempty()) {
    dir = "";
    base = "";
    return;
  }

  // Where to split 'inputPath'.  Everything from 's' and beyond will
  // go into 'base'.
  int s = inputPath.length();
  while (s > 0 && !this->isDirectorySeparator(inputPath[s-1])) {
    s--;
  }

  dir = inputPath.substring(0, s);
  base = inputPath.substring(s, inputPath.length() - s);
}


string SMFileUtil::splitPathDir(string const &inputPath)
{
  string dir, base;
  splitPath(dir, base, inputPath);
  return dir;
}

string SMFileUtil::splitPathBase(string const &inputPath)
{
  string dir, base;
  splitPath(dir, base, inputPath);
  return base;
}


// ----------------------- TestSMFileUtil ------------------------
bool TestSMFileUtil::windowsPathSemantics()
{
  return m_windowsPathSemantics;
}


bool TestSMFileUtil::absolutePathExists(string const &path)
{
  return m_existingPaths.contains(path);
}


// EOF
