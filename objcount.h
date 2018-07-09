// objcount.h
// Things related to "object count" idiom.

// The "object count" idiom is where I define:
//
//   static int s_objectCount;
//
// in a class, increment it in every constructor, and decrement it in
// the destructor.  Then I expect it to be zero at the end, otherwise
// I have a leak or double-free.

#ifndef OBJCOUNT_H
#define OBJCOUNT_H

#include "macros.h"                    // NO_OBJECT_COPIES


// This class is meant to be created as a global variable to check the
// count of some specific class upon termination.
class CheckObjectCount {
  NO_OBJECT_COPIES(CheckObjectCount);

public:      // instance data
  // Name of the class whose count I am watching.
  char const *m_className;

  // Reference to its 'objectCount' static field.
  int &m_objectCount;

public:      // funcs
  CheckObjectCount(char const *name, int &count);

  // This is what does the work.  It will use DEV_WARNING if the count
  // is not zero.
  ~CheckObjectCount();
};


// Instantiate a check object at file scope with program lifetime.
#define CHECK_OBJECT_COUNT(className)                           \
  static CheckObjectCount checkCount_##className(#className,    \
    className::s_objectCount) /* user ; */


#endif // OBJCOUNT_H
