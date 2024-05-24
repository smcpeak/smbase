// refct-serf.cc
// code for refct-serf.h

#include "refct-serf.h"                // this module

// smbase
#include "breaker.h"                   // breaker
#include "objcount.h"                  // CHECK_OBJECT_COUNT
#include "sm-swap.h"                   // swap

// libc
#include <stdio.h>                     // fprintf, stderr, fflush
#include <stdlib.h>                    // abort


// ---------------------- SerfRefCount ----------------------
int SerfRefCount::s_objectCount = 0;

CHECK_OBJECT_COUNT(SerfRefCount);

void (*SerfRefCount::s_preAbortFunction)() = NULL;


STATICDEF void SerfRefCount::callPreAbortFunction()
{
  breaker();
  if (s_preAbortFunction) {
    (*s_preAbortFunction)();
  }
}


SerfRefCount::SerfRefCount()
  : m_serfRefCount(0)
{
  s_objectCount++;
}


SerfRefCount::SerfRefCount(SerfRefCount const &)
  : m_serfRefCount(0)
{
  s_objectCount++;
}


SerfRefCount::~SerfRefCount()
{
  s_objectCount--;

  if (m_serfRefCount != 0) {
    SerfRefCount::callPreAbortFunction();
    if (m_serfRefCount != 0) {
      // I choose to use 'stderr' rather than 'cerr' since I think it is
      // possibly more reliable in situations with possibly corrupted
      // memory.  (Here, the intent is to stop before memory becomes
      // corrupted, but if things are bad here, they might be silently bad
      // elsewhere too.)  If I were really paranoid I would use the
      // 'write' system call instead, but I think this is adequate.
      fprintf(stderr,
        "FATAL: Destroying object at %p with non-zero refct %d.  "
        "Aborting.\n",
        this,
        m_serfRefCount);
      fflush(stderr);
      abort();
    }
  }
}


// --------------------- RCSerfPrivateHelpers -----------------------
STATICDEF void RCSerfPrivateHelpers::incRefct(SerfRefCount const *p)
{
  if (p) {
    p->m_serfRefCount++;               // refct is mutable
  }
}


STATICDEF void RCSerfPrivateHelpers::decRefct(SerfRefCount const *p)
{
  if (p) {
    p->m_serfRefCount--;               // refct is mutable

    if (p->m_serfRefCount < 0) {
      SerfRefCount::callPreAbortFunction();
      if (p->m_serfRefCount < 0) {
        fprintf(stderr,
          "FATAL: RCSerf was pointing at object at %p which "
          "has negative refct %d after decrementing.  Aborting.\n",
          p,
          p->m_serfRefCount);
        fflush(stderr);
        abort();
      }
    }
  }
}


// EOF
