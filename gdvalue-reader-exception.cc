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
  oss << location.m_lc.m_line << ":" << location.m_lc.m_column << ": "
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


void GDValueReaderException::prependGDVNContext(std::string const &context)
{
  m_syntaxError = stringb(context << ": " << m_syntaxError);

  // Recreate the message rather than calling `xBase::prependContext`
  // so `context` ends up in the right place.
  this->msg = makeExceptionMessage(m_location, m_syntaxError);
}


} // namespace gdv


// EOF
