// autofile.cc            see license.txt for copyright and terms of use
// code for autofile.h

#include "autofile.h"     // this module
#include "syserr.h"       // xsyserror

#include <errno.h>        // errno
#include <string.h>       // strerror


FILE *xfopen(char const *fname, char const *mode)
{
  FILE *ret = fopen(fname, mode);
  if (!ret) {
    xsyserror("open", fname);
  }

  return ret;
}


AutoFILE::AutoFILE(char const *fname, char const *mode)
  : AutoFclose(xfopen(fname, mode))
{}

AutoFILE::~AutoFILE()
{
  // ~AutoFclose closes the file
}


// -------------------- test code -------------------
#ifdef TEST_AUTOFILE

#include "sm-test.h"      // ARGS_MAIN
#include "sm-iostream.h"  // cout

void entry(int argc, char *argv[])
{
  if (argc < 2) {
    cout << "usage: " << argv[0] << " filename [mode]\n";
    return;
  }

  char const *mode = "r";
  if (argc >= 3) {
    mode = argv[2];
  }

  cout << "about to open " << argv[1] << " with mode " << mode << endl;

  {
    AutoFILE fp(argv[1], mode);
    cout << argv[1] << " is now open" << endl;
  }

  cout << argv[1] << " is now closed" << endl;
}

ARGS_MAIN


#endif // TEST_AUTOFILE
