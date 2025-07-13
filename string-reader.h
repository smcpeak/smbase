// string-reader.h
// `StringReader` class.

#ifndef SMBASE_STRING_READER_H
#define SMBASE_STRING_READER_H

#include "smbase/reader.h"             // Reader
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <sstream>                     // std::istringstream
#include <utility>                     // std::forward


OPEN_NAMESPACE(smbase)


// Candidate for being moved to someplace more general.
template <typename T>
class DataWrapper {
public:      // data
  T m_wrappedData;

public:      // methods
  // Mirror the constructors of `T`.
  template <typename... Args>
  explicit DataWrapper(Args&&... args)
    : m_wrappedData(std::forward<Args>(args)...)
  {}
};


// Read from a string.
class StringReader :
  // DataWrapper stream carrying the string to read.
  //
  // This has to be a base class that precedes `Reader` because it gets
  // passed to the ctor of `Reader`, and during that passing, undergoes
  // derived-to-base conversion.  See
  // https://stackoverflow.com/questions/78515255/is-it-undefined-behavior-to-pass-a-pointer-to-an-unconstructed-streambuf-object
  // which primarily cites class.cdtor p1.
  private DataWrapper<std::istringstream>,

  // Main interface.
  public Reader
{
public:      // methods
  ~StringReader();

  StringReader(std::string const &str,
               std::optional<std::string> fileName = std::nullopt);

  std::istringstream const &istrstr() const { return m_wrappedData; }
  std::istringstream       &istrstr()       { return m_wrappedData; }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_STRING_READER_H
