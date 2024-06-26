// mysig.cc            see license.txt for copyright and terms of use
// code for mysig.h

#include "mysig.h"      // this module

// libc
#include <string.h>     // strsignal
#include <stdlib.h>     // exit
#include <stdio.h>      // printf

// POSIX
#include <unistd.h>     // sleep

// needed on Solaris; is __sun__ a good way to detect that?
#ifdef __sun__
  #include <siginfo.h>
#endif


// Everything here is for platforms other than Windows.
#if !(defined(__WIN32__))


int mysigModuleWorks()
{
  return 1;
}


void setHandler(int signum, SignalHandler handler)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));       // clear any "extra" fields
  sa.sa_handler = handler;          // handler function
  sigemptyset(&sa.sa_mask);         // don't block other signals
  sa.sa_flags = SA_RESTART;         // automatically restart syscalls

  // install the handler
  if (0 > sigaction(signum, &sa, NULL)) {
    perror("sigaction");
    exit(2);
  }
}


// simple handler that just prints and re-raises
void printHandler(int signum)
{
  fprintf(stderr, "printHandler: I caught signal %d\n", signum);
  psignal(signum, "psignal message");

  //fprintf(stderr, "I'm just going to wait a while...\n");
  //sleep(60);

  // block the signal -- doesn't work for internally-generated
  // signals (i.e. raise)
  //sigset_t mask;
  //sigemptyset(&mask);
  //sigaddset(&mask, SIGINT);

  // reset the signal handler to its default handler
  setHandler(signum, SIG_DFL);

  // re-raise, which doesn't come back to this handler because
  // the signal is blocked while we're in here -- wrong; it
  // is blocked from external signals, but not from signals
  // generated internally...
  fprintf(stderr, "re-raising...\n");
  raise(signum);
}


jmp_buf sane_state;

// handler to do a longjmp
void jmpHandler(int signum)
{
  //fprintf(stderr, "jmpHandler: I caught signal %d\n", signum);
  //psignal(signum, "jmpHandler: caught signal");

  // reset the signal handler to its default handler
  setHandler(signum, SIG_DFL);

  // do it
  //fprintf(stderr, "calling longjmp...\n");
  longjmp(sane_state, 1);
}


void printAddrHandler(int signum, siginfo_t *info, void *)
{
  fprintf(stderr, "faulting address: %p\n", info->si_addr);

  // reset handler and re-raise
  setHandler(signum, SIG_DFL);
  raise(signum);
}


// unfortunately, linux 2.4.18 seems to have some bugs w.r.t.
// sigalstack... anytime MYSZ is as small as 4096, the test program
// hangs repeatedly segfaulting once I get the first.. (but note that
// MINSIGSTKSZ is 2048, so I should be well beyond the acknowledged
// minimum); and with 8192 it works only some of the time, depending on
// how things get laid out.  so I'm going to disable the alt stack
// altogether, and rely on noticing that no "faulting address" is
// printed if I get a stack overflow...

//#define MYSZ 4096
//char mysigstack[MYSZ];

void printSegfaultAddrs()
{
  // allocate the alternate signal stack; I do this because I want
  // the handler to work even for SEGVs caused by stack-overflow
  //if (!mysigstack) {
  //  mysigstack = (char*)malloc(MINSIGSTKSZ);    // "minimum signal stack size"
  //}

  #if 0
  // tell the library about it
  struct sigaltstack sas;
  sas.ss_sp = mysigstack;
  sas.ss_size = MYSZ;
  sas.ss_flags = SS_ONSTACK;

  if (0 > sigaltstack(&sas, NULL)) {
    perror("sigaltstack");
    exit(2);
  }
  #endif // 0


  // NOTE: I have no idea how portable this stuff is, especially the
  // 'sigaltstack' call.  It's only here as a debugging aid.  Feel
  // free to #ifdef-out the entire section if necessary, but tell me
  // (smcpeak@acm.org) about it so I can add detection logic.


  // construct the handler information struct
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = printAddrHandler;
  sigemptyset(&sa.sa_mask);         // don't block other signals
  sa.sa_flags = SA_SIGINFO; // | SA_STACK;

  // install the handler
  if (0 > sigaction(SIGSEGV, &sa, NULL)) {
    perror("sigaction");
    exit(2);
  }
}


#else   // Windows -- just stubs so it compiles

int mysigModuleWorks()
{
  return 0;
}

void setHandler(int, SignalHandler) {}
void printHandler(int) {}
jmp_buf sane_state;
void jmpHandler(int) {}
void printSegfaultAddrs() {}


#endif  // Windows


// EOF
