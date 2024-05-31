// gdvalue-reader-exception.cc
// Code for gdvalue-reader-exception.h.

// This file is in the public domain.

#include "gdvalue-reader-exception.h"            // this module

#include "sm-macros.h"                           // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


GDValueReaderException::GDValueReaderException(
  FileLineCol const &location,
  std::string const &syntaxError) noexcept
  : XBase(),
    m_location(location),
    m_syntaxError(syntaxError)
{
  std::ostringstream oss;
  if (location.m_fileName) {
    oss << *location.m_fileName << ":";
  }
  oss << location.m_lc.m_line << ":" << location.m_lc.m_column << ": "
      << "GDV syntax error";
  prependContext(oss.str());
}


GDValueReaderException::~GDValueReaderException()
{}


void GDValueReaderException::prependGDVNContext(std::string const &context)
{
  m_syntaxError = stringb(context << ": " << m_syntaxError);
}


std::string GDValueReaderException::getConflict() const
{
  return m_syntaxError;
}


CLOSE_NAMESPACE(gdv)


// EOF
