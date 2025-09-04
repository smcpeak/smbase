// syserr.cc
// Code for `syserr.h`.

// This file is in the public domain.

#include "syserr.h"                    // this module

#include "smbase/dev-warning.h"        // devWarning
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/str.h"                // string
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/system-error-code.h"  // SystemErrorCode

#include <cstring>                     // std::strlen
#include <sstream>                     // std::ostringstream


OPEN_NAMESPACE(smbase)


XSysError::XSysError(
  SystemErrorCode systemErrorCode,
  std::string const &syscallName,
  std::string const &argument)
  : XBase(),
    IMEMBFP(systemErrorCode),
    IMEMBFP(syscallName),
    IMEMBFP(argument)
{}


XSysError::XSysError(XSysError const &obj)
  : XBase(obj),
    DMEMB(m_systemErrorCode),
    DMEMB(m_syscallName),
    DMEMB(m_argument)
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

  if (!m_argument.empty()) {
    sb << doubleQuote(m_argument) << ": ";
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


DEFINE_EXN_GET_TYPE_NAME(XSysError)


void xsyserror(char const *syscallName)
{
  xsyserror(syscallName, std::string(""));
}


void xsyserror(std::string const &syscallName,
               std::string const &argument)
{
  SystemErrorCode sec = SystemErrorCode::getCurrent();
  THROW(XSysError(sec, syscallName, argument));
}


std::string sysErrorCodeString(
  SystemErrorCode systemErrorCode,
  std::string const &syscallName,
  std::string const &argument)
{
  XSysError x(systemErrorCode, syscallName, argument);
  return x.getConflict();
}


string sysErrorString(char const *syscallName,
                      char const *argument)
{
  return sysErrorCodeString(
    SystemErrorCode::getCurrent(),
    syscallName,
    argument);
}


void devWarningSysError(char const *file, int line,
                        char const *syscallName, char const *argument)
{
  devWarning(file, line, sysErrorString(syscallName, argument).c_str());
}


CLOSE_NAMESPACE(smbase)


// EOF

