// test.h            see license.txt for copyright and terms of use
// utilities for test code
// Scott McPeak, 1999  This file is public domain.

#ifndef __TEST_H
#define __TEST_H

#include "exc.h"           // xBase
#include "nonport.h"       // getMilliseconds
#include "sm-iostream.h"   // cout
#include "str.h"           // stringb

#include <stdio.h>         // printf


// reports uncaught exceptions
//
// 12/30/02: I used to print "uncaught exception: " before
// printing the exception, but this is meaningless to the
// user and the message usually has enough info anyway
#define USUAL_MAIN                              \
void entry();                                   \
int main()                                      \
{                                               \
  try {                                         \
    entry();                                    \
    return 0;                                   \
  }                                             \
  catch (xBase &x) {                            \
    cout << x << endl;                          \
    return 4;                                   \
  }                                             \
}

// same as above, but with command-line args
#define ARGS_MAIN                               \
void entry(int argc, char *argv[]);             \
int main(int argc, char *argv[])                \
{                                               \
  try {                                         \
    entry(argc, argv);                          \
    return 0;                                   \
  }                                             \
  catch (xBase &x) {                            \
    cout << x << endl;                          \
    return 4;                                   \
  }                                             \
}


// convenient for printing the value of a variable or expression
#define PVAL(val) cout << #val << " = " << (val) << endl

// Same, but also print the file and line.
#define DEBUG_PVAL(val) cout << __FILE__ << ":" << __LINE__ << \
  ": " #val << " = " << (val) << endl


// easy way to time a section of code
class TimedSection {
  char const *name;
  long start;

public:
  TimedSection(char const *n) : name(n) {
    start = getMilliseconds();
  }
  ~TimedSection() {
    cout << name << ": " << (getMilliseconds() - start) << " msecs\n";
  }
};


template <class T>
void expectEq(char const *label, T const &actual, T const &expect)
{
  if (expect != actual) {
    cout << "mismatched " << label << ':' << endl;
    cout << "  actual: " << actual << endl;
    cout << "  expect: " << expect << endl;
    xfailure(stringb("mismatched " << label));
  }
}

#define EXPECT_EQ(actual, expect) \
  expectEq(#actual, actual, expect) /* user ; */


#endif // __TEST_H
