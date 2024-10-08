// gdvalue-reader.h
// GDValueReader class, which does text deserialization for GDValue.

// This file is in the public domain.

#ifndef GDVALUE_READER_H
#define GDVALUE_READER_H

#include "smbase/file-line-col.h"      // FileLineCol
#include "smbase/gdvalue.h"            // GDValue
#include "smbase/reader.h"             // smbase::Reader
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(gdv)


// Manage the process of reading a GDValue from an istream
class GDValueReader : protected smbase::Reader {
protected:   // methods
  // Read the remainder of the stream until EOF.  If anything besides
  // whitespace and comments are present, throw a syntax error.
  void readEOFOrErr();

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

  // Having seen and parsed the first element of a sequence, read the
  // following values and append them to that sequence.  Return after
  // consuming the ']'.
  GDValue readSequenceAfterFirstValue(GDValue &&firstValue);

  // Having seen and consumed '(', read the following values and put
  // them into a sequence.  Return after consuming the ')'.
  GDValue readNextTuple();

  // Having seen and consumed '{' (in which case `ordered` is false) or
  // '[' (in which case `ordered` is ture), read what follows to first
  // determine whether it denotes a map, then parse and return the
  // entire container value.
  GDValue readNextPossibleMap(bool ordered);

  // Having seen '{' followed by `firstValue` and *not* a subsequent
  // colon, return the set consisting of `firstValue` and all of the
  // following values until '}'.
  GDValue readSetAfterFirstValue(GDValue &&firstValue);

  // Having seen '{' or '[' followed by `firstValue` and then a colon,
  // parse and return the remainder of the possibly-ordered map.
  GDValue readPossiblyOrderedMapAfterFirstKey(
    bool ordered, GDValue &&firstKey);

  // Having seen and consumed '"', read the following characters and
  // put them into a string.  Return after consuming the final '"'.
  GDValue readNextDQString();

  // Having seen and consumed `delim`, read the following characters and
  // put them into a string.  Return after consuming the final `delim`.
  std::string readNextQuotedStringContents(int delim);

  // Having seen and consumed "\u", read the following "universal
  // character" sequence.
  int readNextUniversalCharacterEscape();

  // Having seen and consumed "\u", and determined that the following
  // character is not '{', read the following four hexadecimal
  // characters and decode them as a UTF-16 code unit.
  int readNextU4Escape();

  // Having seen and consumed "\u{", read the following hexadecimal
  // characters up to the next '}' and return the denoted code point.
  int readNextDelimitedCharacterEscape();

  // Having seen and consumed 'firstChar', a character that starts an
  // integer (so, it is '-' or a digit), read the remainder and put them
  // into an integer.  Return after consuming the final digit.
  GDValue readNextInteger(int firstChar);

  // Having seen and consumed 'firstChar', a character that starts a
  // symbol, read the remainder and put them into a symbol.  Then, if
  // the immediately following character is '{', parse what follows as a
  // map and return the symbol and map together as a tagged map.
  // Otherwise just return the symbol as its own value.
  GDValue readNextSymbolOrTaggedContainer(int firstChar);

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
  // If a syntax error is encountered, throws `ReaderException'
  // (declared in `reader.h`).
  //
  std::optional<GDValue> readNextValue();

  // Read exactly one value from the stream and check that EOF occurs
  // after it.
  GDValue readExactlyOneValue();
};


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_READER_H
