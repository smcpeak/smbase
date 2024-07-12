// autofile.cc            see license.txt for copyright and terms of use
// code for autofile.h

#include "autofile.h"     // this module
#include "str.h"          // string::string [h]
#include "syserr.h"       // xsyserror

using namespace smbase;


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


// EOF

