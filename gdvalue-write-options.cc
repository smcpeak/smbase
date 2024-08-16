// gdvalue-write-options.cc
// Code for gdvalue-write-options module.

// This file is in the public domain.

#include "gdvalue-write-options.h"     // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


int GDValueWriteOptions::s_defaultSpacesPerIndentLevel = 2;

int GDValueWriteOptions::s_defaultTargetLineWidth = 72;


int GDValueWriteOptions::lineCapacity() const
{
  return m_targetLineWidth - m_indentLevel * m_spacesPerIndentLevel;
}


CLOSE_NAMESPACE(gdv)


// EOF
