// objcount.cc
// code for objcount.h

#include "objcount.h"                  // this module

// smbase
#include "dev-warning.h"               // DEV_WARNING
#include "sm-iostream.h"               // cerr

// libc
#include <stdlib.h>                    // exit


bool CheckObjectCount::s_suppressLeakReports = false;


CheckObjectCount::CheckObjectCount(char const *name, int &count)
  : m_className(name),
    m_objectCount(count)
{}


CheckObjectCount::~CheckObjectCount()
{
  if (m_objectCount != 0 && !s_suppressLeakReports) {
    DEV_WARNING("Class " << m_className <<
                " object count is " << m_objectCount <<
                " upon termination.  It should be zero.");
  }
}


// EOF
