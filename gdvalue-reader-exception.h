// gdvalue-reader-exception.h
// GDValueReaderException class, used to report GDValue syntax errors.

// This file is in the public domain.

#ifndef SMBASE_GDVALUE_READER_EXCEPTION_H
#define SMBASE_GDVALUE_READER_EXCEPTION_H

#include "gdvalue-reader-exception-fwd.h"        // fwds for this module

#include "exc.h"                                 // smbase::XBase
#include "file-line-col.h"                       // FileLineCol
#include "sm-macros.h"                           // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


// Exception used to report a syntax error.
class GDValueReaderException : public smbase::XBase {
public:      // data
  // Where the error occurred.
  FileLineCol m_location;

  // What specifically is wrong with the GDVN syntax at that location?
  std::string m_syntaxError;

public:      // methods
  GDValueReaderException(FileLineCol const &location,
                         std::string const &syntaxError) noexcept;
  ~GDValueReaderException();

  // Prepend "context: " to `m_syntaxError`.
  void prependGDVNContext(std::string const &context);

  // XBase methods
  virtual std::string getConflict() const override;
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_READER_EXCEPTION_H
