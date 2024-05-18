// gdvalue-write-options.cc
// Code for gdvalue-write-options module.

#include "gdvalue-write-options.h"     // this module


namespace gdv {


int GDValueWriteOptions::s_defaultSpacesPerIndentLevel = 2;

int GDValueWriteOptions::s_defaultTargetLineWidth = 72;


int GDValueWriteOptions::lineCapacity() const
{
  return m_targetLineWidth - m_indentLevel * m_spacesPerIndentLevel;
}


} // namespace gdv


// EOF
