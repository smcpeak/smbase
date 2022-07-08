// run-process.cc
// Code for run-process.h

#include "run-process.h"               // this module

#include "sm-iostream.h"               // cout, cerr
#include "sm-posix.h"                  // fork, exec, etc.
#include "sm-windows.h"                // Windows API
#include "syserr.h"                    // xsyserror
#include "vector-utils.h"              // accumulateWith
#include "xassert.h"                   // xassert

#include <errno.h>                     // errno
#include <string.h>                    // strerror


RunProcess::RunProcess()
  : m_command(),
    m_terminated(false),
    m_exitedNormally(false),
    m_exitCodeOrSignal(0)
{}


RunProcess::~RunProcess()
{}


void RunProcess::setCommand(std::vector<string> const &command)
{
  xassert(!command.empty());
  m_command = command;
}


void RunProcess::runAndWait()
{
  xassert(!m_command.empty());

  // Flush output streams before forking or blocking.
  cout.flush();
  cerr.flush();

  if (PLATFORM_IS_WINDOWS) {
    // Do not pass an explicit application name.  That way we get PATH
    // searching automatically.
    LPCSTR lpApplicationName = NULL;

    // Construct a string containing the program and all of its
    // arguments.
    std::vector<char> commandLine;
    buildWindowsCommandLine(commandLine, m_command);

    // I want the invoked process to be able to read my stdin and write
    // to my stdout/stderr.
    BOOL bInheritHandles = TRUE;

    // It doesn't look like any of the flags applies to what I want.
    DWORD dwCreationFlags = 0;

    // Child will inherit the parent environment.
    LPVOID lpEnvironment = NULL;

    // Use parent's current directory.
    LPCSTR lpCurrentDirectory = NULL;

    STARTUPINFOA startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo;
    memset(&processInfo, 0, sizeof(processInfo));

    // Start the child process.
    BOOL cpRes = CreateProcessA(
      lpApplicationName,
      commandLine.data(),   // lpCommandLine
      NULL,                 // lpProcessAttributes
      NULL,                 // lpThreadAttributes
      bInheritHandles,
      dwCreationFlags,
      lpEnvironment,
      lpCurrentDirectory,
      &startupInfo,         // lpStartupInfo
      &processInfo          // lpProcessInformation
    );
    if (!cpRes) {
      xsyserror("CreateProcessA", m_command[0]);
    }

    try {
      // Wait for it to terminate.
      DWORD waitRes = WaitForSingleObject(processInfo.hProcess, INFINITE);
      if (waitRes == WAIT_FAILED) {
        xsyserror("WaitForSingleObject", m_command[0]);
      }
      xassert(waitRes == WAIT_OBJECT_0);
      m_terminated = true;

      // Get its exit code.
      //
      // There is a special exit code, STILL_ACTIVE (259), that the
      // kernel uses to mean the process has not terminated.  But
      // nothing precludes a process from exiting with that code, and I
      // already know the process has terminated due to
      // 'WaitForSingleObject', so allow 259 to propagate if it occurs.
      DWORD exitCode;
      if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
        xsyserror("GetExitCodeProcess", m_command[0]);
      }

      // Compare the exit code to the magic value where Windows
      // exception codes begin.  There does not appear to be a symbolic
      // name for this constant.  There is STATUS_SEVERITY_ERROR=3, but
      // that would have to be left shifted by 30 bits.  An example code
      // is STATUS_ACCESS_VIOLATION, which is 0xC0000005.
      if (exitCode < 0xC0000000) {
        // Consider it a normal exit.
        m_exitedNormally = true;
      }
      else {
        // Consider it a signal.
        m_exitedNormally = false;
      }

      // Either way, stuff it into the designated slot.
      STATIC_ASSERT(sizeof(m_exitCodeOrSignal) >= sizeof(exitCode));
      m_exitCodeOrSignal = exitCode;

      // Close handles.
      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);
    }
    catch (...) {
      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);
      throw;
    }
  }

  else if (PLATFORM_IS_POSIX) {
    pid_t pid = fork();
    if (pid == 0) {
      // In child.

      // Build the argv array.
      std::vector<char const *> argv;
      for (auto it = m_command.begin(); it != m_command.end(); ++it) {
        argv.push_back((*it).c_str());
      }
      argv.push_back(NULL);

      // Exec the program.
      //
      // I'm confused about the type of the second argument here.  Why
      // is it not fully 'const'?
      execvp(argv[0], const_cast<char * const *>(argv.data()));

      // If 'execvp' returns, there was an error.  Ideally the inability
      // to 'exec' would be communicated to the parent directly, but this
      // will do for now.
      cerr << "Error executing \"" << m_command[0] << "\": "
           << strerror(errno) << endl;
      exit(4);
    }

    else {
      // In parent.

      // Wait for child to terminate.
      int status;
      pid_t waitRes = waitpid(pid, &status, 0 /*options*/);
      if (waitRes < 0) {
        // I'm not sure when or if this can happen.  The POSIX docs
        // mention the parent receiving a signal and 'waitpid' terminating
        // with errno==EINTR, but some library functions restart
        // automatically.  For now I'll just bail and see if this ever
        // happens.
        xsyserror("waitpid", m_command[0]);
      }
      xassert(waitRes == pid);

      m_terminated = true;

      // Interpret the resulting status.
      if (WIFEXITED(status)) {
        m_exitedNormally = true;
        m_exitCodeOrSignal = WEXITSTATUS(status);
      }
      else if (WIFSIGNALED(status)) {
        m_exitedNormally = false;
        m_exitCodeOrSignal = WTERMSIG(status);
      }
      else {
        xfailure("child died mysteriously");
      }
    }
  }

  else {
    xfailure("run-process.cc: unknown platform");
  }
}


bool RunProcess::exitedNormally() const
{
  xassert(m_terminated);
  return m_exitedNormally;
}


unsigned RunProcess::getExitCode() const
{
  xassert(m_terminated);
  xassert(exitedNormally());
  return m_exitCodeOrSignal;
}


unsigned RunProcess::getSignal() const
{
  xassert(m_terminated);
  xassert(!exitedNormally());
  return m_exitCodeOrSignal;
}


bool RunProcess::interrupted() const
{
  xassert(m_terminated);
  if (exitedNormally()) {
    return false;
  }

  if (PLATFORM_IS_WINDOWS) {
    return getSignal() == STATUS_CONTROL_C_EXIT;
  }
  else if (PLATFORM_IS_POSIX) {
    return getSignal() == SIGINT;
  }
  else {
    xfailure("run-process.cc: unknown platform");
  }
}


bool RunProcess::aborted() const
{
  xassert(m_terminated);
  if (exitedNormally()) {
    return false;
  }

  if (PLATFORM_IS_POSIX) {
    return getSignal() == SIGABRT;
  }
  else {
    // At least when using Cygwin, 'abort()' looks the same to the
    // caller as 'exit(3)', so if the child called 'abort' then we
    // already called it a "normal" exit.  If we get here, the child
    // died in an unusual way, but it was not due to 'abort'.
    return false;
  }
}


string RunProcess::exitDescription() const
{
  if (exitedNormally()) {
    return stringb("Exit " << getExitCode());
  }
  else if (interrupted()) {
    return string("Interrupted");
  }
  else if (aborted()) {
    return string("Aborted");
  }
  else {
    unsigned sig = getSignal();
    if (sig >= 0x10000) {
      // As a heuristic, if the value is large (such as Windows
      // exception codes), assume it's most sensible to read it as
      // hexadecimal.
      return stringb("Signal " << SBHex(getSignal()));
    }
    else {
      return stringb("Signal " << getSignal());
    }
  }
}


/*static*/ void RunProcess::check_run(std::vector<string> const &command)
{
  RunProcess rproc;
  rproc.setCommand(command);
  rproc.runAndWait();
  if (!rproc.exitedWith0()) {
    xfatal("Command \"" << accumulateWith(command, string(" ")) <<
           "\" failed: " << rproc.exitDescription());
  }
}


// The quoting rules are explained here:
//
//   https://docs.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?redirectedfrom=MSDN&view=msvc-160
//
// They make little sense, and are not completely general (specifically,
// a command name containing a double-quote cannot be expressed), but
// are evidently what we're forced to use.
//
/*static*/ void RunProcess::buildWindowsCommandLine(
  std::vector<char> &commandLine, std::vector<string> const &command)
{
  xassert(!command.empty());

  // Add the program name.
  auto it = command.begin();
  if ((*it).contains('"')) {
    // The rules for escaping double-quotes are not active when
    // decoding argv[0], so there is no way to include them.
    xformatsb(
      "On Windows, it is not possible to invoke a program whose "
      "name contains a double-quote character: \"" << command[0] <<
      "\".");
  }
  commandLine.push_back('"');
  for (char const *p = (*it).c_str(); *p; p++) {
    commandLine.push_back(*p);
  }
  commandLine.push_back('"');

  // Add arguments.
  ++it;
  for (; it != command.end(); ++it) {
    char const *p = (*it).c_str();

    commandLine.push_back(' ');
    commandLine.push_back('"');

    while (*p) {
      if (*p == '"') {
        // Escape a quote with a single backslash.  (It is also possible
        // to escape them by doubling, but only when not preceded by
        // backslashes, so the backslash form is more general.)
        commandLine.push_back('\\');
        commandLine.push_back('"');
        p++;
      }

      else if (*p == '\\') {
        // Scan forward to see if this sequence of backslashes is
        // terminated by a double-quote or the NUL.
        char const *q = p;
        while (*q == '\\') {
          q++;
        }
        if (*q == '"' || *q == '\0') {
          // Terminated by double-quote or NUL: we double every
          // backslash.  If the following character is a quote, it will
          // be escaped with another backslash so the number of
          // consecutive backslashes is odd.  If the following character
          // is NUL, the loop will terminate and we will add the final
          // quote, so the number of backslashes is even.
          while (*p == '\\') {
            commandLine.push_back('\\');
            commandLine.push_back('\\');
            p++;
          }

          // We will handle the double-quote or NUL on the next
          // iteration of the loop over characters.
        }
        else {
          // The backslash(es) are not terminated by quote or NUL, so
          // we have to *not* escape them.  This is the really weird
          // part.
          while (*p == '\\') {
            commandLine.push_back('\\');
            p++;
          }
        }
      }

      else {
        // Not a special character, just insert it normally.
        commandLine.push_back(*p);
        p++;
      }
    }

    commandLine.push_back('"');
  }

  commandLine.push_back(0);
}


// EOF
