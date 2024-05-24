// run-process-test.cc
// Tests for run-process module.

#include "run-process.h"               // module under test

#include "exc.h"                       // XBase
#include "sm-platform.h"               // PLATFORM_IS_POSIX
#include "string-utils.h"              // splitNonEmpty
#include "xassert.h"                   // xassert

#include <cstdlib>                     // std::exit


static void oneBwcl(string expect, char const **argv)
{
  std::vector<string> command;
  for (; *argv; argv++) {
    command.push_back(string(*argv));
  }

  std::vector<char> actualVec;
  RunProcess::buildWindowsCommandLine(actualVec, command);
  string actual(actualVec.data(), actualVec.size() - 1);

  if (actual != expect) {
    cout << "actual: " << actual << endl;
    cout << "expect: " << expect << endl;
    xfailure("actual and expect disagree");
  }

  // Optionally validate by running these against an external program.
  // The results have to be manually inspected.
  char const *validate = getenv("VALIDATE");
  if (validate) {
    command[0] = string(validate);

    cout << "Passing arguments:\n";
    for (size_t i=0; i < command.size(); i++) {
      cout << "  [" << i << "]: " << command[i] << endl;
    }

    RunProcess rproc;
    rproc.setCommand(command);
    rproc.runAndWait();
  }
}


static void testBuildWindowsCommandLine()
{
  cout << "-- testBuildWindowsCommandLine --\n";

  #define ONE_BWCL(expect, ...)              \
  {                                          \
    char const *cmd[] = {__VA_ARGS__, NULL}; \
    oneBwcl(expect, cmd);                    \
  }

  // Examples based on those from the MSDN docs.
  //
  // The initial 'x' is a placeholder for the program name, which is
  // not included in those examples.
  //
  // The first column is not the same as in MSDN because, for me, it is
  // an *output*, so I am showing how I intend to encode the columns to
  // the right, rather than exploring all possible ways of encoding
  // them.
  ONE_BWCL("\"x\" \"a b c\" \"d\" \"e\"",        "x", "a b c",    "d",     "e");
  ONE_BWCL("\"x\" \"ab\\\"c\" \"\\\\\" \"d\"",   "x", "ab\"c",    "\\",    "d");
  ONE_BWCL("\"x\" \"a\\\\\\b\" \"de fg\" \"h\"", "x", "a\\\\\\b", "de fg", "h");
  ONE_BWCL("\"x\" \"a\\\\b c\" \"d\" \"e\"",     "x", "a\\\\b c", "d",     "e");
  ONE_BWCL("\"x\" \"ab\\\" c d\"",               "x", "ab\" c d");

  // My own examples.

  // Space in program name.
  ONE_BWCL("\"a b\" \"c\"",
    "a b", "c");

  // Backslashes in program name.
  ONE_BWCL("\"e:\\foo\\bar\\baz zoo\\goo.exe\" \"c\"",
    "e:\\foo\\bar\\baz zoo\\goo.exe", "c");

  // Backslash at end of program name, which should also not be escaped,
  // although I don't think this ever forms a valid program name.
  ONE_BWCL("\"prog\\\" \"x\"",
    "prog\\", "x");

  // Argument with a backslash followed by a quote.
  ONE_BWCL("\"prog\" \"x\\\\\\\"y\"",
    "prog", "x\\\"y");
  ONE_BWCL("\"prog\" \"x\\\\\\\\\\\"y\"",
    "prog", "x\\\\\"y");

  // Handling of second and later arguments.
  ONE_BWCL("\"prog\" \"a b\" \"c\\d\" \"e\\\"f\"",
    "prog", "a b", "c\\d", "e\"f");

  try {
    ONE_BWCL("", "program with \" quote");
    xfailure("should have failed!");
  }
  catch (xFormat &x) {
    cout << "as expected: " << x.why() << endl;
  }
}


static void runOne(string expect, char const **argv)
{
  cout << "command:";
  std::vector<string> command;
  for (; *argv; argv++) {
    cout << " " << *argv;
    command.push_back(string(*argv));
  }
  cout << endl;

  RunProcess rproc;
  rproc.setCommand(command);
  rproc.runAndWait();

  string actual = rproc.exitDescription();
  cout << "actual: " << actual << endl;

  if (actual != expect) {
    cout << "expect: " << expect << endl;
    xfailure("actual and expect disagree");
  }
}


static void testRun()
{
  cout << "-- testRun --\n";

  #define RUN_ONE(expect, ...)               \
  {                                          \
    char const *cmd[] = {__VA_ARGS__, NULL}; \
    runOne(expect, cmd);                     \
  }

  RUN_ONE("Exit 0", "true");
  RUN_ONE("Exit 1", "false");
  RUN_ONE("Exit 3", "sh", "-c", "exit 3");
  if (PLATFORM_IS_POSIX) {
    // Only run this on POSIX since Windows behavior is probably
    // unspecified.
    RUN_ONE("Signal 15", "sh", "-c", "echo hi; kill $$");
  }

  RunProcess::check_run(std::vector<string>{"true"});
  try {
    RunProcess::check_run(std::vector<string>{"false"});
    xfailure("should have failed");
  }
  catch (XFatal &x) {
    cout << "as expected: " << x.why() << endl;
  }
}


static void testAborted()
{
  cout << "-- testAborted --\n";
  RunProcess rproc;
  rproc.setCommand(std::vector<string>{"./call-abort.exe"});

  if (PLATFORM_IS_POSIX) {
    rproc.runAndWait();
    xassert(rproc.aborted());
  }
  else {
    // The 'aborted' function does not work on Windows, so just skip it.

    // Under winlibs mingw64, 'abort()' pops up the annoying dialog box
    // about reporting the problem to Microsoft, so do not even run the
    // child process.
  }
}


static void unit_test()
{
  testBuildWindowsCommandLine();
  testRun();
  testAborted();
}


// Called from unit-tests.cc.
void test_run_process()
{
  try {
    if (char const *cmdline = getenv("RUN_PROCESS_TEST_CMDLINE")) {
      std::vector<string> command = splitNonEmpty(cmdline, ' ');

      RunProcess rproc;
      rproc.setCommand(command);
      rproc.runAndWait();
      cout << rproc.exitDescription() << endl;
    }
    else {
      unit_test();
    }
  }
  catch (XBase &x) {
    cout << "exception: " << x.why() << endl;
    std::exit(4);
  }
}


// EOF
