// xassert.h            see license.txt for copyright and terms of use
// xassert is an assert()-like macro that throws an exception when it
// fails, instead of calling abort().

#ifndef SMBASE_XASSERT_H
#define SMBASE_XASSERT_H

#include "sm-macros.h"  // NORETURN


OPEN_NAMESPACE(smbase)


// this functions accepts raw 'char const *' instead of 'rostring'
// because I do not want this interface to depend on str.h, and also
// because I do not want the many call sites to have the overhead
// of constructing and destructing temporary objects
//
// This is defined in exc.cc.
void x_assert_fail(char const *cond, char const *file, int line) NORETURN;

// Ordinary 'xassert' *can* be turned off, but the nominal intent
// is that it be left on, under the "ship what you test" theory.
// I advocate using NDEBUG_NO_ASSERTIONS only as a way to gauge the
// performance impact of the existing assertions.
#if !defined(NDEBUG_NO_ASSERTIONS)
  #define xassert(cond) \
    ((cond)? (void)0 : smbase::x_assert_fail(#cond, __FILE__, __LINE__))
#else
  #define xassert(cond) ((void)0)
#endif

// Here's a version which will turn off with ordinary NDEBUG.  It
// is for more expensive checks that need not ship.
#if !defined(NDEBUG) && !defined(NDEBUG_NO_ASSERTIONS)
  #define xassertdb(cond) xassert(cond)
#else
  #define xassertdb(cond) ((void)0)
#endif

// call when state is known to be bad; will *not* return
#define xfailure(why) smbase::x_assert_fail(why, __FILE__, __LINE__)


// This requires 'stringbc', which is declared in 'stringb.h'.  I
// created this macro to make it easier to convert code that was using
// 'stringc' since it allows:
//
//   xfailure(stringc << various << things)
//
// to become:
//
//   xfailure_stringbc(various << things)
//
// which can be accmplished simply by replacing "xfailure(stringc << "
// with "xfailure_stringbc(".  In particular, that does not require any
// balancing of parentheses, which is hard when doing regex-based
// replacement.
//
#define xfailure_stringbc(stuff) xfailure(stringbc(stuff))


// 'xassert_once' is an assertion that is only checked the first time it
// is executed.
#if defined(NDEBUG_NO_ASSERTIONS)
  #define xassert_once(cond) ((void)0)
#else
  #define xassert_once(cond)       \
    do {                           \
      static bool checked = false; \
      if (!checked) {              \
        checked = true;            \
        xassert(cond);             \
      }                            \
    } while (0)
#endif


/* Assert a condition that, at the call site, is the caller's
   responsibility to ensure.

   The idea is to use `xassertPrecondition` at the top of a function,
   after which point ordinary `xassert` checks things that should be
   logical consequences of the preconditions.  That is, if
   `xassertPrecondition` fails, the bug is in the calling code, while if
   `xassert` fails, it is in the code containing the assertion.

   This is an experimental idea that I've only begun to pursue, so the
   above convention is not widespread.

   If this works well I might create a dedicated class to carry the
   exception, and/or modify the message, but for now I think it's enough
   to have a clear indication in the code of which ones are checking
   preconditions.
*/
#define xassertPrecondition(cond) xassert(cond)

// This is used when, for example, the function begins with a `switch`
// and one of the cases corresponds to a violated precondition.
#define xfailurePrecondition(why) xfailure(why)


/* Assert a condition that should be a data structure invariant.

   This is meant to be used in `selfCheck()` methods that check
   invariants.
*/
#define xassertInvariant(cond) xassert(cond)

// Used when a spot in the code can only be reached if data structure
// invariants were previously broken.
#define xfailureInvariant(why) xfailure(why)


// Quick note: one prominent book on writing code recommends that
// assertions *not* include the failure condition, since the file
// and line number are sufficient, and the condition string uses
// memory.  The problem is that sometimes a compiled binary is
// out of date w/respect to the code, and line numbers move, so
// the condition string provides a good way to find the right
// assertion.


/*
  Why throw an exception after an assertion?

  The standard assert() calls abort() after printing its message.
  This is like throwing an exception all the way to the calling
  process.  This is fine if programs are small.

  But when a program is large enough, it may contain subsystems at
  several layers, such that a higher level module is capable of
  recovering from the failure of a lower level module.  Using abort(),
  one would have to resort to catching signals, which is messy.

  An exception is much nicer to catch, and has the added benefit that
  intermediate layers can catch and rethrow, appending little bits of
  context, if they want to make the message more informative.

  In most of my programs, the 'XAssert' exception is only caught in
  main() (implicitly, by catching 'XBase'), and hence 'xassert' acts
  very much like 'assert'.  But by using 'xassert' consistenty, any
  time I *do* have a large program with recovery, all the lower-level
  modules are all ready to cooperate.

  Speaking of recovery: Be aware that when a module fails an
  assertion, its internal state is most likely inconsistent.  Recovery
  actions need to be fairly conservative about what code gets
  re-entered and state re-used after a failure.  This is no different
  than with 'assert', as a program could have inconsistent state *on
  disk* that gets reactivated upon being restarted, but persistent
  (across process boundaries) inconsistent state is simply less
  common.

*/


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_XASSERT_H

