// parsestring.h
// ParseString class.

#ifndef PARSESTRING_H
#define PARSESTRING_H

#include "exc.h"                       // xFormat
#include "str.h"                       // string


// Thrown by ParseString when the string being parsed deviates from the
// expected format.
class XParseString : public xFormat {
public:      // data
  // String we were trying to parse.
  string m_str;

  // Byte offset within that string where the error happened.
  int m_offset;

  // Description of how the string at that location differed from the
  // expectations of the parser.
  string m_conflict;

public:      // funcs
  XParseString(string const &str, int offset, string const &conflict);
  XParseString(XParseString const &obj);
  ~XParseString();
};


// Holds a string to parse and current location within it.  It has
// routines to parse it, but it can also be used as an iterator over
// the characters in the string.
class ParseString {
private:     // data
  // String we are parsing.
  string m_str;

  // Length of the string.
  int m_len;

  // Current byte offset within that string.
  int m_cur;

public:      // funcs
  // This makes its own copy of the string.
  explicit ParseString(string const &str);
  ~ParseString();

  // Throw XParseString for the current offset.
  void throwErr(string const &conflict);

  // True if we are at (or beyond) the end of the string.
  bool eos() const { return m_cur >= m_len; }

  // Character at m_cur.  Requires '!eos()'.
  int cur() const;

  // Character at m_cur, quoted.
  string quoteCur() const;

  // Move to the next character.  Requires '!eos()'.
  void adv();

  // Advance past any whitespace characters.
  void skipWS();

  // All of these routines throw XParseString if the input does not
  // conform to expectations.

  // Advance past the next character, which should be 'c'.
  void parseChar(int c);

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
  string parseCToken();

  // Parse a C delimited literal, i.e., string or character, delimited
  // by 'c'.
  string parseCDelimLiteral(int c);

  // Parse a C number literal.  Currently only handles integers.
  string parseCNumberLiteral();

  // Parse a C identifier.
  string parseCIdentifier();
};


// Unit tests, in test-parsestring.cc.
void test_parsestring();


#endif // PARSE_H
