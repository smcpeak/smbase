// refct-serf.h
// Reference-counted serf pointer.

// The goal of this module is to detect use-after-free errors at run
// time and prevent them from progressing to full-blown memory
// corruption.
//
// It does *not* do any automatic memory management; this is purely a
// verification mechanism.  In a correct program, it should be the case
// that you can delete all uses of this module without affecting the
// program's behavior (aside from using slightly less time and space).
//
// To use this, a class must inherit SerfRefCount.  This gives it a
// reference count field that tracks the number of RCSerf pointers
// pointing at it.  Any attempt to destroy the SerfRefCount object while
// its refct is non-zero aborts the program rather than allowing any
// pointer to dangle.
//
// Then, in place of using an ordinary T* pointer, use RCSerf<T>.  It
// has all the operations pointers do but automatically maintains the
// reference count.
//
// The term "serf" is meant as opposed to "owner".  An owner pointer is
// one that has the obligation to deallocate its referent when it goes
// away.  In contrast, a serf pointer does not.  Serf pointers also do
// not propagate constness onto their referent.
//
// There is no reference-counting "owner" counterpart to RCSerf.  Just
// use ordinary Owner (see owner.h) and it will interoperate correctly
// so long as all serfs are nullified before the Owner deallocates.  The
// same applies to any other explicit memory management scheme,
// including stack allocation, other smart pointer classes, or direct
// manual heap allocation.  When used with a conservative garbage
// collector, holding an RCSerf will prevent an object from being
// deallocated, just like an ordinary pointer would, since internally
// that is all it is.
//
// The name originates from the term in feudal peasant societies for a
// person who works the land but does not own it.  One might object to
// the hint of social injustice in the name, but of course "slave" is
// also used somewhat routinely in technical contexts, and anyway it is
// merely intended as a convenient, short, memorable name, not an
// endorsement of any social structure.

#ifndef REFCT_SERF_H
#define REFCT_SERF_H

#include "sm-noexcept.h"               // NOEXCEPT

#include <stddef.h>                    // NULL


// Forward in this file.
class RCSerfPrivateHelpers;


// Base class of objects to which RCSerf can point.
class SerfRefCount {
  friend class RCSerfPrivateHelpers;

public:      // class data
  // Constructions minus destructions.  This is used to check for
  // memory leaks when the program terminates.
  static int s_objectCount;

  // If non-NULL, this function is called prior to abort() when a fatal
  // reference count problem is detected.  After the function returns,
  // the problem condition is re-checked, and if it has been solved,
  // then we do not call abort().  It is meant for use during testing.
  static void (*s_preAbortFunction)();

private:     // instance data
  // Count of existing RCSerf pointers to this object.
  //
  // This is mutable because the reference count is not logically part
  // of the object's data, rather it is part of bug-catching
  // infrastructure.
  mutable int m_serfRefCount;

private:     // funcs
  // Call 's_preProgramFunction' if it is set.
  static void callPreAbortFunction();

public:      // funcs
  // Initialize with zero reference count.
  SerfRefCount();

  // For use in copying a containing class.  Does *not* copy the
  // reference count.  The containing class does not have to call
  // this, but it exists in case it wants to do so for uniformity.
  // This also ensures the class works with std::swap.
  SerfRefCount(SerfRefCount const &);

  // Aborts the program if the reference count is not zero, since the
  // alternative is to risk memory corruption.
  ~SerfRefCount();

  // Same rationale as for the copy constructor.
  SerfRefCount& operator= (SerfRefCount const &) { return *this; }

  // Possibly useful for testing or debugging.  Correct programs should
  // *not* change their behavior based on this value.
  int getRefCount() const { return m_serfRefCount; }

  // The reference count is not considered part of any object's
  // identity.  Again, base classes do not have to call these, they just
  // exist in case they want to for uniformity.
  bool operator== (SerfRefCount const &) const { return true; }
  bool operator!= (SerfRefCount const &) const { return false; }
};


// Helper functions for RCSerf.  They are not templatized so they can be
// compiled once, separately.  They are packaged into a class in order
// to reduce namespace pollution and simplify 'friend' nomination.
class RCSerfPrivateHelpers {
protected:   // funcs
  // If 'ptr' is not NULL, increment its reference count.
  static void incRefct(SerfRefCount const *p);

  // If 'ptr' is not NULL, decrement its reference count.  If it becomes
  // negative, abort.
  static void decRefct(SerfRefCount const *p);
};


// Reference-counted, nullable serf pointer to T.  Aside from refct
// behavior, acts like T*.  You can use RCSerf<Foo const> to get
// something that acts like Foo const *.
//
// T must inherit SerfRefCount.
//
// If T inherits SerfRefCount more than once (via multiple inheritance),
// the inheritance of SerfRefCount must be virtual.  Otherwise, the
// conversion from T* to SerfRefCount* will fail due to am ambiguous
// base class.
template <class T>
class RCSerf : private RCSerfPrivateHelpers {
private:     // data
  // Pointer to the object whose reference count we are tracking.  This
  // object acts as a wrapper for this pointer.  Can be NULL.
  T *m_ptr;

public:      // funcs
  // Initialize as NULL.
  RCSerf() : m_ptr(NULL) {}

  // Store 'ptr', and increment its refct if not NULL.  It is
  // intentional that this is not 'explicit'.
  RCSerf(T *p)
    : m_ptr(p)
  {
    RCSerfPrivateHelpers::incRefct(m_ptr);
  }

  // Copy the pointer in 'obj', incrementing refct if not NULL.
  RCSerf(RCSerf const &obj)
    : m_ptr(obj.m_ptr)
  {
    RCSerfPrivateHelpers::incRefct(m_ptr);
  }

  // Decrement refct if not NULL.  If the refct goes negative, abort
  // the program, since it means the counts are wrong and thus we are
  // at risk of memory corruption.
  ~RCSerf()
  {
    RCSerfPrivateHelpers::decRefct(m_ptr);
  }

  // Copy pointer value, adjusting refcts as appropriate.
  RCSerf& operator= (RCSerf const &obj)
  {
    this->operator=(obj.m_ptr);
    return *this;
  }

  // Update pointer value, adjusting refcts as appropriate.
  RCSerf& operator= (T *p)
  {
    if (m_ptr != p) {
      RCSerfPrivateHelpers::decRefct(m_ptr);
      m_ptr = p;
      RCSerfPrivateHelpers::incRefct(m_ptr);
    }
    return *this;
  }

  // Exchange pointers with 'other'.  No refcts change.
  void swapWith(RCSerf &other) NOEXCEPT
  {
    if (this != &other) {
      // I do not want to impose the cost of #including sm-swap.h on all
      // clients, so I will do the swap myself here.
      T *tmp = other.m_ptr;
      other.m_ptr = this->m_ptr;
      this->m_ptr = tmp;
    }
  }

  // Get the pointer as an ordinary C++ pointer.
  T *ptr() const
  {
    return m_ptr;
  }

  // Implicit conversion operator to act like T*.
  operator T* () const
  {
    return this->ptr();
  }

  T& operator* () const
  {
    return *(this->ptr());
  }

  T* operator-> () const
  {
    return this->ptr();
  }

  // Set m_ptr to NULL, decrementing refct if not already NULL.
  // Return the value m_ptr had before the call, which may be NULL.
  //
  // This is meant for cases where we want to pass the pointer to a
  // function that will deallocate the object.  It is not a transfer of
  // ownership, since the serf pointer does not own the object, but the
  // serf pointer is being used as a *name* for something that another
  // object owns, and is being used as part of an operation that will
  // result in that thing being deallocated.
  T *release()
  {
    T *ret = m_ptr;
    RCSerfPrivateHelpers::decRefct(m_ptr);
    m_ptr = NULL;
    return ret;
  }
};


template <class T>
void swap(RCSerf<T> &a, RCSerf<T> &b) NOEXCEPT
{
  a.swapWith(b);
}


#endif // REFCT_SERF_H
