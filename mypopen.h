// mypopen.h            see license.txt for copyright and terms of use
// Open a process and yield pipes to its stdin/stdout/stderr.

#ifndef SMBASE_MYPOPEN_H
#define SMBASE_MYPOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

// Returns true if this 'mypopen' module is working.  Otherwise, we are
// running on a platform where it does not work.
int mypopenModuleWorks();

// Wrapper for 'wait' that just returns -1 for an unsupported platform.
int mypopenWait(int *status);

// create a pipe and retrieve the read and write file descriptors
void makePipe(int *readEnd, int *writeEnd);

// function to exec something, given some args to say how; it must
// *not* return
typedef void (*execFunction)(void *extraArgs);

// generic popen with generic exec function; the first two int* must
// not be NULL, but the third can be NULL (in which case stderr will
// not be redirected); alternatively, if 'childStderr' ==
// 'parentReadsChild', then the child's stderr will be directed to the
// same pipe as its stdout
int popen_pipes(int *parentWritesChild, int *parentReadsChild,
                int *childStderr,
                execFunction func, void *extraArgs);

// version that calls execvp internally
int popen_execvp(int *parentWritesChild, int *parentReadsChild,
                 int *childStderr,
                 char const *file, char const * const *argv);


#ifdef __cplusplus
}
#endif

#endif // SMBASE_MYPOPEN_H
