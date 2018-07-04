// sm-file-util.cc
// code for sm-file-util.h

#include "sm-file-util.h"              // this module

#include "array.h"                     // Array
#include "strtokp.h"                   // StrtokParse
#include "syserr.h"                    // xsyserror

#include <algorithm>                   // std::max

#include <string.h>                    // strlen

// Use the Windows API?  I want an easy way to switch it so I can test
// both ways under cygwin.
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
#  include <errno.h>                   // errno
#  include <limits.h>                  // PATH_MAX
#  include <stdlib.h>                  // getcwd
#  include <unistd.h>                  // pathconf
#endif


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


// EOF
