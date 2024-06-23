// breaker.h            see license.txt for copyright and terms of use
// Function for putting a breakpoint in, to get debugger control just
// before an exception is thrown.

#ifndef SMBASE_BREAKER_H
#define SMBASE_BREAKER_H

extern "C" void breaker();

// bassert = breaker assert; failure simply calls breaker, which is
// a breakpoint in the debugger and is ignored when not in debugger;
// useful mainly for places I want to ensure something is true during
// initial testing, but after that it's ok if it's false
template <class T>          // allow possibly null pointers, etc
inline void bassert(T cond)
{
  if (!cond) {
    breaker();
  }
}


// this will call breaker on the first pass, but not any subsequent (unless
// it's called MAXINT*2 times...)
#define BREAK_FIRST_PASS     \
  {                          \
    static int passCount=0;  \
    bassert(passCount++);    \
  } /*no semicolon*/


#endif // SMBASE_BREAKER_H
