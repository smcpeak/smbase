// sm-rc-ptr.h
// RCPtr template class.

#ifndef SMBASE_SM_RC_PTR
#define SMBASE_SM_RC_PTR

#include "sm-rc-obj.h"                 // RefCountObject

#include <stddef.h>                    // NULL


// Pointer to RefCountObject with automatic reference counting.
//
// It is my intention that the semantics broadly aligns with
// std::unique_ptr and std::shared_ptr, with the difference being
// shared_ptr stores the reference count in a separately allocated
// object, whereas this class uses a reference count embedded within the
// pointed-to object.
//
class RCPtrBase {
private:     // data
  // The underlying pointer.
  RefCountObject *m_pointer;

private:     // methods
  void inc()
  {
    if (m_pointer) {
      m_pointer->incRefCount();
    }
  }

  void dec()
  {
    if (m_pointer) {
      m_pointer->decRefCount();
      m_pointer = NULL;
    }
  }

public:      // methods
  RCPtrBase()
    : m_pointer(NULL)
  {}

  explicit RCPtrBase(RefCountObject *p)
    : m_pointer(p)
  {
    inc();
  }

  ~RCPtrBase()
  {
    dec();
  }

  RCPtrBase(RCPtrBase const &obj)
    : m_pointer(obj.m_pointer)
  {
    inc();
  }

  // Move the pointer from 'src', leaving it with NULL and not changing
  // the reference count of the object.
  RCPtrBase(RCPtrBase &&src) noexcept
    : m_pointer(src.m_pointer)
  {
    src.m_pointer = NULL;
  }

  RCPtrBase& operator= (RCPtrBase const &src)
  {
    // The check inside 'reset' takes care of 'this == &src'.
    reset(src.m_pointer);
    return *this;
  }

  RCPtrBase& operator= (RefCountObject *p)
  {
    reset(p);
    return *this;
  }

  RefCountObject *get() const
  {
    return m_pointer;
  }

  // Set or reset the pointer.
  void reset(RefCountObject *p = NULL)
  {
    if (p != m_pointer) {
      dec();
      m_pointer = p;
      inc();
    }
  }

  // Return the current object, releasing control of it and not changing
  // its reference count.
  RefCountObject *release()
  {
    RefCountObject *ret = m_pointer;
    m_pointer = NULL;
    return ret;
  }
};


// Pointer to T with automatic reference counting.
//
// Class T should be derived from 'RefCountObject'.
//
// For the most part, this class's methods just delegate to those of
// RCPtrBase, and I do not repeat the comments that appear there.
//
template <class T>
class RCPtr : public RCPtrBase {
public:      // methods
  RCPtr()
    : RCPtrBase()
  {}

  explicit RCPtr(T *p)
    : RCPtrBase(p)
  {}

  RCPtr(RCPtr const &obj)
    : RCPtrBase(obj)
  {}

  RCPtr(RCPtr &&src) noexcept
    : RCPtrBase(src)
  {}

  RCPtr& operator= (RCPtr const &src)
  {
    RCPtrBase::operator=(src);
    return *this;
  }

  RCPtr& operator= (T *p)
  {
    RCPtrBase::operator=(p);
    return *this;
  }

  T *get() const
  {
    return static_cast<T*>(RCPtrBase::get());
  }

  void reset(T *p = NULL)
  {
    RCPtrBase::reset(p);
  }

  T *release()
  {
    return static_cast<T*>(RCPtrBase::release());
  }

  // Allow RCPtr to be treated like a normal pointer.
  operator T* () const
  {
    return get();
  }

  T& operator * () const
  {
    return *(get());
  }

  T* operator -> () const
  {
    return get();
  }
};


#endif // SMBASE_SM_RC_PTR
