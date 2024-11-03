// reader.cc
// Code for `reader` module.

#include "reader.h"                    // this module

#include "codepoint.h"                 // isASCIIPrintable
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "stringb.h"                   // stringbc, stringb

#include <iomanip>                     // std::hex
#include <iostream>                    // std::istream
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// -------------------------- ReaderException --------------------------
ReaderException::ReaderException(
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
  oss << location.m_lc.m_line << ":" << location.m_lc.m_column;
  prependContext(oss.str());
}


ReaderException::~ReaderException()
{}


void ReaderException::prependErrorContext(std::string const &context)
{
  m_syntaxError = stringb(context << ": " << m_syntaxError);
}


std::string ReaderException::getConflict() const
{
  return m_syntaxError;
}


// ------------------------------ Reader -------------------------------
Reader::Reader(std::istream &is,
               std::optional<std::string> fileName)
  : m_is(is),
    m_location(std::move(fileName))
{}


Reader::~Reader()
{}


void Reader::err(string const &syntaxError) const
{
  locErr(m_location, syntaxError);
}


void Reader::locErr(FileLineCol const &loc,
                    string const &syntaxError) const
{
  // Generally, we read a character, advancing the location in the
  // process, then check for an error.  Consequently, when we report an
  // error the location is one past the place the erroneous character
  // was.  So, back the location spot up.
  FileLineCol prev(loc);
  prev.decrementColumn();

  THROW(ReaderException(prev, syntaxError));
}


void Reader::unexpectedCharErr(int c, char const *lookingFor) const
{
  inCtxUnexpectedCharErr(c, stringbc("while " << lookingFor));
}


void Reader::inCtxUnexpectedCharErr(int c, char const *context) const
{
  if (c == eofCode()) {
    err(stringb("Unexpected end of file " <<
                context << "."));
  }
  // TODO: This should use `singleQuoteChar`.
  else if (isASCIIPrintable(c)) {
    err(stringb("Unexpected '" << (char)c << "' " <<
                context << "."));
  }
  else {
    err(stringb("Unexpected unprintable character code " <<
                (unsigned)c << " (0x" << std::hex << std::setw(2) <<
                std::setfill('0') << (unsigned)c <<
                ") " << context << "."));
  }

  // Not reached.
}


int Reader::readChar()
{
  int c = m_is.get();

  // TODO: Do the correct song-and-dance to interpret the result of
  // `get`:
  //
  //   https://stackoverflow.com/questions/24482728/return-value-of-istreamget

  // Update the location.
  //
  // Note: We do this even for EOF for uniformity.
  m_location.incrementForChar(c);

  return c;
}


void Reader::readCharOrErr(int expectChar, char const *lookingFor)
{
  processCharOrErr(readChar(), expectChar, lookingFor);
}


void Reader::processCharOrErr(int actualChar, int expectChar,
                                     char const *lookingFor)
{
  if (actualChar != expectChar) {
    unexpectedCharErr(actualChar, lookingFor);
  }
}


int Reader::readNotEOFCharOrErr(char const *lookingFor)
{
  int c = readChar();
  if (c == eofCode()) {
    unexpectedCharErr(c, lookingFor);
  }
  return c;
}


void Reader::putback(int c)
{
  if (c == eofCode()) {
    // It is convenient to allow this to make the parsing code more
    // uniform in its treatment of EOF versus other terminators in
    // some places.  But we do not actually put anything back into the
    // stream.
  }
  else {
    m_is.putback(c);
  }

  // Either way, however, the location must be decremented because it
  // was incremented when when we did the corresponding 'readChar', even
  // if it returned EOF.
  m_location.decrementForChar(c);
}


CLOSE_NAMESPACE(smbase)


// EOF
