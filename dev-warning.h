// dev-warning.h
// "Developer warning" mechanism.

#ifndef DEV_WARNING_H
#define DEV_WARNING_H

#include "str.h"                       // stringbc


// When true, any call to 'devWarning' will abort().  This is meant for
// use in unit tests so the test fails if a warning is printed.  It is
// initially false.
extern bool g_abortUponDevWarning;


// Print or log a warning originating at file/line.  This is something
// that the developer thinks should not or cannot happen, but is
// recoverable (no need to abort or throw), and the end user would not
// know or care about it.
void devWarning(char const *file, int line, char const *msg);


// Macro for convenient usage.
#define DEV_WARNING(msg) \
  devWarning(__FILE__, __LINE__, stringbc(msg)) /* user ; */



#endif // DEV_WARNING_H
