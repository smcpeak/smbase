// utf8-reader.h
// Decode UTF-8.

// This file is in the public domain.

#ifndef SMBASE_UTF8_READER_H
#define SMBASE_UTF8_READER_H

#include "exc.h"                       // XBase
#include "reader.h"                    // Reader, ReaderException
#include "xassert.h"                   // xassert

#include <cstddef>                     // std::size_t
#include <iostream>                    // std::istream
#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// This module assumes 'int' can store all Unicode code points.
static_assert(sizeof(int) >= 4);


// Report an issue with UTF-8 input.
class UTF8ReaderException : public ReaderException {
public:      // types
  enum Kind {
    K_UNKNOWN,

    // The byte stream ended in the middle of a UTF-8 byte sequence.
    K_TRUNCATED_STREAM,

    // A continuation byte has its 7th bit set.
    K_INVALID_CONTINUATION,

    // The decoded code point value is in the surrogate pair region.
    K_SURROGATE_PAIR,

    // The encoding byte is too large to appear anywhere in UTF-8.
    K_BYTE_TOO_LARGE,

    NUM_KINDS
  };

public:      // data
  // What sort of problem happened.
  Kind m_kind;

public:      // methods
  UTF8ReaderException(
    FileLineCol const &location,
    std::string const &syntaxError,
    Kind kind);

  UTF8ReaderException(UTF8ReaderException const &obj) = default;
  UTF8ReaderException &operator=(UTF8ReaderException const &obj) = default;
};


// Manage process of reading from an input stream.
class UTF8Reader : public Reader {
private:     // methods
  // Throw an exception due to malformed input.
  //
  // Adjust `m_curByteOffset` by subtracting `adjust` before putting it
  // into the exception object.
  void err(UTF8ReaderException::Kind kind,
           std::size_t adjust,
           std::string const &utf8Details) const;

  // Read the next byte and check it against the continuation byte
  // restrictions.
  unsigned char readContinuationByte();

  // Same as 'readCodePoint' but for the case where the value is not in
  // [0,127].
  int readCodePointSlow(unsigned char firstByte);

public:      // methods
  UTF8Reader(std::istream &is,
             std::optional<std::string> fileName = std::nullopt)
    : Reader(is, fileName)
  {}

  // Read the next Unicode code point from `m_current`, advancing it.
  // If there is an encoding error, throw UTF8Exception.  If the end of
  // the input reached, return -1.
  int readCodePoint()
  {
    int c = readChar();
    if (c == eofCode()) {
      return -1;
    }

    if (c <= 127) {
      // Inline fast path.
      return c;
    }
    else {
      // Out of line slow path.
      xassert(128 <= c && c <= 255);
      return readCodePointSlow((unsigned char)c);
    }
  }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_UTF8_READER_H
