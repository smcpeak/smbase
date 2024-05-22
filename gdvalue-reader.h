// gdvalue-reader.h
// GDValueReader class, which does text deserialization for GDValue.

#ifndef GDVALUE_READER_H
#define GDVALUE_READER_H

#include "file-line-col.h"             // FileLineCol
#include "gdvalue.h"                   // GDValue

#include <optional>                    // std::optional
#include <string>                      // std::string


namespace gdv {


// Manage the process of reading a GDValue from an istream
class GDValueReader {
public:      // data
  // Input stream we are consuming.
  std::istream &m_is;

  // Where in that stream we currently are, i.e., the location of the
  // next character in 'm_is'.
  FileLineCol m_location;

protected:   // methods
  // Return the code that signals EOF from the input stream.
  static constexpr int eofCode();

  // Throw GDValueReaderException with 'm_location'-1 and 'syntaxError'.
  void err(std::string const &syntaxError) const;

  // Report error: 'c' is unexpected.  'c' can be 'eofCode()', and the
  // message will be tailored accordingly.  'lookingFor' is a phrase
  // describing what the parser was looking for when 'c' was
  // encountered.
  void errUnexpectedChar(int c, char const *lookingFor) const;

  // Read a single character from 'm_is', updating 'm_location' so it
  // refers to the *next* character.  (Thus, when we report an error, we
  // must use the immediately prior location.)  Returns 'eofCode()' on
  // end of file.
  int readChar();

  // Read the next character.  If it is not 'expectChar', call
  // 'errUnexpectedChar'.
  void readExpectChar(int expectChar, char const *lookingFor);

  // Same, except we already read the character and it is 'actualChar'.
  // Compare it to 'expectChar', etc.
  void processExpectChar(int actualChar, int expectChar,
                         char const *lookingFor);

  // Read the next character.  If it is EOF, call 'errUnexpectedChar'.
  int readCharNotEOF(char const *lookingFor);

  // Read the remainder of the stream until EOF.  If anything besides
  // whitespace and comments are present, throw a syntax error.
  void readExpectEOF();

  // Put 'c' back into 'm_is'.  It should be the same character as was
  // just read.  It is not possible to put back more than one character
  // between calls to 'readChar()'.  If 'c' is 'eofCode()', this does
  // nothing.
  void putback(int c);

  // Skip whitespace and comments, returning the first character after
  // them, or 'eofCode()'.
  int skipWhitespaceAndComments();

  // Having seen and consumed "/*", scan the comment while balancing
  // those delimiters until the corresponding "*/" is found, then
  // return.  'nestingDepth' is the number of nested comments; 0 means
  // the comment we are about to scan is not nested in anything.
  void skipCStyleComment(int nestingDepth);

  // Having seen and consumed '(', read the following values and put
  // them into a vector.  Return after consuming the ')'.
  GDValue readNextVector();

  // Having seen and consumed "{{", read the following values and put
  // them into a set.  Return after consuming the "}}".
  GDValue readNextSet();

  // Having seen and consumed '{', read the following key/value pairs
  // and put them into a map.  Return after consuming the '}'.
  GDValue readNextMap();

  // Having seen and consumed '"', read the following characters and
  // put them into a string.  Return after consuming the final '"'.
  GDValue readNextDQString();

  // Having seen and consumed 'firstChar', a character that starts an
  // integer (so, it is '-' or a digit), read the remainder and put them
  // into an integer.  Return after consuming the final digit.
  GDValue readNextInteger(int firstChar);

  // Having seen and consumed 'firstChar', a character that starts a
  // symbol or special name, read the remainder and put them into a
  // GDVK_SYMBOL, GDVK_NULL, or GDKV_BOOL.
  GDValue readNextSymbolOrSpecial(int firstChar);

public:      // methods
  GDValueReader(std::istream &is,
                std::optional<std::string> fileName);
  ~GDValueReader();

  // Read the next value from 'm_is'.  It must read enough to determine
  // that the value is complete, and will block if it is not.  It will
  // leave the input stream at the character after the last in the
  // value, typically using istream::putback to do that.
  //
  // If the end of the input or a closing delimiter is encountered
  // without finding any value, returns 'nullopt'.  Note that this is
  // different from a GDValue that 'isNull()'.
  //
  // If a syntax error is encountered, throws 'GDValueReaderException'
  // (declared in gdvalue-reader-exception.h).
  //
  std::optional<GDValue> readNextValue();

  // Read exactly one value from the stream and check that EOF occurs
  // after it.
  GDValue readExactlyOneValue();
};


} // namespace gdv


#endif // GDVALUE_READER_H
