// test-run-process.cc
// Tests for run-process module.

#include "run-process.h"               // module under test

#include "exc.h"                       // xBase


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


static void unit_test()
{
  #define RUN_ONE(expect, ...)               \
  {                                          \
    char const *cmd[] = {__VA_ARGS__, NULL}; \
    runOne(expect, cmd);                     \
  }

  RUN_ONE("Exit 0", "true");
  RUN_ONE("Exit 1", "false");
  RUN_ONE("Exit 3", "sh", "-c", "exit 3");
  RUN_ONE("Signal 15", "sh", "-c", "echo hi; kill $$");
}


int main(int argc, char **argv)
{
  try {
    if (argc <= 1) {
      cout << "usage: " << argv[0] << " program [args...]\n"
              "  or\n"
              "       " << argv[0] << " --unit-test\n";
      return 2;
    }

    std::vector<string> command;
    for (int i=1; i < argc; i++) {
      command.push_back(string(argv[i]));
    }

    if (command[0] == "--unit-test") {
      unit_test();
      return 0;
    }

    RunProcess rproc;
    rproc.setCommand(command);
    rproc.runAndWait();

    if (rproc.exitedNormally()) {
      cout << "Exited with code " << rproc.getExitCode() << endl;
    }
    else if (rproc.interrupted()) {
      cout << "Interrupted by signal " << rproc.getSignal() << endl;
    }
    else {
      cout << "Died by signal " << rproc.getSignal() << endl;
    }
  }
  catch (xBase &x) {
    cout << "exception: " << x.why() << endl;
    return 4;
  }
}


// EOF
