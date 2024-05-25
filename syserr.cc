// syserr.cc            see license.txt for copyright and terms of use
// code for syserr.h
// Scott McPeak, 1999-2000  This file is public domain.

#include "syserr.h"                    // this module

#include "dev-warning.h"               // devWarning
#include "strutil.h"                   // quoted


// ---------------- portable code ----------------
char const * const XSysError::reasonStrings[] = {
  "No error occurred",
  "File not found",
  "Path not found",
  "Access denied",
  "Out of memory (maybe)",    // always a suspicious message
  "Invalid pointer address",
  "Invalid data format",
  "Invalid argument",
  "Attempt to modify read-only data",
  "The object already exists",
  "Resource is temporarily unavailable",
  "Resource is busy",
  "File name is invalid (too long, or bad chars, or ...)",
  "Unknown or unrecognized error",
  "(bug -- invalid reason code)"        // corresponds to NUM_REASONS
};


STATICDEF char const *XSysError::
  getReasonString(XSysError::Reason r)
{
  // at compile-time, verify consistency between enumeration and string array
  // (it's in here because, at least on Borland, the preprocessor respects
  // the member access rules (strangely..))
  #ifdef __BORLANDC__
    #if TABLESIZE(reasonStrings) != NUM_REASONS+1
      #error table and enumeration do not match
    #endif
  #endif

  if ((unsigned)r < NUM_REASONS) {
    return reasonStrings[r];
  }
  else {
    return reasonStrings[NUM_REASONS];
  }
}


XSysError::XSysError(XSysError::Reason r, int sysCode, rostring sysReason,
                     rostring syscall, rostring ctx)
  : XBase(),
    reason(r),
    reasonString(getReasonString(r)),
    sysErrorCode(sysCode),
    sysReasonString(sysReason),
    syscallName(syscall),
    context(ctx)
{}


STATICDEF string XSysError::
  constructWhyString(XSysError::Reason r, rostring sysReason,
                     rostring syscall, rostring ctx)
{
  // build string; start with syscall that failed
  stringBuilder sb;
  sb << syscall;
  if (!ctx.empty()) {
    // Printing the context here makes it clearly associated with the
    // syscall.  There is a danger of thinking it is literally an
    // argument to that syscall, which may or may not be the case, but
    // I think that is tolerable.
    sb << '(' << quoted(ctx) << ')';
  }
  sb << ": ";

  // now a failure reason string
  if (r != R_UNKNOWN) {
    sb << getReasonString(r);
  }
  else if ( /*(sysReason != NULL) &&*/ (sysReason[0] != 0)) {
    sb << sysReason;
  }
  else {
    // no useful info, use the R_UNKNOWN string
    sb << getReasonString(r);
  }

  return sb.str();
}


XSysError::XSysError(XSysError const &obj)
  : XBase(obj),
    reason(obj.reason),
    reasonString(obj.reasonString),
    sysErrorCode(obj.sysErrorCode),
    sysReasonString(obj.sysReasonString),
    syscallName(obj.syscallName),
    context(obj.context)
{}


XSysError::~XSysError()
{}


std::string XSysError::getConflict() const
{
  return constructWhyString(reason, sysReasonString,
                            syscallName, context);
}


STATICDEF void XSysError::
  xsyserror(rostring syscallName, rostring context)
{
  // retrieve system error code
  int code = getSystemErrorCode();

  // translate it into one of ours
  string sysMsg;
  Reason r = portablize(code, sysMsg);

  // construct an object to throw
  XSysError obj(r, code, sysMsg, syscallName, context);

  // toss it
  THROW(obj);
}

void xsyserror(char const *syscallName)
{
  XSysError::xsyserror(syscallName, string(""));
}

void xsyserror(rostring syscallName, rostring context)
{
  XSysError::xsyserror(syscallName, context);
}


string sysErrorCodeString(int systemErrorCode,
                                   rostring syscallName,
                                   rostring context)
{
  string sysMsg;
  XSysError::Reason r = XSysError::portablize(systemErrorCode, sysMsg);
  return XSysError::constructWhyString(
           r, sysMsg,
           syscallName, context);
}

string sysErrorString(char const *syscallName,
                               char const *context)
{
  return sysErrorCodeString(XSysError::getSystemErrorCode(),
                            syscallName, context);
}


void devWarningSysError(char const *file, int line,
                        char const *syscallName, char const *context)
{
  devWarning(file, line, sysErrorString(syscallName, context).c_str());
}


// ----------------------- Win32 code ------------------------------------
#ifdef __WIN32__

#ifdef USE_MINWIN_H
#  include "minwin.h"   // api
#else
#  include <windows.h>  // api
#endif
#include <errno.h>      // errno

STATICDEF int XSysError::getSystemErrorCode()
{
  int ret = GetLastError();

  // update: The confusing behavior I observed was with the Borland 4.5
  // runtime libraries.  When I linked with the non-multithreaded versions,
  // GetLastError worked as expected.  But when I linked with the
  // multithreaded versions, GetLastError always returned 0, and I had to
  // consult errno instead.  Further, the errno values didn't coincide
  // exactly with expected values.  Therefore, the solution (at least for
  // now) is to link only with the non-multithreaded versions, and not
  // look to errno for anything.
  #ifdef MT
  #  error something is fishy with multithreaded..
  #endif

  // I thought something was happening, but now it seems
  // it's not..
  #if 0     // ?
  if (ret == ERROR_SUCCESS) {
    // for some calls, like mkdir, GetLastError is not
    // set, but errno is; fortunately, MS decided to
    // (mostly) overlap GetLastError codes with errno codes,
    // so let's try this:
    return errno;
  }
  #endif // 0

  return ret;
}


STATICDEF XSysError::Reason XSysError::portablize(
  int sysErrorCode, string &sysMsg)
{
  // I'd like to put this into a static class member, but then
  // the table would have to prepend R_ constants with XSysError::,
  // which is a pain.

  // method to translate an error code into a string on win32; this
  // code is copied+modified from the win32 SDK docs for FormatMessage
  {
    // get the string
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );

    // now the SDK says: "Process any inserts in lpMsgBuf."
    //
    // I think this means that lpMsgBuf might have "%" escape
    // sequences in it... oh well, I'm just going to keep them

    // make a copy of the string
    sysMsg = (char*)lpMsgBuf;

    // Free the buffer.
    LocalFree( lpMsgBuf );
  }

  static struct S {
    int code;
    Reason reason;
  } const arr[] = {
    { ERROR_SUCCESS,           R_NO_ERROR          },
    { ERROR_FILE_NOT_FOUND,    R_FILE_NOT_FOUND    },
    { ERROR_PATH_NOT_FOUND,    R_PATH_NOT_FOUND    },
    { ERROR_ACCESS_DENIED,     R_ACCESS_DENIED     },
    { ERROR_NOT_ENOUGH_MEMORY, R_OUT_OF_MEMORY     },
    { ERROR_OUTOFMEMORY,       R_OUT_OF_MEMORY     },
    { ERROR_INVALID_BLOCK,     R_SEGFAULT          },
    { ERROR_BAD_FORMAT,        R_FORMAT            },
    { ERROR_INVALID_DATA,      R_INVALID_ARGUMENT  },
    { ERROR_WRITE_PROTECT,     R_READ_ONLY         },
    { ERROR_ALREADY_EXISTS,    R_ALREADY_EXISTS    },
    // ???                     R_AGAIN
    { ERROR_BUSY,              R_BUSY              },
  };

  smbase_loopi(TABLESIZE(arr)) {
    if (arr[i].code == sysErrorCode) {
      // found it
      return arr[i].reason;
    }
  }

  // I don't know
  return R_UNKNOWN;
}


// ---------------------- unix ---------------------------
#else      // unix

#include <errno.h>       // errno
#include <string.h>      // strerror

// mappings to a set of error codes I can use below
// (I am sure I've already done this somewhere else, but I
// may have lost that file)
#ifndef EZERO
#  define EZERO 0
#endif
#ifndef ENOFILE
#  define ENOFILE ENOENT
#endif
#ifndef ENOPATH
#  define ENOPATH ENOENT
#endif
#ifndef EINVMEM
#  define EINVMEM EFAULT
#endif
#ifndef EINVFMT
#  define EINVFMT 0         // won't be seen because EZERO is first
#endif


STATICDEF int XSysError::getSystemErrorCode()
{
  return errno;          // why was this "errno()"??
}


STATICDEF XSysError::Reason XSysError::portablize(
  int sysErrorCode, string &sysMsg)
{
  sysMsg = strerror(sysErrorCode);
    // operator= copies to local storage

  static struct S {
    int code;
    Reason reason;
  } const arr[] = {
    { EZERO,        R_NO_ERROR          },
    { ENOFILE,      R_FILE_NOT_FOUND    },
    { ENOPATH,      R_PATH_NOT_FOUND    },
    { EACCES,       R_ACCESS_DENIED     },
    { ENOMEM,       R_OUT_OF_MEMORY     },
    { EINVMEM,      R_SEGFAULT          },
    { EINVFMT,      R_FORMAT            },
    { EINVAL,       R_INVALID_ARGUMENT  },
    { EROFS,        R_READ_ONLY         },
    { EEXIST,       R_ALREADY_EXISTS    },
    { EAGAIN,       R_AGAIN             },
    { EBUSY,        R_BUSY              },
    { ENAMETOOLONG, R_INVALID_FILENAME  },
  };

  smbase_loopi(TABLESIZE(arr)) {
    if (arr[i].code == sysErrorCode) {
      // found it
      return arr[i].reason;
    }
  }

  // I don't know
  return R_UNKNOWN;
}


#endif  // unix


// EOF

