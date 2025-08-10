// syserr.cc
// Code for `syserr.h`.

// This file is in the public domain.

#include "syserr.h"                    // this module

#include "smbase/dev-warning.h"        // devWarning
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/system-error-code.h"  // SystemErrorCode

#include <cstring>                     // std::strlen
#include <sstream>                     // std::ostringstream


OPEN_NAMESPACE(smbase)


XSysError::XSysError(
  SystemErrorCode systemErrorCode,
  std::string const &syscallName,
  std::string const &context)
  : XBase(),
    IMEMBFP(systemErrorCode),
    IMEMBFP(syscallName),
    IMEMBFP(context)
{}


XSysError::XSysError(XSysError const &obj)
  : XBase(obj),
    DMEMB(m_systemErrorCode),
    DMEMB(m_syscallName),
    DMEMB(m_context)
{}


XSysError::~XSysError()
{}


SystemErrorCode XSysError::getSystemErrorCode() const
{
  return m_systemErrorCode;
}


std::string XSysError::getSystemErrorDescription() const
{
  return m_systemErrorCode.codeDescription();
}


PortableErrorCode XSysError::getPortableErrorCode() const
{
  return m_systemErrorCode.portableCode();
}


std::string XSysError::getPortableErrorDescription() const
{
  return portableCodeDescription(getPortableErrorCode());
}


std::string XSysError::getImmediateContext() const
{
  std::ostringstream sb;

  sb << m_syscallName << ": ";

  if (!m_context.empty()) {
    sb << doubleQuote(m_context) << ": ";
  }

  return sb.str();
}


std::string XSysError::getPortableConflict() const
{
  return getImmediateContext() + getPortableErrorDescription();
}


std::string XSysError::getConflict() const
{
  return getImmediateContext() + getSystemErrorDescription();
}


void xsyserror(char const *syscallName)
{
  xsyserror(syscallName, std::string(""));
}


void xsyserror(std::string const &syscallName,
               std::string const &context)
{
  SystemErrorCode sec = SystemErrorCode::getCurrent();
  THROW(XSysError(sec, syscallName, context));
}


std::string sysErrorCodeString(
  SystemErrorCode systemErrorCode,
  std::string const &syscallName,
  std::string const &context)
{
  XSysError x(systemErrorCode, syscallName, context);
  return x.getConflict();
}


string sysErrorString(char const *syscallName,
                      char const *context)
{
  return sysErrorCodeString(
    SystemErrorCode::getCurrent(),
    syscallName,
    context);
}


void devWarningSysError(char const *file, int line,
                        char const *syscallName, char const *context)
{
  devWarning(file, line, sysErrorString(syscallName, context).c_str());
}


CLOSE_NAMESPACE(smbase)


// EOF

