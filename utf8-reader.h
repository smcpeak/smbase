// utf8-reader.h
// Decode UTF-8.

// This file is in the public domain.

#ifndef SMBASE_UTF8_READER_H
#define SMBASE_UTF8_READER_H

#include "exc.h"                       // xBase


namespace smbase {


// This module assumes 'int' can store all Unicode code points.
static_assert(sizeof(int) >= 4);


enum UTF8ErrorKind {
  ...
};


class UTF8Exception : public xBase {
public:      // data
  UTF8ErrorKind m_kind;
};


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
  // Same as 'readCodePoint' but for the case where the value is not in
  // [0,127].
  int readCodePointSlow();

public:      // methods
  UTF8Reader(char const *start, char const *end)
    : m_start(start),
      m_current(start),
      m_end(end)
  {}

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


} // namespace smbase


#endif // SMBASE_UTF8_READER_H
