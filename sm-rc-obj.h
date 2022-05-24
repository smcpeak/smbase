// sm-rc-obj.h
// RefCountObject class.

#ifndef SMBASE_SM_RC_OBJ_H
#define SMBASE_SM_RC_OBJ_H

// Base class that contains a reference count.  The object is
// deallocated when its reference count hits zero.
//
// It is entirely up to clients to increment and decrement the count
// appropriately.  A newly constructed object starts with a reference
// count of zero, so a client should typically increment it immediately.
//
class RefCountObject {
public:      // class data
  // Total number of allocated objects of this (or derived) type.  This
  // is to help catch reference counting errors; at the end of the
  // program, the count should be zero, so if it's not, then there is a
  // bug somewhere.
  static int s_objectCount;

private:     // instance data
  // Number of pointers to this object.
  int m_referenceCount;

public:      // methods
  // Upon initial creation, the count is zero.
  RefCountObject()
    : m_referenceCount(0)
  {
    ++s_objectCount;
  }

  // The destructor must of course be virtual because we will be
  // deleting 'this' without knowing the full dynamic type.
  virtual ~RefCountObject();

  // A newly-created copy also has an initial count of zero.
  RefCountObject(RefCountObject const &obj)
    : m_referenceCount(0)
  {
    ++s_objectCount;
  }

  // Copy assignment does *not* alter the reference count.
  RefCountObject& operator= (RefCountObject const &obj)
  {
    return *this;
  }

  int getRefCount() const
  {
    return m_referenceCount;
  }

  void incRefCount()
  {
    ++m_referenceCount;
  }

  // Decrement the reference count, and if it thereby becomes zero,
  // delete 'this'.
  void decRefCount();
};


// If the argument is not NULL, increment its reference count and return
// it.  This is convenient for wrapping object creation expressions.
template <class T>
T *incRefCount(T *obj)
{
  if (obj) {
    obj->incRefCount();
  }
  return obj;
}


// Decrement the given object's reference count if it is not zero.
inline void decRefCount(RefCountObject *obj)
{
  if (obj) {
    obj->decRefCount();
  }
}


#endif // SMBASE_SM_RC_OBJ_H
