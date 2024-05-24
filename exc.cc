// exc.cc            see license.txt for copyright and terms of use
// code for exc.h
// Scott McPeak, 1996-1998  This file is public domain.

#include "exc.h"                       // this module

// smbase
#include "dev-warning.h"               // DEV_WARNING
#include "sm-iostream.h"               // clog

// libc
#include <ctype.h>                     // toupper, tolower
#include <stdarg.h>                    // va_xxx
#include <string.h>                    // strlen, strcpy


void printUnhandled(XBase const &x)
{
  DEV_WARNING("Unhandled exception: " << x.why());
}


// ------------------------- XBase -----------------
bool XBase::logExceptions = false;
int XBase::creationCount = 0;


XBase::XBase(rostring m)
  : msg(m)
{
  if (logExceptions) {
    clog << "Exception thrown: " << m << endl;
  }

  // done at very end when we know this object will
  // successfully be created
  creationCount++;
}


XBase::XBase(XBase const &obj)
  : msg(obj.msg)
{
  creationCount++;
}


XBase::~XBase()
{
  creationCount--;
}


void XBase::insert(ostream &os) const
{
  os << why();
}


void xbase(rostring msg)
{
  XBase x(msg);
  THROW(x);
}


void XBase::addContext(rostring context)
{
  // for now, fairly simple
  msg = stringb("while " << context << ",\n" << msg);
}


void XBase::addContextLeft(rostring context)
{
  msg = stringb(context << msg);
}


void XBase::prependContext(rostring context)
{
  msg = stringb(context << ": " << msg);
}


// ------------------- x_assert -----------------
x_assert::x_assert(rostring cond, rostring fname, int line)
  : XBase(stringb(
      fname << ":" << line << ": assertion failed: " << cond)),
    condition(cond),
    filename(fname),
    lineno(line)
{}

x_assert::x_assert(x_assert const &obj)
  : XBase(obj),
    condition(obj.condition),
    filename(obj.filename),
    lineno(obj.lineno)
{}

x_assert::~x_assert()
{}


// failure function, declared in xassert.h
void x_assert_fail(char const *cond, char const *file, int line)
{
  THROW(x_assert(cond, file, line));
}


// --------------- xFormat ------------------
xFormat::xFormat(rostring cond)
  : XBase(cond)
{}

xFormat::xFormat(xFormat const &obj)
  : XBase(obj)
{}

xFormat::~xFormat()
{}


void xformat(rostring condition)
{
  xFormat x(condition);
  THROW(x);
}

void formatAssert_fail(char const *cond, char const *file, int line)
{
  xFormat x(stringb("format assertion failed, "
                    << file << ":" << line << ": "
                    << cond));
  THROW(x);
}


// -------------------- XUnimp -------------------
XUnimp::XUnimp(rostring msg)
  : XBase(stringb("unimplemented: " << msg))
{}

XUnimp::XUnimp(XUnimp const &obj)
  : XBase(obj)
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
  : XBase(stringb("error: " << msg))
{}

XFatal::XFatal(XFatal const &obj)
  : XBase(obj)
{}

XFatal::~XFatal()
{}


void throw_XFatal(rostring msg)
{
  XFatal x(msg);
  THROW(x);
}


// ---------------- test code ------------------
#ifdef TEST_EXC

int main()
{
  XBase x("yadda");
  cout << x << endl;

  try {
    THROW(x);
  }
  catch (XBase &x) {
    cout << "caught XBase: " << x << endl;
  }

  return 0;
}

#endif // TEST_EXC

