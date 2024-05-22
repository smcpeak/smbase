// gdvalue-reader-exception.h
// GDValueReaderException class, used to report GDValue syntax errors.

#ifndef SMBASE_GDVALUE_READER_EXCEPTION_H
#define SMBASE_GDVALUE_READER_EXCEPTION_H

#include "gdvalue-reader-exception-fwd.h"        // fwds for this module

#include "exc.h"                                 // xFormat
#include "file-line-col.h"                       // FileLineCol


namespace gdv {


// Exception used to report a syntax error.
class GDValueReaderException : public xFormat {
public:      // data
  // Where the error occurred.
  FileLineCol m_location;

  // What specifically is wrong with the syntax at that location?
  std::string m_syntaxError;

public:      // methods
  GDValueReaderException(FileLineCol const &location,
                         std::string const &syntaxError) noexcept;
  ~GDValueReaderException();
};


} // namespace gdv


#endif // SMBASE_GDVALUE_READER_EXCEPTION_H
