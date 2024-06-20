// c-string-reader.h
// Read a C/C++ string with escape sequences.

#ifndef SMBASE_C_STRING_READER_H
#define SMBASE_C_STRING_READER_H

#include "sm-macros.h"                 // OPEN_NAMESPACE, ENUM_BITWISE_OPS

#include "reader.h"                    // smbase::Reader
#include "std-string-fwd.h"            // std::string [n]

#include <iosfwd>                      // std::{istream [n], ostream [n]}


OPEN_NAMESPACE(smbase)


// Set of boolean options to influence `CStringReader`.
enum CStringReaderFlags {
  CSRF_NONE                            = 0x00,

  // If not set, unescaped newlines trigger an error.
  CSRF_ALLOW_NEWLINES                  = 0x01,

  // If set, then if a denoted code point exceeds 0x10FFFF, silently
  // clamp it to that value.  Otherwise, report an error.
  CSRF_ALLOW_TOO_LARGE_CODE_POINTS     = 0x02,

  CSRF_ALL                             = 0x03
};

ENUM_BITWISE_OPS(CStringReaderFlags, CSRF_ALL);


// Manage the process of reading and decoding a C string literal.
class CStringReader : public Reader {
public:      // data
  // If non-zero, the delimiter that was used to enclose the string.
  // This is used for error detection.
  char m_delim;

  // Boolean options.
  CStringReaderFlags m_flags;

private:     // methods
  // Having read and consumed a backslash, interpret the following
  // characters as a backslash escape sequence and return the denoted
  // code point.
  int readEscapeSequence();

  // After seeing backslash, and having either read and consumed the
  // indicator digit 'x', or else read and put back an octal digit,
  // decode the successive digits as the indicated radix.
  int decodeHexOrOctal(bool hex);

  // Complain about an unquoted delimiter.
  void unquotedDelimErr() NORETURN;

  // Complain about an unquoted newline.
  void unquotedNewlineErr() NORETURN;

public:      // methods
  ~CStringReader();

  // This constructor does not accept a file name because I am basically
  // never using this directly on a file.  But the client can still set
  // the file name in the `m_location` member after construction if that
  // turns out to be necessary.
  CStringReader(std::istream &is, char delim, CStringReaderFlags flags);

  // Interpret the flags.
  bool allowNewlines() const
    { return !!(m_flags & CSRF_ALLOW_NEWLINES); }
  bool allowTooLargeCodePoints() const
    { return !!(m_flags & CSRF_ALLOW_TOO_LARGE_CODE_POINTS); }

  // Read the next denoted code point, or -1 upon EOF.
  int readCodePoint()
  {
    int c = readChar();
    if (c == eofCode()) {
      return -1;
    }
    else if (c == '\\') {
      // Slow path out of line.
      return readEscapeSequence();
    }
    else if (c == m_delim) {
      unquotedDelimErr();
    }
    else if (c == '\n' && !allowNewlines()) {
      unquotedNewlineErr();
    }
    else {
      return c;
    }
  }
};


/* Decode the characters in `str` and write them to `os`, expecting
   `str` to follow the syntax of the interior of a C string literal
   (i.e., without the delimiters).

   If `delim` is non-zero, then it is an error if there is an
   unescaped occurrence of that character.  If `allowNewlines` is false,
   it is an error to have an unescaped newline.

   Throw `ReaderException` if there is a problem with the syntax.

   Ideally `str` would be a `string_view` rather than a `string`, but
   the implementation uses `istringstream`, which does not work with
   string views until C++26.
*/
void decodeCStringEscapesToStream(
  std::ostream &os,
  std::string const &str,
  char delim = 0,
  CStringReaderFlags flags = CSRF_NONE);

// Same, but yielding the result as a string.
std::string decodeCStringEscapesToString(
  std::string const &str,
  char delim = 0,
  CStringReaderFlags flags = CSRF_NONE);

// Given a string enclosed by double-quotes and possibly containing C
// string literal escape sequences, yield the denoted string.
//
// This throws `smbase::XFormat` if the enclosing double-quotes are
// missing, and `smbase::ReaderException` if there is a problem with the
// interior.  (That is an unfortunate artifact of the implementation,
// but there's little value in rectifying it.)
std::string parseQuotedCString(std::string const &text,
  char delim = '"',
  CStringReaderFlags flags = CSRF_NONE);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_C_STRING_READER_H
