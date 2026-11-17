// fasttime.cc
// code for fasttime.h

/*

The basic idea of this module is to use a small shared-memory variable
(small enough to atomically read and write without bus locks) that is
periodically written by a timer thread and can be read by the main (or
any other) thread to sample the clock.  Reading it is very fast because,
if it is being read frequently, it will live in the CPU cache of the
main thread.  It only gets evicted when the timer thread updates it,
which is rare compared to the likely sample frequency.  (And if the
sample frequency is low, then the speed of doing so is inconsequential.)

The clock precision is relatively low, typically on the order of 10 ms,
but over large numbers of samples, the precision errors tend to cancel,
like in any sampling-based profiling system, leading to reasonably
accurate measurements for tasks that take time noticeable to humans.
(This assumes that the tasks being measured are not correlated with
thread switching.)

In this module, I specifically avoid using C or C++ thread-creation
primitives because I do not want this module to cause programs compiled
with it to be required to link with the multithreaded (MT) C/C++
libraries.  However, that means that the timer thread must avoid
invoking any C/C++ facilities that would, in the MT libraries, be
synchronized (e.g., with a mutex), since the goal is to be safe even
when that synchronization is absent.

That said, the timer thread does use `cout` to report fatal errors
during startup.  That is unsafe in general (iostreams is necessarily
synchronized in the MT libraries), but it happens only right before an
`abort()`, and only while the main thread is blocked.

*/


#include "smbase/fasttime.h"           // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <atomic>                      // std::atomic_uint


OPEN_NAMESPACE(smbase)

SMBASE_FASTTIME_UNSIGNED fastTimeMilliseconds = 0;

SMBASE_FASTTIME_UNSIGNED fastTimeThread1000Loops = 0;

CLOSE_NAMESPACE(smbase)


#if defined(__MINGW32__)

#include <stdlib.h>                    // abort
#include <process.h>                   // _beginthreadex
#include <windows.h>                   // Sleep, GetTickCount, CreateSemaphore, etc.

#include <iostream>                    // std::cout

using namespace std;


OPEN_NAMESPACE(smbase)


// Handle to a semaphore used to wait for the thread to start.
static HANDLE hSemaphore = 0;


static unsigned __stdcall fastTimeThreadFunc(void*)
{
  // Do an initial query before incrementing the semaphore.
  fastTimeMilliseconds = GetTickCount();

  // Now let the main thread continue.
  if (!ReleaseSemaphore(hSemaphore, 1 /*count*/, NULL /*prevCount*/)) {
    // I would like to avoid calls into the C++ library in this thread,
    // but this is probably better than being completely silent.
    cout << "fasttime: ReleaseSemaphore failed!" << endl;
    abort();
  }

  unsigned loops = 0;
  while (true) {
    // Keep track of how many times this loop runs in order to ensure
    // it isn't consuming excessive resources.  The shared counter
    // variable is only updated occasionally in order to limit
    // cross-CPU memory traffic.
    //
    // One data point: On Windows 11 / x86_64, the outer `while` loop
    // only runs about once every 10 ms, so the "1000 loop" counter only
    // gets incremented about once every 10 seconds.
    loops++;
    if (loops % 1000 == 0) {
      fastTimeThread1000Loops++;
    }

    // Wait a bit.
    Sleep(1 /*ms*/);

    // Update the time count.
    fastTimeMilliseconds = GetTickCount();
  }
}


bool fastTimeIsImplemented()
{
  return true;
}


void fastTimeInitialize()
{
  // Don't start more than one thread.
  static bool started = false;
  if (started) {
    return;
  }
  started = true;

  // Create the semaphore with initial value 0.
  hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

  // Start the time update thread.
  if (!_beginthreadex(NULL /*security*/,
                      0 /*stackSize*/,
                      &fastTimeThreadFunc,
                      NULL /*argList*/,
                      0 /*initFlag*/,
                      NULL /*thrdAddr*/)) {
    cout << "fasttime: _beginthreadex failed!" << endl;
    abort();
  }

  // Wait for the semaphore to be incremented by the thread.
  DWORD res = WaitForSingleObject(hSemaphore, 1000 /*ms*/);
  if (res != WAIT_OBJECT_0) {
    cout << "fasttime: WaitForSingleObject failed!" << endl;
    abort();
  }
}


CLOSE_NAMESPACE(smbase)


#else // !__MINGW32__


OPEN_NAMESPACE(smbase)


bool fastTimeIsImplemented()
{
  return false;
}


void fastTimeInitialize()
{}


CLOSE_NAMESPACE(smbase)


#endif


// EOF
