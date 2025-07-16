// mypopen.c            see license.txt for copyright and terms of use
// code for myopen.h
// this module's implementation is in C, and not dependent on anything
// else in smbase, so it can be extracted and used independently

#include "mypopen.h"         // this module

#include <stdlib.h>          // exit, perror
#include <stdio.h>           // printf
#include <string.h>          // strlen

// POSIX
#include <unistd.h>          // pipe, read, etc.
#ifndef __WIN32__
  #include <sys/types.h>     // pid_t
  #include <sys/wait.h>      // wait
#endif

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#ifndef max
  #define max(a,b) ((a)>(b)?(a):(b))
#endif


// The entire module only works on non-Windows.
#ifndef __WIN32__


int mypopenModuleWorks()
{
  return 1;
}


int mypopenWait(int *status)
{
  return wait(status);
}


// -------------------- helpers ----------------------
static void die(char const *fn)
{
  perror(fn);
  exit(2);
}


void makePipe(int *readEnd, int *writeEnd)
{
  int pipes[2];
  if (pipe(pipes) < 0) {
    die("pipe");
  }

  *readEnd = pipes[0];
  *writeEnd = pipes[1];
}


// ------------------- execvp -------------------
struct ExecvArgs {
  char const *file;
  char const * const *argv;
};

static void call_execvp(void *_args)
{
  char *msg;
  struct ExecvArgs *args = (struct ExecvArgs*)_args;

  // I think execvp is declared incorrectly; I cast to circumvent
  execvp(args->file, (char * const *)(args->argv));

  // execvp only returns if there was an error; if this happens,
  // the error message will be printed to the stderr pipe if
  // it was redirected
  msg = (char*)malloc(strlen(args->file) + 6 + 2 + 1);
  sprintf(msg, "execvp: %s", args->file);
  die(msg);
}

int popen_execvp(int *parentWritesChild, int *parentReadsChild,
                 int *childStderr,
                 char const *file, char const * const *argv)
{
  struct ExecvArgs args;
  args.file = file;
  args.argv = argv;

  return popen_pipes(parentWritesChild, parentReadsChild, childStderr,
                     call_execvp, &args);
}


// ------------------ primary function ------------------
int popen_pipes(int *parentWritesChild, int *parentReadsChild,
                int *readChildStderr,
                execFunction func, void *extraArgs)
{
  int childReadsParent = -1;
  int childWritesParent = -1;
  int childWritesStderr = -1;
  int childPid = -1;
  int stderrToStdout = 0;

  // create pipes
  makePipe(&childReadsParent, parentWritesChild);
  makePipe(parentReadsChild, &childWritesParent);
  if (parentReadsChild == readChildStderr) {
    // caller wants child stdout and stderr going to same place
    stderrToStdout = 1;
    *readChildStderr = *parentReadsChild;
    readChildStderr = NULL;     // most of the code behaves as if stderr isn't being changed
  }
  else if (readChildStderr) {
    makePipe(readChildStderr, &childWritesStderr);
  }

  // fork a child
  childPid = fork();
  if (childPid < 0) {
    die("fork");
  }

  if (childPid != 0) {      // parent
    // close the pipe ends I'm not going to use
    if (close(childReadsParent) < -1   ||
        close(childWritesParent) < -1  ||
        (readChildStderr && close(childWritesStderr) < -1)) {
      die("close");
    }

    return childPid;
  }

  else {                    // child
    // rearrange file descriptors so stdin and stdout of the
    // program we're about to exec will talk to parent

    // sleep so debugger can attach
    //sleep(10);

    // close the pipe ends I'm not going to use
    if (close(*parentReadsChild) < -1   ||
        close(*parentWritesChild) < -1  ||
        (readChildStderr && close(*readChildStderr) < -1)) {
      die("close");
    }

    // first, close parent's stdin/stdout
    if (close(STDIN) < -1   ||
        close(STDOUT) < -1  ||
        (readChildStderr && close(STDERR) < -1)) {
      die("close");
    }

    // now, duplicate the pipe fds to stdin/stdout
    if (dup2(childReadsParent, STDIN) < -1                         ||
        dup2(childWritesParent, STDOUT)	< -1                       ||
        (readChildStderr && dup2(childWritesStderr, STDERR) < -1)  ||
        (stderrToStdout && dup2(childWritesParent, STDERR) < -1)   ) {
      die("dup2");
    }

    // finally, close the original pipe fds
    if (close(childReadsParent) < -1   ||
        close(childWritesParent) < -1  ||
        (readChildStderr && close(childWritesStderr) < -1)) {
      die("close");
    }

    // ok, fds are in order.  let's exec the child program
    func(extraArgs);

    // not reached; silence warning
    return -1;
  }
}


#else // __WIN32__

int mypopenModuleWorks()
{
  return 0;
}


static int unsupported()
{
  return -1;
}


int mypopenWait(int *status)
{
  return unsupported();
}


void makePipe(int *readEnd, int *writeEnd)
{}


int popen_pipes(int *parentWritesChild, int *parentReadsChild,
                int *childStderr,
                execFunction func, void *extraArgs)
{
  return unsupported();
}


int popen_execvp(int *parentWritesChild, int *parentReadsChild,
                 int *childStderr,
                 char const *file, char const * const *argv)
{
  return unsupported();
}


#endif


// EOF
