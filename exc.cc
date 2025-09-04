// exc.cc            see license.txt for copyright and terms of use
// code for exc.h
// Scott McPeak, 1996-1998  This file is public domain.

#include "exc.h"                       // this module

#include "smbase/dev-warning.h"        // DEV_WARNING
#include "smbase/sm-iostream.h"        // clog
#include "smbase/sm-macros.h"          // DMEMB, CMEMB
#include "smbase/sm-span-util.h"       // smbase::joinTerminate
#include "smbase/sm-span.h"            // smbase::Span
#include "smbase/str.h"                // rostring
#include "smbase/string-util.h"        // join, withoutDirectoryPrefix
#include "smbase/vector-util.h"        // vecEraseFirstN
#include "smbase/xassert.h"            // x_assert_fail

#include <algorithm>                   // std::min


OPEN_NAMESPACE(smbase)


void printUnhandled(XBase const &x)
{
  DEV_WARNING("Unhandled exception: " << x);
}


std::vector<std::string> &getExnContextVector()
{
  static std::vector<std::string> exnContextVector;
  return exnContextVector;
}


std::string getExnContextString()
{
  return join(suffixAll(getExnContextVector(), ": "), "");
}


std::size_t getExnContextSize()
{
  return getExnContextVector().size();
}


// ------------------------------- XBase -------------------------------
XBase::XBase() noexcept
  : std::exception(),
    m_whatStorage(),
    m_contexts(getExnContextVector())
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
  // class reflects the current information.  There is no need to
  // optimize the speed of exception stringification.
  m_whatStorage = getMessage();

  return m_whatStorage.c_str();
}


std::string XBase::getMessage() const
{
  return getContext() + getConflict();
}


std::string XBase::getContext() const
{
  return joinTerminate(m_contexts, ": ");
}


void XBase::prependContext(std::string const &context)
{
  // The expected insertion point is `getExnContextSize()`, but for
  // safety, limit it to the bounds of `m_contexts`.
  std::size_t index = std::min(m_contexts.size(), getExnContextSize());

  // Insert `context` after any context still on the stack, but before
  // whatever was added closer to the throw site.
  m_contexts.insert(m_contexts.begin() + index, context);
}


void XBase::appendContext(std::string const &context)
{
  m_contexts.push_back(context);
}


std::string XBase::getRelayContext() const
{
  // Compute the length of the common prefix.  Normally this is the
  // same as `getExnContextSize()`.
  std::size_t prefixLen =
    vecCommonPrefixLength(m_contexts, getExnContextVector());

  // Get a span that excludes the common prefix.
  auto afterPrefix =
    Span<std::string const>(m_contexts).subspan(prefixLen);

  // Join/terminate that with colons.
  return joinTerminate(afterPrefix, ": ");
}


std::string XBase::getRelayMessage() const
{
  return getRelayContext() + getConflict();
}


DEFINE_EXN_GET_TYPE_NAME(XBase)


void XBase::insert(ostream &os) const
{
  os << getMessage();
}


char const *getExceptionTypeName(std::exception const &x)
{
  if (XBase const *xbase = dynamic_cast<XBase const *>(&x)) {
    return xbase->getTypeName();
  }
  else {
    return "std::exception";
  }
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


DEFINE_EXN_GET_TYPE_NAME(XMessage)


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


DEFINE_EXN_GET_TYPE_NAME(XAssert)


// failure function, declared in xassert.h
void x_assert_fail(char const *cond, char const *file, int line)
{
  // The `file` that I get from `__FILE__` can have directory
  // information, but that should never be needed for disambiguation, so
  // I want to remove the clutter.
  file = withoutDirectoryPrefix(file);

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


DEFINE_EXN_GET_TYPE_NAME(XFormat)


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


DEFINE_EXN_GET_TYPE_NAME(XUnimp)


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


DEFINE_EXN_GET_TYPE_NAME(XFatal)


void throw_XFatal(rostring msg)
{
  XFatal x(msg);
  THROW(x);
}


CLOSE_NAMESPACE(smbase)


// EOF

