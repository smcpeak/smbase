// sm-file-util.cc
// code for sm-file-util.h

#include "sm-file-util.h"              // this module

// smbase
#include "array.h"                     // Array
#include "codepoint.h"                 // isLetter
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
#  include <aclapi.h>                  // GetNamedSecurityInfo
#else
#  include <limits.h>                  // PATH_MAX
#  include <stdlib.h>                  // getcwd
#  include <unistd.h>                  // pathconf
#endif


static bool runningOnWindows =
#ifdef __WIN32__
  true;
#else
  false;
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


// ---------------------- SMFileName -------------------------
SMFileName::SMFileName()
  : m_fileSystem(""),
    m_isAbsolute(false),
    m_pathComponents(),
    m_trailingSlash(false)
{}


SMFileName::SMFileName(string const &path, Syntax syntax)
  : m_fileSystem(""),
    m_isAbsolute(false),
    m_pathComponents(),
    m_trailingSlash(false)
{
  bool windowsSyntax = isWindowsSyntax(syntax);

  // Parse state.
  enum State {
    S_INIT,                            // Start of parsing.
    S_AFTER_SLASH,                     // Just passed a slash.
    S_IN_PATH_COMPONENT,               // Accumulating a path component.

    // Windows-only states:
    S_AFTER_INITIAL_LETTER,            // Saw an initial letter.
    S_AFTER_INITIAL_SLASH,             // Saw an initial slash.
    S_AFTER_FILE_SYSTEM,               // Saw a file system designator.
  };
  State state = S_INIT;

  // Current path component being accumulated.
  ArrayStack<unsigned char> curComponent;

  for (unsigned char const *p = (unsigned char const*)path.c_str(); *p; p++) {
    switch (state) {
      case S_INIT:
        if (isPathSeparator(*p, syntax)) {
          xassert(!m_isAbsolute);       // Should only be set once.
          m_isAbsolute = true;
          if (windowsSyntax) {
            state = S_AFTER_INITIAL_SLASH;
          }
          else {
            state = S_AFTER_SLASH;
          }
        }
        else if (isLetter(*p) && windowsSyntax) {
          // Possible drive letter.
          curComponent.push(*p);
          state = S_AFTER_INITIAL_LETTER;
        }
        else {
          // Element of path component.
          curComponent.push(*p);
          state = S_IN_PATH_COMPONENT;
        }
        break;

      case S_AFTER_SLASH:
      case S_AFTER_INITIAL_SLASH:
        if (isPathSeparator(*p, syntax)) {
          if (state == S_AFTER_INITIAL_SLASH) {
            // UNC path.
            xassert(m_fileSystem.empty());     // Only do once.
            m_fileSystem = "/";
            state = S_AFTER_SLASH;
          }
          else {
            // We just saw a slash.  Ignore the repetition.
          }
        }
        else {
          curComponent.push(*p);
          state = S_IN_PATH_COMPONENT;
        }
        break;

      case S_IN_PATH_COMPONENT:
        if (isPathSeparator(*p, syntax)) {
          // Finish this path component.
          m_pathComponents.push(::toString(curComponent));
          curComponent.clear();
          state = S_AFTER_SLASH;
        }
        else {
          curComponent.push(*p);
        }
        break;

      case S_AFTER_INITIAL_LETTER:
        if (*p == ':') {
          // Finish drive letter.
          curComponent.push(*p);
          m_fileSystem = ::toString(curComponent);
          curComponent.clear();
          state = S_AFTER_FILE_SYSTEM;
        }
        else if (isPathSeparator(*p, syntax)) {
          // The letter is not a drive letter.
          m_pathComponents.push(::toString(curComponent));
          curComponent.clear();
          state = S_AFTER_SLASH;
        }
        else {
          // Not a drive letter.
          curComponent.push(*p);
          state = S_IN_PATH_COMPONENT;
        }
        break;

      case S_AFTER_FILE_SYSTEM:
        if (isPathSeparator(*p, syntax)) {
          xassert(!m_isAbsolute);       // Should only be set once.
          m_isAbsolute = true;
          state = S_AFTER_SLASH;
        }
        else {
          // Element of path component.
          curComponent.push(*p);
          state = S_IN_PATH_COMPONENT;
        }
        break;

      default:
        xfailure("invalid state");
    }
  }

  if (state == S_AFTER_SLASH && m_pathComponents.isNotEmpty()) {
    m_trailingSlash = true;
  }

  if (curComponent.isNotEmpty()) {
    // Final path component.
    m_pathComponents.push(::toString(curComponent));
  }
}


SMFileName::SMFileName(string fileSystem, bool isAbsolute,
                       ArrayStack<string> const &pathComponents,
                       bool trailingSlash)
  : m_fileSystem(fileSystem),
    m_isAbsolute(isAbsolute),
    m_pathComponents(pathComponents),
    m_trailingSlash(trailingSlash)
{}


SMFileName::SMFileName(SMFileName const &obj)
  : DMEMB(m_fileSystem),
    DMEMB(m_isAbsolute),
    DMEMB(m_pathComponents),
    DMEMB(m_trailingSlash)
{}


SMFileName::~SMFileName()
{}


bool SMFileName::operator== (SMFileName const &obj) const
{
  return EMEMB(m_fileSystem) &&
         EMEMB(m_isAbsolute) &&
         EMEMB(m_pathComponents) &&
         EMEMB(m_trailingSlash);
}


void SMFileName::getPathComponents(ArrayStack<string> /*OUT*/ &pathComponents) const
{
  pathComponents = m_pathComponents;
}


SMFileName SMFileName::withFileSystem(string const &newFileSystem) const
{
  return SMFileName(newFileSystem, m_isAbsolute,
                    m_pathComponents, m_trailingSlash);
}

SMFileName SMFileName::withIsAbsolute(bool newIsAbsolute) const
{
  return SMFileName(m_fileSystem, newIsAbsolute,
                    m_pathComponents, m_trailingSlash);
}

SMFileName SMFileName::withPathComponents(ArrayStack<string> const &newPathComponents) const
{
  return SMFileName(m_fileSystem, m_isAbsolute,
                    newPathComponents, m_trailingSlash);
}

SMFileName SMFileName::withTrailingSlash(bool newTrailingSlash) const
{
  return SMFileName(m_fileSystem, m_isAbsolute,
                    m_pathComponents, newTrailingSlash);
}


string SMFileName::toString(Syntax /*syntax*/) const
{
  stringBuilder sb;
  sb << m_fileSystem;
  if (m_isAbsolute) {
    sb << '/';
  }
  sb << this->getPathComponentsString();
  if (m_trailingSlash) {
    sb << '/';
  }
  return sb.str();
}


string SMFileName::getPathComponentsString() const
{
  stringBuilder sb;
  for (int i=0; i < m_pathComponents.length(); i++) {
    sb << m_pathComponents[i];
    if (i+1 < m_pathComponents.length()) {
      sb << '/';
    }
  }
  return sb.str();
}


/*static*/ bool SMFileName::isWindowsSyntax(Syntax syntax)
{
  return syntax == S_WINDOWS ||
         (runningOnWindows && syntax == S_NATIVE);
}


/*static*/ bool SMFileName::isPathSeparator(unsigned char c, Syntax syntax)
{
  return c == '/' || (isWindowsSyntax(syntax) && c == '\\');
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


int SMFileUtil::DirEntryInfo::compareTo(DirEntryInfo const &obj) const
{
  int res = m_name.compareTo(obj.m_name);
  if (res) { return res; }

  return (int)m_kind - (int)obj.m_kind;
}


/*static*/ int SMFileUtil::DirEntryInfo::compare(
  DirEntryInfo const *a, DirEntryInfo const *b)
{
  return a->compareTo(*b);
}


// ----------------------- SMFileUtil ------------------------
bool SMFileUtil::windowsPathSemantics()
{
  return runningOnWindows;
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


bool SMFileUtil::isReadOnly(string const &path) NOEXCEPT
{
#if SM_FILE_UTIL_USE_WINDOWS_API
  // This is a nasty bit of code.  The Windows security API is a mess
  // and MSDN is utterly unhelpful.  I found this code that does
  // something similar and borrowed a number of ideas from it:
  //
  // https://www.ncbi.nlm.nih.gov/IEB/ToolBox/CPP_DOC/lxr/source/src/corelib/ncbi_os_mswin.cpp

  // ----------------- part 0: closing handles -------------------
  struct CallCloseHandle {
    HANDLE m_handle;
    string m_context;

    CallCloseHandle(HANDLE h, string const &context)
      : m_handle(h),
        m_context(context)
    {}

    ~CallCloseHandle()
    {
      if (!CloseHandle(m_handle)) {
        DEV_WARNING_SYSERROR_CTXT("CloseHandle", m_context.c_str());
      }
    }
  };

  struct CallLocalFree {
    HLOCAL m_handle;

    CallLocalFree(HLOCAL h)
      : m_handle(h)
    {}

    ~CallLocalFree()
    {
      if (NULL != LocalFree(m_handle)) {
        DEV_WARNING_SYSERROR("LocalFree");
      }
    }
  };

  struct CallRevertToSelf {
    CallRevertToSelf()
    {}

    ~CallRevertToSelf()
    {
      if (!RevertToSelf()) {
        DEV_WARNING_SYSERROR("RevertToSelf");
      }
    }
  };

  // --------------- part 1: get file security info ----------------
  // Query the file's security attributes.
  PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
  DWORD res = GetNamedSecurityInfoA(
    path.c_str(),                      // pObjectName
    SE_FILE_OBJECT,                    // ObjectType

    // We have to ask for everything (except the SACL, which would be
    // denied) so that the security descriptor is sufficiently populated
    // to allow 'AccessCheck' to work properly.  But we do not actually
    // need these details ourselves, which is why we pass NULL for all
    // of the other pointers like 'ppDacl'.
    DACL_SECURITY_INFORMATION |        // SecurityInfo
      GROUP_SECURITY_INFORMATION |
      OWNER_SECURITY_INFORMATION,

    NULL,                              // ppsidOwner
    NULL,                              // ppsidGroup
    NULL,                              // ppDacl
    NULL,                              // ppSacl
    &pSecurityDescriptor);             // ppSecurityDescriptor
  if (res != ERROR_SUCCESS) {
    // A failure here generally means the file does not exist or we lack
    // permissions to query it.
    return false;
  }

  // Arrange to free the security descriptor structure.
  CallLocalFree callLocalFree(pSecurityDescriptor);

  // --------------- part 2: get a thread token ----------------
  // This is some black magic that is required in order for
  // OpenThreadToken to work.  I would not have figured this out without
  // looking at the NIH code linked above.  MSDN barely hints at it.
  if (!ImpersonateSelf(SecurityImpersonation)) {
    // Can this fail?  The world may never know.
    DEV_WARNING_SYSERROR("ImpersonateSelf");
    return false;
  }

  // Arrange to call RevertToSelf.
  CallRevertToSelf callRevertToSelf;

  // Get access token for the current thread.  Why not the process?
  // Well, I tried that first and it didn't work.  I don't remember the
  // error message anymore but it made no sense.  NIH code to the
  // rescue!  (Except they do this twice for some reason.)
  HANDLE hThread = GetCurrentThread(); // Does not need to be closed.
  HANDLE hThreadToken = INVALID_HANDLE_VALUE;
  BOOL bres = OpenThreadToken(
    hThread,        // ThreadHandle

    // MSDN 'AccessCheck' mentions needing TOKEN_QUERY.
    TOKEN_QUERY,    // DesiredAccess

    // This parameter is total black magic.  Try making sense of this
    // quote from MSDN:
    //
    //   "Without this parameter, the calling thread cannot open the
    //   access token on the specified thread because it is impossible
    //   to open executive-level objects by using the
    //   SecurityIdentification impersonation level."
    //
    // Uh, right.  Well, the NIH code passes FALSE so I will too.
    FALSE,          // OpenAsSelf

    &hThreadToken); // TokenHandle
  if (!bres) {
    // This should only fail if I messed up API calls above.
    DEV_WARNING_SYSERROR("OpenThreadToken");
    return false;
  }

  // Arrange to close the process token.
  CallCloseHandle callCloseHandle_hThreadToken(hThreadToken, "thread token");

  // --------------- part 3: call AccessCheck ----------------
  // This is a mysterious pile of goo.  MSDN is worthless, but it seems
  // you can just pass zeroes and it works.  I probably would have
  // figured out this part on my own.
  GENERIC_MAPPING genericMapping;
  memset(&genericMapping, 0, sizeof(genericMapping));
  PRIVILEGE_SET privilegeSet;
  memset(&privilegeSet, 0, sizeof(privilegeSet));
  DWORD privilegeSetLength = sizeof(privilegeSet);

  // Output of 'AccessCheck'.
  ACCESS_MASK grantedAccess = 0;
  BOOL accessStatus = FALSE;

  // One reading of MSDN would suggest I could pass FILE_WRITE_DATA (see
  // below) instead of MAXIMUM_ALLOWED.  But there is this passage: "If
  // access is granted, the requested access mask becomes the object's
  // granted access mask."  What does that mean?  It could mean that
  // passing less than MAXIMUM_ALLOWED somehow permanently restricts my
  // thread or process in future interactions with the file.  NIH code
  // passes MAXIMUM_ALLOWED, I follow suit.
  bres = AccessCheck(
    pSecurityDescriptor,               // pSecurityDescriptor
    hThreadToken,                      // ClientToken
    MAXIMUM_ALLOWED,                   // DesiredAccess
    &genericMapping,                   // GenericMapping
    &privilegeSet,                     // PrivilegeSet
    &privilegeSetLength,               // PrivilegeSetLength
    &grantedAccess,                    // GrantedAccess
    &accessStatus);                    // AccessStatus
  if (!bres) {
    // I think the only way this fails is if I messed up my API calls
    // above, which of course is certainly possible.
    DEV_WARNING_SYSERROR_CTXT("AccessCheck", path.c_str());
    return false;
  }

  // --------------- part 4: interpret the result ----------------
  // It is far from obvious which bit to test here.  There are many
  // bits, varying in meaning depending on the kind of object.  I got to
  // FILE_WRITE_DATA through a combination of experimentation and
  // clicking around in the MSDN maze.
  return !(grantedAccess & FILE_WRITE_DATA);

#else
  if (access(path.c_str(), W_OK) == 0) {
    // File exists and is writable.
    return false;
  }
  else if (errno == EACCES || errno == EROFS) {
    // File exists but is not writable.
    return true;
  }
  else {
    // File does not exist, or permission error, or something else.
    return false;
  }
#endif // !SM_FILE_UTIL_USE_WINDOWS_API
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
#if SM_FILE_UTIL_USE_WINDOWS_API
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

#endif // !SM_FILE_UTIL_USE_WINDOWS_API
}


void SMFileUtil::getSortedDirectoryEntries(
  ArrayStack<DirEntryInfo> /*OUT*/ &entries, string const &directory)
{
  this->getDirectoryEntries(entries, directory);
  entries.sort(&DirEntryInfo::compare);
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


string SMFileUtil::collapseDots(string const &inputPath)
{
  // Parse into components.  Use S_WINDOWS since it should work fine in
  // practice, for this purpose, on all platforms, and ensures this
  // function behaves the same on all platforms, which is convenient.
  SMFileName fn(inputPath, SMFileName::S_WINDOWS);
  ArrayStack<string> inputComponents;
  fn.getPathComponents(inputComponents);

  // Rebuild the path components, discarding some in response to "." and "..".
  ArrayStack<string> outputComponents;
  for (int i=0; i < inputComponents.length(); i++) {
    string const &comp = inputComponents[i];
    if (comp == ".") {
      // Discard.  (But we might add it back at the end.)
    }
    else if (comp == "..") {
      if (outputComponents.isNotEmpty() && outputComponents.top() != "..") {
        // Cancel the last output component with this "..".
        outputComponents.pop();
      }
      else if (fn.isAbsolute()) {
        // The path "/.." is equivalent to "/" since the ".." entry
        // in the root of the file system points to itself.  Skip.
      }
      else {
        // Retain as part of a prefix of ".." entries.
        outputComponents.push(comp);
      }
    }
    else {
      outputComponents.push(comp);
    }
  }

  if (!fn.isAbsolute() && inputComponents.isNotEmpty() && outputComponents.isEmpty()) {
    // A non-empty relative path collapsed to nothing.  Yield "." to
    // preserve the non-emptiness.  Example: "a/.." -> ".".
    outputComponents.push(".");
  }

  SMFileName fn2(fn.withPathComponents(outputComponents));
  return fn2.toString();
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
