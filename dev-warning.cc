// dev-warning.cc
// code for dev-warning.h

#include "dev-warning.h"               // this module

#include "sm-iostream.h"               // cerr

#include <stdlib.h>                    // abort


bool g_abortUponDevWarning = false;


void devWarning(char const *file, int line, char const *msg)
{
  // For now we just print to stderr.
  cerr << "DEV_WARNING: " << file << ':' << line << ": " << msg << endl;

  if (g_abortUponDevWarning) {
    cerr << "Aborting due to DEV_WARNING." << endl;
    abort();
  }
}


// EOF
