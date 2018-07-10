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
// In place of T const *, use RCSerfC<T>.
//
// The term "serf" is meant as opposed to "owner".  An owner pointer is
// one that has the obligation to deallocate its referent when it goes
// away.  In contrast, a serf pointer does not.  Serf pointers also do
// not propagate constness onto their referent.
//
// The term originates from the term in feudal peasant societies for a
// person who works the land but does not own it.  One might object to
// the hint of social injustice in the name, but of course "slave" is
// also used somewhat routinely in technical contexts, and anyway it is
// merely intended as a convenient, short, memorable name.
//
// There is no reference-counting "owner" counterpart to RCSerf.  Just
// use ordinary Owner (see owner.h) and it will interoperate correctly
// so long as all serfs are nullified before the Owner deallocates.

#ifndef REFCT_SERF_H
#define REFCT_SERF_H

#include "sm-noexcept.h"               // NOEXCEPT

#include <stddef.h>                    // NULL


// Forward declarations in this file.
class RCSerfBaseC;


// Base class of objects to which RCSerf can point.
class SerfRefCount {
  friend class RCSerfBaseC;

public:      // class data
  // Constructions minus destructions.
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

  // Possibly useful for testing or debugging.
  int getRefCount() const { return m_serfRefCount; }

  // The reference count is not considered part of any object's
  // identity.
  bool operator== (SerfRefCount const &) const { return true; }
  bool operator!= (SerfRefCount const &) const { return false; }
};


// The base class of RCSerf[C].  Maintains the reference count inside
// SerfRefCount objects.  Otherwise, acts like a pointer to
// SerfRefCount.
//
// The trailing "C" means 'const'.  This is the version whose interface
// uses pointers to const.  RCSerfBase exposes a non-const interface.
//
// This is not meant to be used directly by clients.
class RCSerfBaseC {
private:     // data
  // Pointer to the object whose reference count we are tracking.  This
  // object acts as a wrapper for this pointer.  Can be NULL.
  SerfRefCount const *m_ptr;

private:     // funcs
  // Discarding the current m_ptr, set it to 'ptr', and increment the
  // refct if not NULL.
  void acquire(SerfRefCount const *ptr);

public:      // funcs
  // Initialize as NULL.
  RCSerfBaseC() : m_ptr(NULL) {}

  // Store 'ptr', and increment its refct if not NULL.  It is
  // intentional that this is not 'explicit'.
  RCSerfBaseC(SerfRefCount const *ptr);

  // Copy the pointer in 'obj', incrementing refct if not NULL.
  RCSerfBaseC(RCSerfBaseC const &obj);

  // Decrement refct if not NULL.  If the refct goes negative, abort
  // the program, since it means the counts are wrong and thus we are
  // at risk of memory corruption.
  ~RCSerfBaseC();

  // Copy pointer value, adjusting refcts as appropriate.
  RCSerfBaseC& operator= (RCSerfBaseC const &obj);

  // Update pointer value, adjusting refcts as appropriate.
  RCSerfBaseC& operator= (SerfRefCount const *ptr);

  // Exchange pointers with 'other'.  No refcts change.
  void swapWith(RCSerfBaseC &other) NOEXCEPT;

  // Get the pointer as an ordinary C++ pointer.
  SerfRefCount const *ptr() const { return m_ptr; }

  // Set m_ptr to NULL, decrementing refct if not already NULL.
  // Return the value m_ptr had before the call, which may be NULL.
  //
  // This is meant for cases where we want to pass the pointer to a
  // function that will deallocate the object.  It is not a transfer of
  // ownership, since the serf pointer does not own the object, but the
  // serf pointer is being used as a *name* for something that something
  // else owns, and is being used to instruct that thing to deallocate
  // the object.
  SerfRefCount const *release();
};


// Reference-counted serf pointer to const T.  T must inherit
// SerfRefCount.  Aside from refct behavior, acts like T const *.
template <class T>
class RCSerfC : private RCSerfBaseC {
public:      // funcs
  // Initialize as NULL.
  RCSerfC() : RCSerfBaseC() {}

  // Store 'ptr', and increment its refct if not NULL.
  RCSerfC(T const *ptr)
    : RCSerfBaseC(ptr)
  {}

  // Copy the pointer in 'obj', incrementing refct if not NULL.
  RCSerfC(RCSerfC const &obj)
    : RCSerfBaseC(obj)
  {}

  // Decrement refct if not NULL.  Aborts program if refct goes negative.
  ~RCSerfC()
  {
    // The work is done by ~RCSerfBaseC().
  }

  // Copy pointer value, adjusting refcts as appropriate.
  RCSerfC& operator= (RCSerfC const &obj)
  {
    RCSerfBaseC::operator=(obj);
    return *this;
  }

  // Update pointer value, adjusting refcts as appropriate.
  RCSerfC& operator= (T const *ptr)
  {
    RCSerfBaseC::operator=(ptr);
    return *this;
  }

  // Exchange pointers with 'other'.  No refcts change.
  void swapWith(RCSerfC &other) NOEXCEPT
  {
    RCSerfBaseC::swapWith(other);
  }

  // Get the pointer as an ordinary C++ pointer.
  T const *ptr() const
  {
    return static_cast<T const *>(RCSerfBaseC::ptr());
  }

  // Implicit conversion operator to act like T const *.
  operator T const * () const
  {
    return this->ptr();
  }

  T const & operator* () const
  {
    return *(this->ptr());
  }

  T const * operator-> () const
  {
    return this->ptr();
  }

  // Get the pointer, setting 'this' to NULL simultaneously.  May
  // return NULL.
  T const * release()
  {
    return static_cast<T const *>(RCSerfBaseC::release());
  }

  // Get the underlying RCSerfBase.  Among the reasons it is unsafe is
  // it would allow one to substitute anything that inherits
  // SerfRefCount for the current pointer, thus violating type safety.
  // This is exposed only for the use of unit test code.
  RCSerfBaseC& unsafe_getRCSerfBaseC()
  {
    return *this;
  }
};


template <class T>
void swap(RCSerfC<T> &a, RCSerfC<T> &b) NOEXCEPT
{
  a.swapWith(b);
}


// Reference-counted serf pointer to T.  T must inherit SerfRefCount.
// Aside from refct behavior, acts like T*.
//
// The reason I chose to define both RCSerf and RCSerfC, rather than,
// say, have the user just write RCSerf<Foo const> is that allowing the
// latter would have meant that RCSerf would speculatively need
// const_cast<static_cast<...> > everywhere (both places...) it
// currently has static_cast, and I didn't like that.  I prefer all the
// static_casts to be in RCSerfC, and all the const_casts here in
// RCSerf.
template <class T>
class RCSerf {
private:     // data
  // Implement on top of RCSerfC.
  RCSerfC<T> impl;

public:      // funcs
  // Initialize as NULL.
  RCSerf() : impl() {}

  // Store 'ptr', and increment its refct if not NULL.
  RCSerf(T *ptr)
    : impl(ptr)
  {}

  // Copy the pointer in 'obj', incrementing refct if not NULL.
  RCSerf(RCSerf const &obj)
    : impl(obj.impl)
  {}

  // Decrement refct if not NULL.  Aborts program if refct goes negative.
  ~RCSerf()
  {
    // The work is done when 'impl' is destroyed.
  }

  // Copy pointer value, adjusting refcts as appropriate.
  RCSerf& operator= (RCSerf const &obj)
  {
    impl.operator=(obj);
    return *this;
  }

  // Update pointer value, adjusting refcts as appropriate.
  RCSerf& operator= (T *ptr)
  {
    impl.operator=(ptr);
    return *this;
  }

  // Exchange pointers with 'other'.  No refcts change.
  void swapWith(RCSerf &other) NOEXCEPT
  {
    impl.swapWith(other.impl);
  }

  // Get the pointer as an ordinary C++ pointer.  This is 'const'
  // because my convention is const serf pointers do not carry the
  // constness forward to the referent.  (In contrast, owner pointers
  // do carry constness forward.)
  T *ptr() const
  {
    return const_cast<T*>(impl.ptr());
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

  // Get the pointer, setting 'this' to NULL simultaneously.  May
  // return NULL.
  T* release()
  {
    return const_cast<T*>(impl.release());
  }

  // Get the underlying RCSerfBase.  Among the reasons it is unsafe is
  // it would allow one to substitute anything that inherits
  // SerfRefCount for the current pointer, thus violating type safety.
  // This is exposed only for the use of unit test code.
  RCSerfBaseC& unsafe_getRCSerfBaseC()
  {
    return impl.unsafe_getRCSerfBaseC();
  }
};


template <class T>
void swap(RCSerf<T> &a, RCSerf<T> &b) NOEXCEPT
{
  a.swapWith(b);
}


#endif // REFCT_SERF_H
