// parsestring.h
// ParseString class.

#ifndef SMBASE_PARSESTRING_H
#define SMBASE_PARSESTRING_H

#include "exc.h"                       // smbase::XFormat

#include <cstddef>                     // std::size_t
#include <string>                      // std::string


// Thrown by ParseString when the string being parsed deviates from the
// expected format.
class XParseString : public smbase::XFormat {
public:      // data
  // String we were trying to parse.
  std::string m_str;

  // Byte offset within that string where the error happened.
  std::size_t m_offset;

  // Description of how the string at that location differed from the
  // expectations of the parser.
  std::string m_conflict;

public:      // funcs
  XParseString(
    std::string const &str,
    std::size_t offset,
    std::string const &conflict);
  XParseString(XParseString const &obj);
  ~XParseString();
};


// Holds a string to parse and current location within it.  It has
// routines to parse it, but it can also be used as an iterator over
// the characters in the string.
//
// This class operates on bytes in the range [0,255], which are
// represented using `int`.
//
class ParseString {
private:     // data
  // String we are parsing.
  std::string m_str;

  // Current byte offset within that string.  Should always be in
  // [0, m_str.size()].
  std::size_t m_curOffset;

public:      // funcs
  ~ParseString();

  // This makes its own copy of the string.
  explicit ParseString(std::string const &str);

  // Move the string contents.
  explicit ParseString(std::string      &&str);

  // Throw XParseString for the current offset.
  void throwErr(std::string const &conflict);

  // How many bytes of the string we have read.
  std::size_t curOffset() const { return m_curOffset; }

  // True if we are at (or beyond) the end of the string.
  bool eos() const { return m_curOffset >= m_str.size(); }

  // Byte value at m_curOffset, in [0,255].  Requires '!eos()'.
  int curByte() const;

  // Value of `curByte` represented as plain `char`.
  char curByteAsChar() const;

  // Byte at m_curOffset, quoted.
  std::string quoteCurByte() const;

  // Move to the next character.  Requires '!eos()'.
  void adv();

  // Advance past any whitespace characters.
  void skipWS();

  // All of these routines throw XParseString if the input does not
  // conform to expectations.

  // Advance past the next byte, which should be 'c'.
  void parseByte(int c);

  // Advance past the next sequence of characters, expecting them all to
  // match those in 's'.
  void parseString(char const *s);

  // Expect to be at the end of the string.
  void parseEOS();

  // Parse a non-empty sequence of decimal digits without sign as an
  // int.  Throws if the value is too large to represent.
  int parseDecimalUInt();

  // Parse the next sequence of characters as a single C token.
  //
  // This currently only handles literals and identifiers.
  std::string parseCToken();

  // Parse a C delimited literal, i.e., string or character, delimited
  // by 'c'.
  std::string parseCDelimLiteral(int c);

  // Parse a C number literal.  Currently only handles integers.
  std::string parseCNumberLiteral();

  // Parse a C identifier.
  std::string parseCIdentifier();

  // Read all bytes up to and including the first occurrence of `c`.  If
  // it does not occur, return all remaining bytes.
  std::string getUpToByte(int c);

  // Read bytes until we have read `size` of them.  If that is more than
  // the total, return all remaining bytes.
  std::string getUpToSize(std::size_t size);
};


// Unit tests, in parsestring-test.cc.
void test_parsestring();


#endif // SMBASE_PARSE_H
