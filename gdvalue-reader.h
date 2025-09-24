// gdvalue-reader.h
// GDValueReader class, which does text deserialization for GDValue.

// This file is in the public domain.

#ifndef GDVALUE_READER_H
#define GDVALUE_READER_H

#include "smbase/file-line-col-fwd.h"  // smbase::FileLineCol [n]
#include "smbase/gdvalue-fwd.h"        // GDValue [n]
#include "smbase/gdvalue-srcloc.h"     // GDValueSourceLocation
#include "smbase/reader.h"             // smbase::Reader
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::optional [n]
#include "smbase/std-string-fwd.h"     // std::string [n]
#include "smbase/std-vector-fwd.h"     // std::vector [n]

#include <cstdint>                     // std::uint32_t
#include <iosfwd>                      // std::istream [n]


OPEN_NAMESPACE(gdv)


// A character read from the input and the location where it occurs.
class GDValueSourceLocationAndChar {
public:      // data
  // Location where `m_c` was read.
  GDValueSourceLocation m_loc;

  // The character value read.
  int m_c;

public:      // methods
  // ---- create-tuple-class: declarations for GDValueSourceLocationAndChar
  /*AUTO_CTC*/ explicit GDValueSourceLocationAndChar(GDValueSourceLocation const &loc, int c);
  /*AUTO_CTC*/ GDValueSourceLocationAndChar(GDValueSourceLocationAndChar const &obj) noexcept;
  /*AUTO_CTC*/ GDValueSourceLocationAndChar &operator=(GDValueSourceLocationAndChar const &obj) noexcept;
};


// Manage the process of reading a GDValue from an istream.
class GDValueReader : protected smbase::Reader {
public:      // types
  using FileIndex = GDValueSourceLocation::FileIndex;

public:      // data
  // The file index to include in the value source locations.  0 means
  // no file.
  FileIndex m_fileIndex;

protected:   // methods
  // Get the location of the next character plus `columnOffset`.
  GDValueSourceLocation gdvLocOffset(int columnOffset) const;

  // Get the source location of the character before the next one to
  // consume.
  GDValueSourceLocation gdvLocPrevChar() const;

  // Get the source location of the next character to read.
  GDValueSourceLocation gdvLoc() const;

  // Read the next character and get its location.
  GDValueSourceLocationAndChar readLocChar();

  // Read the remainder of the stream until EOF.  If anything besides
  // whitespace and comments are present, throw a syntax error.
  void readEOFOrErr();

  // Report that the character in `locChar` was not what we were
  // `lookingFor`.
  void unexpectedLocCharErr(
    GDValueSourceLocationAndChar const &locChar,
    char const *lookingFor);

  // True if 'c' is among the characters (including 'eofCode()') that
  // can directly follow the last character of a value.
  bool isAllowedAfterValue(int c);

  // If 'c' is not allowed after a value, throw an error.
  void checkAfterValueOrErr(int c);

  // Check that 'c' is allowed after a value and put it back.
  void putbackAfterValueOrErr(int c);

  // Skip whitespace and comments, returning the first character after
  // them, or 'eofCode()'.
  int readCharAfterWhitespaceAndComments();

  // Same, but with location.
  GDValueSourceLocationAndChar readLocCharAfterWhitespaceAndComments();

  // Having seen and consumed "/*", scan the comment while balancing
  // those delimiters until the corresponding "*/" is found, then
  // return.  'nestingDepth' is the number of nested comments; 0 means
  // the comment we are about to scan is not nested in anything.
  void skipCStyleComment(int nestingDepth);

  // Having seen and parsed the first element of a sequence, read the
  // following values and append them to that sequence.  Return after
  // consuming the ']'.
  GDValue readSequenceAfterFirstValue(
    GDValueSourceLocationAndChar const &openingDelim,
    GDValue &&firstValue);

  // Having seen and consumed '(', read the following values and put
  // them into a sequence.  Return after consuming the ')'.
  GDValue readNextTuple(
    GDValueSourceLocationAndChar const &openingDelim);

  // Having seen and consumed '{' or '[', read what follows to first
  // determine whether it denotes a map, then parse and return the
  // entire container value.
  GDValue readNextPossibleMap(
    GDValueSourceLocationAndChar const &openingDelim);

  // Having seen '{' followed by `firstValue` and *not* a subsequent
  // colon, return the set consisting of `firstValue` and all of the
  // following values until '}'.
  GDValue readSetAfterFirstValue(
    GDValueSourceLocationAndChar const &openingDelim,
    GDValue &&firstValue);

  // Having seen '{' or '[' followed by `firstValue` and then a colon,
  // parse and return the remainder of the possibly-ordered map.
  GDValue readPossiblyOrderedMapAfterFirstKey(
    GDValueSourceLocationAndChar const &openingDelim,
    bool ordered,
    GDValue &&firstKey);

  // Having seen and consumed '"', read the following characters and
  // put them into a string.  Return after consuming the final '"'.
  GDValue readNextDQString(
    GDValueSourceLocationAndChar const &openingDelim);

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

  // Having seen and consumed 'firstChar', a character that starts a
  // number (so, it is '-' or a digit), read the remainder and put them
  // into a number, depending on what follows.  Return after consuming
  // the final digit.
  GDValue readNextNumber(
    GDValueSourceLocationAndChar const &firstChar);

  // We have seen the start of a number (at `firstLoc`) and accumulated
  // it into `digits`.  We then read either a decimal point or the 'e'
  // or 'E' that starts an exponent, which is in `c`.  Add that to
  // `digits` and continue reading the rest of the float.  Return the
  // denoted value after reading the final digit.
  GDValue continueReadingFloat(
    GDValueSourceLocation firstLoc,
    std::vector<char> &digits,
    int c);

  // Having seen and consumed 'firstChar', a character that starts a
  // symbol, read the remainder and put them into a symbol.  Then, if
  // the immediately following character is '{' or '[' or '(', parse
  // what follows as a container and return it with the symbol as a
  // tagged container.  Otherwise just return the symbol as its own
  // value.
  GDValue readNextSymbolOrTaggedContainer(
    GDValueSourceLocationAndChar const &firstChar);

public:      // methods
  GDValueReader(std::istream &is,
                std::optional<std::string> fileName);
  ~GDValueReader();

  // Get the current location.  The reference is invalidated by calling
  // any non-const method.
  smbase::FileLineCol const &getLocation() const;

  /* Advance past any whitespace and comments, such that `getLocation()`
     is the location of whatever follows them.  The intent is to use
     this as part of a sequence like:

       reader.skipWhitespaceAndComments();
       FileLineCol flc = reader.getLocation();
       if (auto valueOpt = reader.readNextValue()) {
         // Use `valueOpt`, knowing that `flc` is where it begins.
       }

     TODO: This approach is obsoleted by having `GDValue` directly store
     a source location.  Delete it once I've changed the users.
  */
  void skipWhitespaceAndComments();

  // Read the next value from 'm_is'.  It must read enough to determine
  // that the value is complete, and will block if it is not.  It will
  // leave the input stream at the character after the last in the
  // value, typically using istream::putback to do that.
  //
  // If the end of the input or a closing delimiter is encountered
  // without finding any value, returns 'nullopt'.  Note that this is
  // different from a GDValue that 'isNull()'.
  //
  // If a syntax error is encountered, throws `XReader' (declared in
  // `reader.h`).
  //
  std::optional<GDValue> readNextValue();

  // Read exactly one value from the stream and check that EOF occurs
  // after it.
  GDValue readExactlyOneValue();
};


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_READER_H
