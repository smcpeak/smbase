// nonport-test.cc
// Tests for nonport.

#include "nonport.h"                   // module under test

#include "save-restore.h"              // SET_RESTORE
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // dummy_printf

#include <errno.h>                     // errno
#include <stdarg.h>                    // va_list
#include <stdio.h>                     // printf
#include <stdlib.h>                    // exit, getenv
#include <string.h>                    // strcmp

OPEN_ANONYMOUS_NAMESPACE


bool verbose = false;

#define printf (verbose? printf : dummy_printf)


// helper for testing applyToCwdFiles
bool printFirst10(char const *name, void *extra)
{
  int &count = *((int*)extra);
  count++;
  if (count <= 10) {
    printf("  %s\n", name);
    return true;    // continue
  }
  else {
    return false;   // stop
  }
}


bool printIt(char const *name, void*)
{
  printf("%s\n", name);
  return true;    // continue
}


void testingFail(char const *call, char const *ctx)
{
  printf("FAIL: call=%s, ctx=%s, errno=%d\n",
         call, (ctx? ctx : "(null)"), errno);
}


void nprintfVector(char const *format, ...)
{
  va_list args;

  // run vnprintf to obtain estimate
  va_start(args, format);
  int est = vnprintf(format, args);
  va_end(args);

  // make that much space
  char *buf = new char[est+1 + 50 /*safety margin*/];

  // run the real vsprintf
  va_start(args, format);
  int len = vsprintf(buf, format, args);
  va_end(args);

  if (len > est) {
    printf("nprintf failed to conservatively estimate!\n");
    printf("    format: %s\n", format);
    printf("  estimate: %d\n", est);
    printf("    actual: %d\n", len);
    exit(2);
  }

  if (len != est) {
    // print the overestimates; they shouldn't be much noise in the
    // common case, but might hint at a problem earlier than I'd
    // otherwise notice
    printf("nprintf overestimate:\n");
    printf("    format: %s\n", format);
    printf("  estimate: %d\n", est);
    printf("    actual: %d\n", len);
  }

  delete[] buf;
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_nonport()
{
  SET_RESTORE(nonportFail, testingFail);

  verbose = !!getenv("NONPORT_TEST_VERBOSE");

  char s[4];
  s[0] = '-';
  s[1] = 'l';
  s[2] = 's';
  s[3] = 0;
  if (0!=strcmp(s, "-ls")) {
    printf("strcmp failed!\n");
    exit(4);
  }

  // Process envvar switches.
  if (getenv("NONPORT_TEST_LS")) {
    // do an ls, and bail
    applyToCwdContents(printIt);
    return;
  }
  bool interactive = false;
  if (getenv("NONPORT_TEST_INTERACTIVE")) {
    interactive = true;
  }

  // trying to figure out why backspace sometimes gives ^? crap
  // (turns out Konsole sometimes sends ^? in response to BS,
  // even when the option looks unchecked)
  //char buf[80];
  //printf("type stuff and try backspace: ");
  //gets(buf);
  //printf("%s (%d chars)\n", buf, strlen(buf));
  //return 0;

  long startTime = getMilliseconds();

  if (interactive) {
    printf("Type some characters; you should see each\n"
           "character echoed once as you type it (q to stop):\n");
    setRawMode(true);
    char ch;
    do {
      ch = getConsoleChar();
      printf("%c", ch);
    } while (ch != 'q');

    setRawMode(false);

    printf("\n\nYou typed for %ld milliseconds\n",
           getMilliseconds() - startTime);
  }

  limitFileAccess("chmod.test");

  printf("if the current dir contains a file called "
         "chmod.test, I just attempted to limit\n"
         "its access to just the owner\n");

  createDirectory("test.dir");

  // test chdir, which also implicitly tests mkdir
  bool didFirst=false;
  if (!changeDirectory("test.dir") || (didFirst=true, false) ||
      !changeDirectory("..")) {
    printf("failed while trying to chdir to %s\n",
           (didFirst? ".." : "test.dir"));
  }

  // more straightforward
  if (!fileOrDirectoryExists("test.dir")) {
    printf("test.dir didn't get created?\n");
  }

  printf("what's more, I just tried to mkdir & chdir test.dir\n");

  // Test getFileModificationTime (crudely).
  {
    int64_t t;
    if (!getFileModificationTime("nonport.cc", t /*OUT*/)) {
      printf("getFileModificationTime(\"nonport.cc\") failed!\n");
      exit(4);
    }
    printf("modification time of nonport.cc: %ld\n",
           (long)t);
  }

  // test ensurePath
  if (!ensurePath("test.dir/a/b/c/d", false /*isDirectory*/)) {
    printf("ensurePath test.dir/a/b/c/d failed\n");
  }

  // try to list partial directory contents
  printf("listing of first 10 files in this directory:\n");
  {
    int count = 0;
    applyToCwdContents(printFirst10, &count);
  }

  // test date function
  {
    int m, d, y;
    getCurrentDate(m, d, y);

    printf("I think the date is (m/d/yyyy): %d/%d/%d\n",
           m, d, y);
  }

  // test sleep (mostly just to make sure it doesn't segfault)
  //
  // This test is a waste of time.
  //printf("sleeping for 1 second...\n");
  //portableSleep(1);

  printf("sleeping for 10 ms...\n");
  sleepForMilliseconds(10);

  // test user name
  char buf[80];
  getCurrentUsername(buf, 80);
  printf("current user name is: %s\n", buf);

  if (interactive) {
    // test nonecho reading
    printf("Type something and press Enter; it won't be echoed (yet):\n");
    readNonechoString(buf, 80, "  > ");
    printf("You typed: %s\n", buf);
  }

  // test random stuff
  printf("hasSystemCryptoRandom: ");
  if (!hasSystemCryptoRandom()) {
    printf("no\n");
  }
  else {
    printf("yes\n");

    printf("three random numbers: %u %u %u\n",
           getSystemCryptoRandom(),
           getSystemCryptoRandom(),
           getSystemCryptoRandom());
  }

  printf("testing nprintf...\n");
  nprintfVector("simple");
  nprintfVector("a %s more", "little");
  nprintfVector("some %4d more %s complicated %c stuff",
                33, "yikes", 'f');
  nprintfVector("%f", 3.4);

  printf("nonport works\n");
}


// EOF
