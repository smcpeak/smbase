// gdvalue-reader-exception.cc
// Code for gdvalue-reader-exception.h.

#include "gdvalue-reader-exception.h"            // this module


namespace gdv {


static std::string makeExceptionMessage(
  FileLineCol const &location,
  std::string const &syntaxError) noexcept
{
  std::ostringstream oss;
  if (location.m_fileName) {
    oss << *location.m_fileName << ":";
  }
  oss << location.m_line << ":" << location.m_column << ": "
      << "GDV syntax error: " << syntaxError;
  return oss.str();
}


GDValueReaderException::GDValueReaderException(
  FileLineCol const &location,
  std::string const &syntaxError) noexcept
  : xFormat(makeExceptionMessage(location, syntaxError)),
    m_location(location),
    m_syntaxError(syntaxError)
{}


GDValueReaderException::~GDValueReaderException()
{}


} // namespace gdv


// EOF
