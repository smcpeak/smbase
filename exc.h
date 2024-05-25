// exc.h            see license.txt for copyright and terms of use
// Various exception classes.  The intent is derive everything from
// XBase, so a program can catch this one exception type in main() and
// be assured no exception will propagate out of the program (or any
// other unit of granularity you want).

#ifndef SMBASE_EXC_H
#define SMBASE_EXC_H

#include "breaker.h"                   // breaker
#include "str.h"                       // string
#include "stringb.h"                   // stringb

#include <exception>                   // std::exception
#include <iosfwd>                      // std::ostream
#include <string>                      // std::string
#include <vector>                      // std::vector


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


// ------------------------------- XBase -------------------------------
/* This is the base class for all exceptions in smbase and the other
   projects of mine that use it.

   One service it provides is implementing `std::exception::what()`,
   which has an annoying signature due to not returning `std::string`.
   Derived classes of `XBase` instead override `getConflict()` (and
   optionally a few others), which returns a `std::string`, and this
   class saves the result in order to return it from `what()`.

   Another service is storing a sequence of context strings to make it
   easy to augment the exception object with useful information about
   the program state as it propagates up the stack toward the catch
   site.  (This is not a substitute for a stack trace; that is an
   entirely different concern.)

   TODO: It would be nice to have the ability to collect and store a
   stack trace here.
*/
class XBase : public std::exception {
private:     // data
  // This is what the `what()` function points at.  It is only populated
  // when that function is called.
  mutable std::string m_whatStorage;

protected:   // data
  // A sequence of English context phrases describing where an issue
  // arose or what the program was trying to do at the time.  The
  // phrases should be meaningful to the *user*, not only the programmer
  // (this is not a stack trace!).  It is ordered from outermost
  // (furthest from the conflict) to innermost (nearest to the
  // conflict).  It can be empty.
  std::vector<std::string> m_contexts;

public:      // methods
  // The `XBase` constructor does not take any arguments.  In the past
  // it accepted a `string` argument.  If you are porting older code
  // that relied on that, you probably want to inherit `XMessage`
  // instead.
  XBase() noexcept;

  XBase(XBase const &m) noexcept;
  XBase &operator=(XBase const &m) noexcept;

  virtual ~XBase();

  // `std::exception` overrides.  This is not meant to be further
  // overridden by subclasses.  It populates `m_whatStorage` with
  // `getMessage()` and returns `m_whatStorage.c_str()`.
  virtual char const *what() const noexcept override;

  // Construct a message suitable to be delivered to a human user in the
  // event that this exception is the cause of a user-visible error
  // (which may or may not be fatal).
  //
  // Default: Returns `getContext()` + ": =" + `getConflict()`, unless
  // the context is the empty string, in which case it returns only the
  // conflict.
  //
  virtual std::string getMessage() const;

  /* Return a properly punctuated English sentence that explains the
     conflict, i.e., what was expected and what was observed.

     Beware: This is generally called at the location that an exception
     is caught, which might be many levels above where it was thrown.
     There could be resources destroyed during unwinding that this
     method might naively want to use (for example, `SourceLocManager`
     if a `SourceLoc` is stored).  That might motivate inheriting from
     `XMessage` (below) instead of directly from `XBase` since the
     former creates and stores the exception message eagerly near the
     throw site.
  */
  virtual std::string getConflict() const = 0;

  // Return a context string for this exception, or the empty string if
  // there is no context available.
  //
  // Default: Return the strings in `m_contexts` separated by ": ".
  //
  virtual std::string getContext() const;

  // This is meant to be called from within a `catch` block that ends
  // with `throw;` in order to augment the exception object with
  // additional context.  It should be called with the innermost context
  // first and outermost context last so the final result begins with
  // the outermost context.
  //
  // Context strings can be simple nouns like file names, line:col
  // locations, etc.  They can also be phrases that describe what user
  // request or intermediate task was being performed.  The context is
  // *not* meant to be a tool for debugging--it should only contain
  // information that is meaningful and useful to the end user.  (Of
  // course, if the exception arises in a context where only a developer
  // could see it, like in unit tests, then debug information can be
  // appropriate.)
  //
  // Default: Prepend `context` to `m_contexts`.
  //
  virtual void prependContext(std::string const &context);

  // Although unusual, there may be cases where some piece of context
  // should be inserted as the new innermost context.
  //
  // Default: Append `context` to `m_contexts`.
  virtual void appendContext(std::string const &context);

  // This is a legacy alias for `getMessage()`.
  std::string why() const
    { return getMessage(); }

  // Print `getMessage()`.
  void insert(std::ostream &os) const;
  friend std::ostream& operator << (std::ostream &os, XBase const &obj)
    { obj.insert(os); return os; }
};


// This is used when we do not expect an exception to be thrown, and
// do not have a good recovery available, but it might happen, and we
// do not want to simply terminate.
//
// Print details about 'x' to stderr using DEV_WARNING (dev-warning.h).
void printUnhandled(XBase const &x);


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
  catch (XBase &x) {                  \
    printUnhandled(x);                \
  }

// Variant for functions that return a value.
#define GENERIC_CATCH_END_RET(retval) \
  }                                   \
  catch (XBase &x) {                  \
    printUnhandled(x);                \
    return retval;                    \
  }


// ----------------------------- XMessage ------------------------------
// Exception that just carries a conflict message.  It can be thrown
// when there isn't a realistic possibility of automatic recovery, or
// used as a base class for an exception class whose type conveys all
// of the discriminating information.
class XMessage : public XBase {
public:      // data
  // The conflict message.
  std::string m_message;

public:      // methods
  XMessage(std::string const &message) noexcept;
  XMessage(XMessage const &obj) noexcept;
  XMessage &operator=(XMessage const &obj) noexcept;

  // Returns `m_message`.
  virtual std::string getConflict() const override;
};


// Equivalent to THROW(XMessage(msg)).
void xmessage(std::string const &msg) NORETURN;

// For compatibility with older code, define this alias.
inline void xbase(std::string const &msg) NORETURN;
inline void xbase(std::string const &msg) { xmessage(msg); }


// Define a subclass of XMessage.  All methods are inline.
#define DEFINE_XMESSAGE_SUBCLASS(SubclassName)                           \
  class SubclassName : public XMessage {                                 \
  public:                                                                \
    SubclassName(std::string const &message) noexcept                    \
      : XMessage(message) {}                                             \
    SubclassName(SubclassName const &obj) noexcept = default;            \
    SubclassName &operator=(SubclassName const &obj) noexcept = default; \
  } /* user ; */


// In the original definition of `XBase`, it carried a conflict message,
// and some exceptions used this macro to derive from it.  For now I am
// keeping this macro but redirecting it to `XMessage`.
#define DEFINE_XBASE_SUBCLASS(SubclassName) \
  DEFINE_XMESSAGE_SUBCLASS(SubclassName)


// ------------------------------ XAssert ------------------------------
// Thrown by `x_assert_fail`, which is declared in xassert.h.
// Throwing this corresponds to detecting a bug in the program.
class XAssert : public XBase {
  string condition;          // text of the failed condition
  string filename;           // name of the source file
  int lineno;                // line number

public:
  XAssert(rostring cond, rostring fname, int line);
  XAssert(XAssert const &obj);
  ~XAssert();

  rostring cond() const { return condition; }
  rostring fname() const { return filename; }
  int line() const { return lineno; }

  // XBase methods.
  virtual std::string getConflict() const override;
};


// ---------------------- XFormat -------------------
// throwing this means a formatting error has been detected
// in some input data; the program cannot process it, but it
// is not a bug in the program
class XFormat : public XMessage {
public:      // methods
  XFormat(rostring cond);
  XFormat(XFormat const &obj);
  ~XFormat();

  // Compatibility alias.
  std::string cond() const { return getMessage(); }
};

// compact way to throw an XFormat
void xformat(rostring condition) NORETURN;

#define xformatsb(msg) xformat(stringb(msg))

// convenient combination of condition and human-readable message
#define checkFormat(cond, message) \
  ((cond)? (void)0 : xformat(message))

// assert-like interface to XFormat
void formatAssert_fail(char const *cond, char const *file, int line) NORETURN;

#define formatAssert(cond) \
  ((cond)? (void)0 : formatAssert_fail(#cond, __FILE__, __LINE__))


// 2022-07-18: There was previously a class called XOpen, and another
// called XOpenEx.  These have been removed in favor of xSysError.


// ------------------- XUnimp ---------------------
// thrown in response to a condition that is in principle
// allowed but not yet handled by the existing code
class XUnimp : public XMessage {
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
class XFatal : public XMessage {
public:
  XFatal(rostring msg);
  XFatal(XFatal const &obj);
  ~XFatal();
};

void throw_XFatal(rostring msg) NORETURN;
#define xfatal(msg) throw_XFatal(stringb(msg))


#endif // SMBASE_EXC_H

