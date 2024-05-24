// utf8-reader.h
// Decode UTF-8.

// This file is in the public domain.

#ifndef SMBASE_UTF8_READER_H
#define SMBASE_UTF8_READER_H

#include "exc.h"                       // xFormat

#include <cstddef>                     // std::ptrdiff_t
#include <string>                      // std::string


namespace smbase { // see smbase-namespace.txt


// This module assumes 'int' can store all Unicode code points.
static_assert(sizeof(int) >= 4);


// Report an issue with UTF-8 input.
class UTF8ReaderException : public xFormat {
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

  // The specific complaint about the UTF-8 input.  (In contrast,
  // `xFormat::condition` combines all of the information into one
  // string.)
  std::string m_utf8Details;

  // Byte offset from `m_start` in the throwing reader.
  std::ptrdiff_t m_byteOffset;

private:     // methods
  // Create the `xFormat::condition` string.
  static std::string makeCondition(
    std::string const &utf8Details,
    std::ptrdiff_t byteOffset);

public:      // methods
  UTF8ReaderException(
    Kind kind,
    std::string const &utf8Details,
    std::ptrdiff_t byteOffset);

  UTF8ReaderException(UTF8ReaderException const &obj) = default;
  UTF8ReaderException &operator=(UTF8ReaderException const &obj) = default;
};


// Manage process of reading from an array of char.
//
// TODO: This should just read from a 'std::istream', symmetric to how
// UTFWriter works.
class UTF8Reader {
public:      // data
  // Start of the input being processed.  This is used to report
  // error location information.
  char const *m_start;

  // Beginning of the next UTF-8 character.
  char const *m_current;

  // One-past-the-end pointer to the string that `m_current` points to.
  char const *m_end;

  // Invariant: m_start <= m_current <= m_end

private:     // methods
  // Throw an exception due to malformed input.
  void err(UTF8ReaderException::Kind kind,
           std::string const &utf8Details);

  // Read the next byte of input, throwing if we run out.
  unsigned char readNextByte();

  // Read the next byte and check it against the continuation byte
  // restrictions.
  unsigned char readContinuationByte();

  // Same as 'readCodePoint' but for the case where the value is not in
  // [0,127].
  int readCodePointSlow();

public:      // methods
  UTF8Reader(char const *start, char const *end)
    : m_start(start),
      m_current(start),
      m_end(end)
  {
    selfCheck();
  }

  // True if there are more characters to read.
  bool hasMore() const { return m_current < m_end; }

  // Read the next Unicode code point from `m_current`, advancing it.
  // If there is an encoding error, throw UTF8Exception.  If the end of
  // the input reached, return -1.
  int readCodePoint()
  {
    if (m_current < m_end) {
      int c = (unsigned char)(*m_current);
      if (c <= 127) {
        // Inline fast path.
        ++m_current;
        return c;
      }
      else {
        // Out of line slow path.
        return readCodePointSlow();
      }
    }
    else {
      return -1;
    }
  }

  // Assert invariants.
  void selfCheck() const;
};


// Read from a std::string.
class UTF8StringReader {
public:      // data
  // The input to decode as a string.
  std::string m_encodedInput;

  // The inner reader.
  UTF8Reader m_reader;

public:
  UTF8StringReader(std::string const &encodedInput);

  bool hasMore() const { return m_reader.hasMore(); }
  int readCodePoint() { return m_reader.readCodePoint(); }
  void selfCheck() { m_reader.selfCheck(); }
};


} // namespace smbase


#endif // SMBASE_UTF8_READER_H
