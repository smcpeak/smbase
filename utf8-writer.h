// utf8-writer.h
// Encode UTF-8.

// This file is in the public domain.

#ifndef SMBASE_UTF8_WRITER_H
#define SMBASE_UTF8_WRITER_H

#include "xassert.h"                   // xassert

#include <iostream>                    // std::ostream


namespace smbase { // see smbase-namespace.txt


// This module assumes 'int' can store all Unicode code points.
static_assert(sizeof(int) >= 4);


// Write encode UTF-8 octets to a character stream.
//
// Right now, using a class to manage this is overkill.  But I'm
// thinking of a future optimization that maintains a private buffer,
// and that will be easier to do if the interface already supports it.
//
class UTF8Writer {
private:     // data
  // Where to send the encoded data.
  std::ostream &m_os;

private:     // methods
  // Same as `writeCodePoint` but for values larger than 127.
  void writeCodePointSlow(int c);

public:      // methods
  UTF8Writer(std::ostream &os)
    : m_os(os)
  {}

  // Encode `c` in UTF-8 and write its octets to `m_os`.
  void writeCodePoint(int c)
  {
    xassert(0 <= c && c <= 0x10FFFF);

    if (c <= 127) {
      // Inline fast path.
      m_os << (char)c;
    }
    else {
      // Out of line slow path.
      writeCodePointSlow(c);
    }
  }
};


} // namespace smbase


#endif // SMBASE_UTF8_WRITER_H
