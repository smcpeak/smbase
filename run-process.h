// run-process.h
// Run a process.

#ifndef SMBASE_RUN_PROCESS_H
#define SMBASE_RUN_PROCESS_H

#include "macros.h"                    // NO_OBJECT_COPIES
#include "str.h"                       // string

#include <vector>                      // std::vector


// Object to set up a process to run and inspect the result.
class RunProcess {
  NO_OBJECT_COPIES(RunProcess);

private:     // data
  // Command to run.
  std::vector<string> m_command;

  // True if we ran the program and it terminated.
  bool m_terminated;

  // True for a normal exit.
  bool m_exitedNormally;

  // The exit code if 'm_exitedNormally', the signal otherwise.
  unsigned m_exitCodeOrSignal;

public:      // methods
  RunProcess();
  ~RunProcess();

  // Set the program to run (first string) and its arguments (subsequent
  // strings).
  void setCommand(std::vector<string> const &command);

  // Run the program and wait for it to terminate.
  void runAndWait();

  // True if the program exited normally, i.e., it called exit().  False
  // if it terminated due to a signal.
  bool exitedNormally() const;

  // If 'exitedNormally()', the value passed to exit().
  unsigned getExitCode() const;

  // If '!exitedNormally()', the signal number.
  unsigned getSignal() const;

  // True if the program was interrupted by Ctrl-C or similar.  This
  // implies '!exitedNormally()'.  This is useful in some cases where
  // the parent wants to bail out if the child is interrupted.
  bool interrupted() const;

  // One of:
  //   - "Exit N"
  //   - "Interrupted"
  //   - "Signal N"
  string exitDescription() const;

  // Apply the bizarre Windows API quoting rules to 'command' in order
  // to form 'commandLine' that can be passed to CreateProcess.  The
  // resulting vector ends with a NUL byte.
  static void buildWindowsCommandLine(std::vector<char> &commandLine,
    std::vector<string> const &command);
};


#endif // SMBASE_RUN_PROCESS_H
