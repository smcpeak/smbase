// run-process.cc
// Code for run-process.h

#include "run-process.h"               // this module

#include "syserr.h"                    // xsyserror
#include "xassert.h"                   // xassert

#include "sm-iostream.h"               // cout, cerr

#include <errno.h>                     // errno
#include <string.h>                    // strerror

#ifdef __WIN32__
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>                 // Windows API
#else
  #include <signal.h>                  // SIGINT
  #include <sys/wait.h>                // waitpid
  #include <unistd.h>                  // fork, exec
#endif


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

#ifdef __WIN32__
  // TODO
  m_terminated = true;

#else
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
#endif
}


bool RunProcess::exitedNormally() const
{
  xassert(m_terminated);
  return m_exitedNormally;
}


int RunProcess::getExitCode() const
{
  xassert(m_terminated);
  xassert(exitedNormally());
  return m_exitCodeOrSignal;
}


int RunProcess::getSignal() const
{
  xassert(m_terminated);
  xassert(!exitedNormally());
  return m_exitCodeOrSignal;
}


bool RunProcess::interrupted() const
{
  xassert(m_terminated);

#ifdef __WIN32__
  // TODO
#else
  return !exitedNormally() && getSignal() == SIGINT;
#endif
}

string RunProcess::exitDescription() const
{
  if (exitedNormally()) {
    return stringb("Exit " << getExitCode());
  }
  else if (interrupted()) {
    return string("Interrupted");
  }
  else {
    return stringb("Signal " << getSignal());
  }
}


// EOF
