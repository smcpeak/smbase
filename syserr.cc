// syserr.cc
// Code for `syserr.h`.

// This file is in the public domain.

#include "syserr.h"                    // this module

#include "smbase/dev-warning.h"        // devWarning
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/system-error-code.h"  // SystemErrorCode

#include <cstring>                     // std::strlen


OPEN_NAMESPACE(smbase)


XSysError::XSysError(PortableErrorCode r, int sysCode, rostring sysReason,
                     rostring syscall, rostring ctx)
  : XBase(),
    reason(r),
    reasonString(reasonCodeDescription(r)),
    sysErrorCode(sysCode),
    sysReasonString(sysReason),
    syscallName(syscall),
    context(ctx)
{}


STATICDEF string XSysError::
  constructWhyString(PortableErrorCode r, rostring sysReason,
                     rostring syscall, rostring ctx)
{
  // build string; start with syscall that failed
  stringBuilder sb;
  sb << syscall;
  if (!ctx.empty()) {
    sb << ": " << doubleQuote(ctx);
  }
  sb << ": ";

  // now a failure reason string
  if (r != PortableErrorCode::PEC_UNKNOWN) {
    sb << reasonCodeDescription(r);
  }
  else if ( /*(sysReason != NULL) &&*/ (sysReason[0] != 0)) {
    sb << sysReason;
  }
  else {
    // no useful info, use the PEC_UNKNOWN string
    sb << reasonCodeDescription(r);
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


STATICDEF int XSysError::getSystemErrorCode()
{
  return SystemErrorCode::getCurrent().systemCode();
}


STATICDEF PortableErrorCode XSysError::portablize(
  int sysErrorCode, std::string &sysReason)
{
  SystemErrorCode sec(sysErrorCode);
  sysReason = sec.codeDescription();
  return sec.portableCode();
}


STATICDEF void XSysError::
  xsyserror(rostring syscallName, rostring context)
{
  // retrieve system error code
  int code = getSystemErrorCode();

  // translate it into one of ours
  string sysMsg;
  PortableErrorCode r = portablize(code, sysMsg);

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
  PortableErrorCode r = XSysError::portablize(systemErrorCode, sysMsg);
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


CLOSE_NAMESPACE(smbase)


// EOF

