// gdvalue-reader.h
// GDValueReader class, which does text deserialization for GDValue.

// This file is in the public domain.

#ifndef GDVALUE_READER_H
#define GDVALUE_READER_H

#include "file-line-col.h"             // FileLineCol
#include "gdvalue.h"                   // GDValue
#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(gdv)


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
  //
  // Naming convention: Any method that can call `err` in a fairly
  // direct way has an name that ends in "Err".  That way, it is easy to
  // find the places that need to be tested for syntax error detection
  // and reporting by searching for "err(" case-insensitively.
  //
  void err(std::string const &syntaxError) const;

  // Throw with 'loc-1' and 'syntaxError'.
  void locErr(FileLineCol const &loc,
              std::string const &syntaxError) const;

  // Report error: 'c' is unexpected.  'c' can be 'eofCode()', and the
  // message will be tailored accordingly.  'lookingFor' is a phrase
  // describing what the parser was looking for when 'c' was
  // encountered.
  void unexpectedCharErr(int c, char const *lookingFor) const;

  // Slightly more general version that does not insert the word
  // "while".
  void inCtxUnexpectedCharErr(int c, char const *context) const;

  // Read a single character from 'm_is', updating 'm_location' so it
  // refers to the *next* character.  (Thus, when we report an error, we
  // must use the immediately prior location.)  Returns 'eofCode()' on
  // end of file.
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

  // Read the remainder of the stream until EOF.  If anything besides
  // whitespace and comments are present, throw a syntax error.
  void readEOFOrErr();

  // Put 'c' back into 'm_is'.  It should be the same character as was
  // just read.  It is not possible to put back more than one character
  // between calls to 'readChar()'.  If 'c' is 'eofCode()', this does
  // nothing.
  void putback(int c);

  // True if 'c' is among the characters (including 'eofCode()') that
  // can directly follow the last character of a value.
  bool isAllowedAfterValue(int c);

  // If 'c' is not allowed after a value, throw an error.
  void checkAfterValueOrErr(int c);

  // Check that 'c' is allowed after a value and put it back.
  void putbackAfterValueOrErr(int c);

  // Skip whitespace and comments, returning the first character after
  // them, or 'eofCode()'.
  int skipWhitespaceAndComments();

  // Having seen and consumed "/*", scan the comment while balancing
  // those delimiters until the corresponding "*/" is found, then
  // return.  'nestingDepth' is the number of nested comments; 0 means
  // the comment we are about to scan is not nested in anything.
  void skipCStyleComment(int nestingDepth);

  // Having seen and consumed '(', read the following values and put
  // them into a sequence.  Return after consuming the ')'.
  GDValue readNextSequence();

  // Having seen and consumed "{{", read the following values and put
  // them into a set.  Return after consuming the "}}".
  GDValue readNextSet();

  // Having seen and consumed '{', read the following key/value pairs
  // and put them into a map.  Return after consuming the '}'.
  GDValue readNextMap();

  // Having seen and consumed '"', read the following characters and
  // put them into a string.  Return after consuming the final '"'.
  GDValue readNextDQString();

  // Having seen and consumed "\u", read the following four hexadecimal
  // characters and decode them as a UTF-16 code unit.
  int readNextU4Escape();

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


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_READER_H
