// sm-rc-obj.cc
// Code for sm-rc-obj.h.

#include "sm-rc-obj.h"                 // this module

#include "dev-warning.h"               // DEV_WARNING
#include "objcount.h"                  // CHECK_OBJECT_COUNT


CHECK_OBJECT_COUNT(RefCountObject);


int RefCountObject::s_objectCount = 0;


RefCountObject::~RefCountObject()
{
  --s_objectCount;
}


void RefCountObject::decRefCount()
{
  if (m_referenceCount <= 0) {
    // This does not use xfailure because decrementing a reference
    // count is often done within a destructor, and we do not want
    // to throw in that situation.
    DEV_WARNING("Attempting to decrement reference count that is "
                "already zero or negative.");
    return;
  }

  --m_referenceCount;
  if (m_referenceCount == 0) {
    delete this;
  }
}


// EOF
