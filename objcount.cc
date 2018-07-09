// objcount.cc
// code for objcount.h

#include "objcount.h"                  // this module

#include "sm-iostream.h"               // cerr

#include <stdlib.h>                    // exit

// TODO: DEV_WARNING is currently defined in 'editor'.  I need to
// move it into smbase.


// This should be a variable declared in dev-warning.h, but that
// module is not in smbase yet.
bool CheckObjectCount::s_exitUponFailure = false;


CheckObjectCount::CheckObjectCount(char const *name, int &count)
  : m_className(name),
    m_objectCount(count)
{}


CheckObjectCount::~CheckObjectCount()
{
  if (m_objectCount != 0) {
    cerr << "DEV_WARNING: Class " << m_className
         << " object count is " << m_objectCount
         << " upon termination.  It should be zero." << endl;
    if (s_exitUponFailure) {
      cerr << "Exiting with code 4 due to DEV_WARNING." << endl;
      exit(4);
    }
  }
}


// EOF
