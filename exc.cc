// exc.cc            see license.txt for copyright and terms of use
// code for exc.h
// Scott McPeak, 1996-1998  This file is public domain.

#include "exc.h"                       // this module

// smbase
#include "dev-warning.h"               // DEV_WARNING
#include "sm-iostream.h"               // clog
#include "sm-macros.h"                 // DMEMB, CMEMB
#include "string-utils.h"              // join

// libc
#include <ctype.h>                     // toupper, tolower
#include <stdarg.h>                    // va_xxx
#include <string.h>                    // strlen, strcpy


void printUnhandled(XBase const &x)
{
  DEV_WARNING("Unhandled exception: " << x);
}


// ------------------------------- XBase -------------------------------
XBase::XBase() noexcept
  : std::exception(),
    m_whatStorage(),
    m_contexts()
{}


XBase::XBase(XBase const &obj) noexcept
  : std::exception(obj),
    DMEMB(m_whatStorage),
    DMEMB(m_contexts)
{}


XBase &XBase::operator=(XBase const &obj) noexcept
{
  if (this != &obj) {
    std::exception::operator=(obj);
    CMEMB(m_whatStorage);
    CMEMB(m_contexts);
  }
  return *this;
}


XBase::~XBase()
{}


char const *XBase::what() const noexcept
{
  // We recompute this every time in order to ensure that a derived
  // class that it reflects the current information.  There is no need
  // to optimize the speed of exception stringification.
  m_whatStorage = getMessage();

  return m_whatStorage.c_str();
}


std::string XBase::getMessage() const
{
  std::string context = getContext();
  if (context.empty()) {
    return getConflict();
  }
  else {
    return context + ": " + getConflict();
  }
}


std::string XBase::getContext() const
{
  return join(m_contexts, ": ");
}


void XBase::prependContext(std::string const &context)
{
  m_contexts.insert(m_contexts.begin(), context);
}


void XBase::appendContext(std::string const &context)
{
  m_contexts.push_back(context);
}


void XBase::insert(ostream &os) const
{
  os << getMessage();
}


// ----------------------------- XMessage ------------------------------
XMessage::XMessage(std::string const &message) noexcept
  : XBase(),
    m_message(message)
{}


XMessage::XMessage(XMessage const &obj) noexcept
  : XBase(obj),
    DMEMB(m_message)
{}


XMessage &XMessage::operator=(XMessage const &obj) noexcept
{
  if (this != &obj) {
    XBase::operator=(obj);
    CMEMB(m_message);
  }
  return *this;
}


std::string XMessage::getConflict() const
{
  return m_message;
}


void xmessage(std::string const &msg)
{
  XMessage x(msg);
  THROW(x);
}


// ------------------------------ XAssert ------------------------------
XAssert::XAssert(rostring cond, rostring fname, int line)
  : XBase(),
    condition(cond),
    filename(fname),
    lineno(line)
{}

XAssert::XAssert(XAssert const &obj)
  : XBase(obj),
    condition(obj.condition),
    filename(obj.filename),
    lineno(obj.lineno)
{}

XAssert::~XAssert()
{}


std::string XAssert::getConflict() const
{
  return stringb(
    fname() << ":" << line() << ": assertion failed: " << cond());
}


// failure function, declared in xassert.h
void x_assert_fail(char const *cond, char const *file, int line)
{
  THROW(XAssert(cond, file, line));
}


// --------------- XFormat ------------------
XFormat::XFormat(rostring cond)
  : XMessage(cond)
{}

XFormat::XFormat(XFormat const &obj)
  : XMessage(obj)
{}

XFormat::~XFormat()
{}


void xformat(rostring condition)
{
  XFormat x(condition);
  THROW(x);
}

void formatAssert_fail(char const *cond, char const *file, int line)
{
  XFormat x(stringb("format assertion failed, "
                    << file << ":" << line << ": "
                    << cond));
  THROW(x);
}


// -------------------- XUnimp -------------------
XUnimp::XUnimp(rostring msg)
  : XMessage(stringb("unimplemented: " << msg))
{}

XUnimp::XUnimp(XUnimp const &obj)
  : XMessage(obj)
{}

XUnimp::~XUnimp()
{}


void throw_XUnimp(rostring msg)
{
  XUnimp x(msg);
  THROW(x);
}


void throw_XUnimp(char const *msg, char const *file, int line)
{
  throw_XUnimp(stringb(file << ":" << line << ": " << msg));
}


// -------------------- XFatal -------------------
// That this error is "fatal" need not be stated in the error message
// itself.  Doing so would unnecessarily alarm novice users, and the
// fatal-ness is sufficiently expressed by the fact that an exception
// is thrown, as opposed to simply printing the message and continuing.
XFatal::XFatal(rostring msg)
  : XMessage(stringb("error: " << msg))
{}

XFatal::XFatal(XFatal const &obj)
  : XMessage(obj)
{}

XFatal::~XFatal()
{}


void throw_XFatal(rostring msg)
{
  XFatal x(msg);
  THROW(x);
}


// EOF

