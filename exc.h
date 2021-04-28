// exc.h            see license.txt for copyright and terms of use
// exception classes for SafeTP project
// Scott McPeak, 1996-1998  This file is public domain.

// I apologize for the inconsistent naming in this module.  It is
// the product of an extended period of experimenting with naming
// conventions for exception-related concepts.  The names near the
// end of the file reflect my current preferences.

#ifndef EXC_H
#define EXC_H

#include "breaker.h"     // breaker
#include "typ.h"         // bool
#include "xassert.h"     // xassert, for convenience for #includers
#include "str.h"         // string
#include "sm-iostream.h" // ostream


// by using this macro, the debugger gets a shot before the stack is unwound
#ifdef THROW
#undef THROW
#endif
#define THROW(obj) \
  { breaker(); throw (obj); }


// My intention is to put a call to this macro at the beginning of
// every catch block.  Its (default) definition is to call breaker so
// that in the debugger I can easily get to the point where an
// exception is caught.
#define HANDLER() breaker() /* user ; */


// Removed 2018-07-06:
//   unwinding
//   unwinding_other
//   CAUTIOUS_RELAY
// The replacement to checking 'unwinding()' is to wrap the code in
// GENERIC_CATCH_BEGIN and GENERIC_CATCH_END so exceptions do not
// propagate out of destructors.


// -------------------- xBase ------------------
// intent is to derive all exception objects from this
class xBase {
protected:
  // the human-readable description of the exception
  string msg;

public:
  // Initially false.  When true, we write a record of the thrown
  // exception to clog for debug assistance.
  static bool logExceptions;

  // current # of xBases running about; used to support unrolling()
  static int creationCount;

public:
  xBase(rostring m);       // create exception object with message 'm'
  xBase(xBase const &m);   // copy ctor
  virtual ~xBase();

  rostring why() const
    { return msg; }

  // print why
  void insert(ostream &os) const;
  friend ostream& operator << (ostream &os, xBase const &obj)
    { obj.insert(os); return os; }

  // add a string describing what was going on at the time the
  // exception was thrown; this should be called with the innermost
  // context string first, i.e., in the normal unwind order
  void addContext(rostring context);
  // dsw: sometimes I want the context to be, say a "file:line" number
  // on the left
  void addContextLeft(rostring context);

  // Like 'addContextLeft', except it also puts ": " between 'context'
  // and the existing 'msg'.
  void prependContext(rostring context);
};

// equivalent to THROW(xBase(msg))
void xbase(rostring msg) NORETURN;


// This is used when we do not expect an exception to be thrown, and
// do not have a good recovery available, but it might happen, and we
// do not want to simply terminate.
//
// Print details about 'x' to stderr using DEV_WARNING (dev-warning.h).
void printUnhandled(xBase const &x);


// This goes at the top of any function we do not want to let throw
// an exception.  Often such functions are marked 'noexcept'.
#define GENERIC_CATCH_BEGIN         \
  try {

// And this goes at the bottom.  This if the client does not do
// anything special, 'printUnhandled' will use the global version,
// but in a class method, a client can provide their own handler
// with that name.
#define GENERIC_CATCH_END             \
  }                                   \
  catch (xBase &x) {                  \
    printUnhandled(x);                \
  }

// Variant for functions that return a value.
#define GENERIC_CATCH_END_RET(retval) \
  }                                   \
  catch (xBase &x) {                  \
    printUnhandled(x);                \
    return retval;                    \
  }


// Define a subclass of xBase.  All methods are inline.
#define DEFINE_XBASE_SUBCLASS(SubclassName)                          \
  class SubclassName : public xBase {                                \
  public:                                                            \
    SubclassName(char const *p) : xBase(p) {}                        \
    SubclassName(string const &s) : xBase(s) {}                      \
    SubclassName(SubclassName const &obj) : xBase(obj) {}            \
  } /* user ; */


// -------------------- x_assert -----------------------
// thrown by _xassert_fail, declared in xassert.h
// throwing this corresponds to detecting a bug in the program
class x_assert : public xBase {
  string condition;          // text of the failed condition
  string filename;           // name of the source file
  int lineno;                // line number

public:
  x_assert(rostring cond, rostring fname, int line);
  x_assert(x_assert const &obj);
  ~x_assert();

  rostring cond() const { return condition; }
  rostring fname() const { return filename; }
  int line() const { return lineno; }
};


// ---------------------- xFormat -------------------
// throwing this means a formatting error has been detected
// in some input data; the program cannot process it, but it
// is not a bug in the program
class xFormat : public xBase {
  string condition;          // what is wrong with the input

public:
  xFormat(rostring cond);
  xFormat(xFormat const &obj);
  ~xFormat();

  rostring cond() const { return condition; }
};

// compact way to throw an xFormat
void xformat(rostring condition) NORETURN;

#define xformatsb(msg) xformat(stringb(msg))

// convenient combination of condition and human-readable message
#define checkFormat(cond, message) \
  ((cond)? (void)0 : xformat(message))

// assert-like interface to xFormat
void formatAssert_fail(char const *cond, char const *file, int line) NORETURN;

#define formatAssert(cond) \
  ((cond)? (void)0 : formatAssert_fail(#cond, __FILE__, __LINE__))


// -------------------- XOpen ---------------------
// thrown when we fail to open a file
class XOpen : public xBase {
public:
  string filename;

public:
  XOpen(rostring fname);
  XOpen(XOpen const &obj);
  ~XOpen();
};

void throw_XOpen(rostring fname) NORETURN;


// -------------------- XOpenEx ---------------------
// more informative
class XOpenEx : public XOpen {
public:
  string mode;         // fopen-style mode string, e.g. "r"
  string cause;        // errno-derived failure cause, e.g. "no such file"

public:
  XOpenEx(rostring fname, rostring mode, rostring cause);
  XOpenEx(XOpenEx const &obj);
  ~XOpenEx();

  // convert a mode string as into human-readable participle,
  // e.g. "r" becomes "reading"
  static string interpretMode(rostring mode);
};

void throw_XOpenEx(rostring fname, rostring mode, rostring cause) NORETURN;


// ------------------- XUnimp ---------------------
// thrown in response to a condition that is in principle
// allowed but not yet handled by the existing code
class XUnimp : public xBase {
public:
  XUnimp(rostring msg);
  XUnimp(XUnimp const &obj);
  ~XUnimp();
};

void throw_XUnimp(rostring msg) NORETURN;

// throw XUnimp with file/line info
void throw_XUnimp(char const *msg, char const *file, int line) NORETURN;

#define xunimp(msg) throw_XUnimp(msg, __FILE__, __LINE__)


// ------------------- XFatal ---------------------
// thrown in response to a user action that leads to an unrecoverable
// error; it is not due to a bug in the program
class XFatal : public xBase {
public:
  XFatal(rostring msg);
  XFatal(XFatal const &obj);
  ~XFatal();
};

void throw_XFatal(rostring msg) NORETURN;
#define xfatal(msg) throw_XFatal(stringc << msg)


#endif // EXC_H

