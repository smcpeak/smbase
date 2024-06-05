// reader.h
// `Reader` base class for `char` stream readers/parsers.

#ifndef SMBASE_READER_H
#define SMBASE_READER_H

#include "exc.h"                       // XBase
#include "file-line-col.h"             // FileLineCol
#include "sm-macros.h"                 // OPEN_NAMESPACE, NORETURN, NO_OBJECT_COPIES

#include <cstddef>                     // std::size_t
#include <iostream>                    // std::istream
#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// An exception thrown to indicate there is a problem with the input
// being read by a `Reader`.
class ReaderException : public XBase {
public:      // data
  // Where the error occurred.
  FileLineCol m_location;

  // What specifically is wrong with the input at that location?
  std::string m_syntaxError;

public:      // methods
  ~ReaderException();

  ReaderException(FileLineCol const &location,
                  std::string const &syntaxError) noexcept;

  ReaderException(ReaderException const &obj) = default;
  ReaderException &operator=(ReaderException const &obj) = default;

  // Prepend "context: " to `m_syntaxError`.
  void prependErrorContext(std::string const &context);

  // XBase methods
  virtual std::string getConflict() const override;
};


// Holds an input stream and a current location within it.
class Reader {
  NO_OBJECT_COPIES(Reader);

public:      // data
  // Data source.
  std::istream &m_is;

  // Where in that stream we currently are, i.e., the location of the
  // next character in 'm_is'.  The line/col part is initialized and
  // updated automatically, and the file is set in the constructor.  The
  // client can update either at any time.
  FileLineCol m_location;

public:      // methods
  ~Reader();

  Reader(std::istream &is,
         std::optional<std::string> fileName = std::nullopt);

  // Return the code that signals EOF from the input stream.
  static constexpr int eofCode()
    { return std::istream::traits_type::eof(); }

  // Throw ReaderException with 'm_location'-1 and 'syntaxError'.
  //
  // Naming convention: Any method that can call `err` in a fairly
  // direct way has an name that ends in "Err".  That way, it is easy to
  // find the places that need to be tested for syntax error detection
  // and reporting by searching for "err(" case-insensitively.
  //
  void err(std::string const &syntaxError) const NORETURN;

  // Throw with 'loc-1' and 'syntaxError'.
  void locErr(FileLineCol const &loc,
              std::string const &syntaxError) const NORETURN;

  // Report error: 'c' is unexpected.  'c' can be 'eofCode()', and the
  // message will be tailored accordingly.  'lookingFor' is a phrase
  // describing what the parser was looking for when 'c' was
  // encountered.
  void unexpectedCharErr(int c, char const *lookingFor) const NORETURN;

  // Slightly more general version that does not insert the word
  // "while".
  void inCtxUnexpectedCharErr(int c, char const *context) const NORETURN;

  /* Read a single character from 'm_is', updating 'm_location' so it
     refers to the *next* character.  (Thus, when we report an error, we
     must use the immediately prior location.)  Returns 'eofCode()' on
     end of file, or a non-negative character value otherwise.

     This updates `m_location`.  If the returned value is '\n', then
     the line is incremented and the column reset to 0; otherwise, the
     column is incremented.  The latter happens even for `eofCode()` so
     that the caller can consistently say that an error occurred one
     column earlier than the current location if the return value of
     `readChar` triggers an error.

     TODO: Return an instance of `CodePoint` instead of `int`.
  */
  int readChar();

  // Read the next character.  If it is not 'expectChar', call
  // 'unexpectedCharErr'.
  void readCharOrErr(int expectChar, char const *lookingFor);

  // Same, except we already read the character and it is 'actualChar'.
  // Compare it to 'expectChar', etc.
  void processCharOrErr(int actualChar, int expectChar,
                        char const *lookingFor);

  // Read the next character.  If it is EOF, call 'unexpectedCharErr'.
  int readNotEOFCharOrErr(char const *lookingFor);

  /* Put `c` back into `m_is`, thereby undoing the effect of the most
     recent call to `readChar`.

     `c` must be the same character as was just read.  It is not
     possible to put back more than one character between calls to
     `readChar()`.  If `c` is `eofCode()`, then no character is returned
     to the stream.

     Even if `c` is `eofCode()`, `m_location` decremented, although if
     it is '\n', then the old column information is lost (it is set to
     0).
  */
  void putback(int c);
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_READER_H
