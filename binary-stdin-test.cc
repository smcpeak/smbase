// binary-stdin-test.cc
// Tests for binary-stdin.

#include "binary-stdin.h"              // module under test

// smbase
#include "autofile.h"                  // AutoFILE
#include "exc.h"                       // XBase, xfatal
#include "sm-iostream.h"               // cout, cin
#include "stringb.h"                   // stringb
#include "syserr.h"                    // smbase::xsyserror
#include "xassert.h"                   // xassert

// libc++
#include <vector>                      // std::vector

// libc
#include <string.h>                    // strcmp

// POSIX
#include <unistd.h>                    // read, write

// According to POSIX 6 and 7, <unistd.h> should be sufficient to get
// `ssize_t`.
// IWYU pragma: no_include <sys/types.h>

using namespace smbase;


static std::vector<unsigned char> allbytes()
{
  std::vector<unsigned char> vec;
  for (int i=0; i < 256; i++) {
    vec.push_back((unsigned char)i);
  }
  return vec;
}


// Read all available data from 'fp' into 'vec'.  'srcname' is used for
// providing additional context in error reporting.
static void freadAll(std::vector<unsigned char> &vec, FILE *fp,
                     char const *srcname)
{
  unsigned char buf[1024];
  while (size_t numRead = fread(buf, 1, 1024, fp)) {
    if (ferror(fp)) {
      xsyserror("read", srcname);
    }

    vec.insert(vec.end(), buf, buf+numRead);
  }
}


static void readSource(std::vector<unsigned char> &vec,
                       char const *srcname)
{
  if (0==strcmp(srcname, "allbytes")) {
    vec = allbytes();
  }

  else if (0==strcmp(srcname, "read0")) {
    unsigned char buf[1024];
    while (ssize_t numRead = read(0, buf, 1024)) {
      if (numRead < 0) {
        xsyserror("read", "stdin");
      }

      vec.insert(vec.end(), buf, buf+numRead);
    }
  }

  else if (0==strcmp(srcname, "cin_read")) {
    unsigned char buf[1024];
    while (true) {
      cin.read((char*)buf, 1024);
      vec.insert(vec.end(), buf, buf+cin.gcount());
      if (cin.eof()) {
        break;
      }
      if (cin.fail()) {
        xsyserror("read", "cin");
      }
    }
  }

  else if (0==strcmp(srcname, "fread_stdin")) {
    freadAll(vec, stdin, srcname);
  }

  else {
    // Treat 'srcname' as a file name.
    AutoFILE fp(srcname, "rb");

    freadAll(vec, fp, srcname);
  }
}


// Write all of 'vec' to 'fp'.  'destname' is used for error reporting
// context.
static void fwriteAll(std::vector<unsigned char> const &vec,
                      FILE *fp, char const *destname)
{
  size_t written = 0;
  while (written < vec.size()) {
    size_t res = fwrite(vec.data()+written, 1, vec.size()-written, fp);
    if (ferror(fp)) {
      xsyserror("write", destname);
    }
    if (res == 0) {
      xfatal(stringb("Writing to " << destname <<
                     " unexpectedly hit EOF after " <<
                     written << " bytes."));
    }

    written += res;
  }
}


static void writeDestination(std::vector<unsigned char> const &vec,
                             char const *destname)
{
  if (0==strcmp(destname, "allbytes")) {
    // Require that 'vec' be 'allbytes'.  Don't actually write anything.
    xassert(vec == allbytes());
  }

  else if (0==strcmp(destname, "write1")) {
    size_t written = 0;
    while (written < vec.size()) {
      ssize_t res = write(1, vec.data()+written, vec.size()-written);
      if (res < 0) {
        xsyserror("write", "stdout");
      }
      if (res == 0) {
        xfatal(stringb("Writing to stdout unexpectedly hit EOF after " <<
                       written << " bytes."));
      }
      written += res;
    }
  }

  else if (0==strcmp(destname, "cout_write")) {
    // No loop is required for ostream::write.
    cout.write((char*)vec.data(), vec.size());
    if (cout.fail()) {
      xsyserror("write", "cout");
    }
  }

  else if (0==strcmp(destname, "fwrite_stdout")) {
    fwriteAll(vec, stdout, destname);
  }

  else {
    // Treat 'destname' as a file name'.
    AutoFILE fp(destname, "wb");

    fwriteAll(vec, fp, destname);
  }
}


int innerMain(int argc, char **argv)
{
  setStdinToBinary();
  setStdoutToBinary();

  // The Makefile target out/binary-stdin-test.ok invokes this program.
  if (argc != 3) {
    cerr << "usage: " << argv[0] << " <srcname> <destname>\n";
    return 2;
  }

  char const *srcname = argv[1];
  char const *destname = argv[2];

  std::vector<unsigned char> vec;
  readSource(vec, srcname);

  writeDestination(vec, destname);

  return 0;
}


int main(int argc, char **argv)
{
  try {
    innerMain(argc, argv);
  }
  catch (XBase &x) {
    cerr << x.why() << endl;
    return 2;
  }
}


// EOF
