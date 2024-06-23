// autofile.h            see license.txt for copyright and terms of use
// AutoFILE, a simple wrapper around FILE* to open it or throw
// an exception, and automatically close it.

// I have chosen to use 'char const *' here instead of 'rostring'
// to reduce dependencies on other modules ...

#ifndef AUTOFILE_H
#define AUTOFILE_H

#include <stdio.h>      // FILE


// Like 'fopen', but throw an XSysError exception (see syserr.h) on
// failure instead of returning NULL.
FILE *xfopen(char const *fname, char const *mode);


// automatically close a file in the destructor
class AutoFclose {
private:       // data
  FILE *fp;

private:       // disallowed
  AutoFclose(AutoFclose&);
  void operator=(AutoFclose&);

public:
  AutoFclose(FILE *f) : fp(f) {}
  ~AutoFclose() { fclose(fp); }

  // may as well allow access to my storage
  FILE *getFP() { return fp; }
};


// simple wrapper on FILE*
class AutoFILE : private AutoFclose {
public:
  // Open, throwing XSysError on failure.
  AutoFILE(char const *fname, char const *mode);

  // close the file
  ~AutoFILE();

  // behave like FILE* in between
  operator FILE* () { return getFP(); }
};


#endif // AUTOFILE_H
