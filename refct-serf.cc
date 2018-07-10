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


/*static*/ void SerfRefCount::callPreAbortFunction()
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


// --------------------------- RCSerfBaseC ---------------------------
void RCSerfBaseC::acquire(SerfRefCount const *ptr)
{
  m_ptr = ptr;
  if (m_ptr) {
    m_ptr->m_serfRefCount++;           // refct is mutable
  }
}


SerfRefCount const *RCSerfBaseC::release()
{
  SerfRefCount const *ret = m_ptr;
  if (m_ptr) {
    m_ptr->m_serfRefCount--;           // refct is mutable

    if (m_ptr->m_serfRefCount < 0) {
      SerfRefCount::callPreAbortFunction();
      if (m_ptr->m_serfRefCount < 0) {
        fprintf(stderr,
          "FATAL: Pointer at %p was pointing at object at %p which "
          "has negative refct %d after decrementing.  Aborting.\n",
          this,
          m_ptr,
          m_ptr->m_serfRefCount);
        fflush(stderr);
        abort();
      }
    }

    m_ptr = NULL;
  }
  return ret;
}


RCSerfBaseC::RCSerfBaseC(SerfRefCount const *ptr)
  : m_ptr(NULL)
{
  this->acquire(ptr);
}


RCSerfBaseC::RCSerfBaseC(RCSerfBaseC const &obj)
  : m_ptr(NULL)
{
  this->acquire(obj.m_ptr);
}


RCSerfBaseC::~RCSerfBaseC()
{
  this->release();
}


RCSerfBaseC& RCSerfBaseC::operator= (RCSerfBaseC const &obj)
{
  return operator=(obj.m_ptr);
}


RCSerfBaseC& RCSerfBaseC::operator= (SerfRefCount const *ptr)
{
  if (m_ptr != ptr) {
    this->release();
    this->acquire(ptr);
  }
  return *this;
}


void RCSerfBaseC::swapWith(RCSerfBaseC &other) NOEXCEPT
{
  if (this != &other) {
    swap(m_ptr, other.m_ptr);
  }
}


// EOF
