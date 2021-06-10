// sm-posix.h
// Wrapper around some POSIX headers.

// Pull in a subset of POSIX when targeting a POSIX platform.
//
// Otherwise, this declares (but does not define) an even smaller subset
// containing the functions I have used.  This allows code inside
// "if (PLATFORM_IS_POSIX) {...}" to compile and then be discarded by
// the compiler when not on POSIX.

#ifndef SMBASE_SM_POSIX_H
#define SMBASE_SM_POSIX_H

#include "sm-platform.h"               // PLATFORM_IS_POSIX

// I think this is available even on non-POSIX.
#include <sys/types.h>                 // pid_t


#if PLATFORM_IS_POSIX

#include <signal.h>                    // SIGINT
#include <sys/wait.h>                  // waitpid
#include <unistd.h>                    // fork, exec


#else // PLATFORM_IS_POSIX

pid_t fork();
int execvp(const char *file, char *const argv[]);
pid_t waitpid(pid_t pid, int *stat_loc, int options);
#define WIFEXITED(s) 0
#define WEXITSTATUS(s) 0
#define WIFSIGNALED(s) 0
#define WTERMSIG(s) 0


#endif // PLATFORM_IS_POSIX

#endif // SMBASE_SM_POSIX_H
