// mysig.h            see license.txt for copyright and terms of use
// Some signal-related utilities.

#ifndef SMBASE_MYSIG_H
#define SMBASE_MYSIG_H

#include <signal.h>     // signal stuff
#include <setjmp.h>     // jmp_buf

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Returns true if this 'mysig' module is working.  Otherwise, we are
// running on a platform where it does not work.
int mysigModuleWorks();


// type of a signal handler function; generally, there are
// three options for a signal handler:
//   - return, in which case the default action for the
//     signal is taken
//   - longjmp to a state where computation can resume
//   - abort(2) or exit(2)
// it's somewhat dangerous to do other system calls, but people
// do it anyway
typedef void (*SignalHandler)(int signum);


// install the given handler on the given signal
void setHandler(int signum, SignalHandler handler);


// simple handler that just prints and re-raises
void printHandler(int signum);


// to use jmpHandler, call setjmp(sane_state) before
// installing the handler
extern jmp_buf sane_state;

// handler to do a longjmp to sane_state
void jmpHandler(int signum);


// install a segfault handler that will print the address that
// caused the fault; this is very useful for debugging
void printSegfaultAddrs();


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SMBASE_MYSIG_H
