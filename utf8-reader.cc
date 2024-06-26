// utf8-reader.cc
// Code for utf8-reader.h.

// This file is in the public domain.

#include "utf8-reader.h"               // this module

#include "exc.h"                       // THROW
#include "xassert.h"                   // xassert


OPEN_NAMESPACE(smbase)


// ------------------------ UTF8ReaderException ------------------------
UTF8ReaderException::UTF8ReaderException(
  FileLineCol const &location,
  std::string const &syntaxError,
  Kind kind)
  : ReaderException(location, syntaxError),
    m_kind(kind)
{}


// ---------------------------- UTF8Reader -----------------------------
void UTF8Reader::err(UTF8ReaderException::Kind kind,
                     std::size_t adjust,
                     std::string const &utf8Details) const
{
  FileLineCol loc = m_location;

  // The byte that caused the error may not be (typically is not) the
  // one at the current offset.
  while (adjust-- > 0) {
    loc.decrementColumn();
  }

  THROW(UTF8ReaderException(loc, utf8Details, kind));
}


unsigned char UTF8Reader::readContinuationByte()
{
  int c = readChar();
  if (c == eofCode()) {
    err(UTF8ReaderException::K_TRUNCATED_STREAM, 0 /*adjust*/,
      "The byte stream stops in the middle of a character encoding.");
  }

  xassert(0 <= c && c <= 0xFF);
  unsigned char b = (unsigned char)c;

  if ((b & 0xC0) != 0x80) {
    err(UTF8ReaderException::K_INVALID_CONTINUATION, 1 /*adjust*/,
      stringf("The byte 0x%02X is supposed to be a continuation byte "
              "but its 7th bit is set.", b));
  }
  return b;
}


// Compute the additive contribution of start byte `b` to the code point
// value.
static inline int startByte(
  unsigned char b,
  unsigned char mask,
  int shiftAmount)
{
  return ((int)(b & mask)) << shiftAmount;
}


// Compute the additive contribution of continuation byte `b` to the
// code point value.
static inline int continuationByte(
  unsigned char b,
  int shiftAmount)
{
  return ((int)(b & 0x3F)) << shiftAmount;
}


int UTF8Reader::readCodePointSlow(unsigned char b1)
{
  xassert(b1 >= 0x80);

  if (b1 <= 0xDF) {
    unsigned char b2 = readContinuationByte();

    return startByte(b1, 0x1F, 6) |
           continuationByte(b2, 0);
  }

  else if (b1 <= 0xEF) {
    unsigned char b2 = readContinuationByte();
    unsigned char b3 = readContinuationByte();

    int ret = startByte(b1, 0x0F, 12) |
              continuationByte(b2, 6) |
              continuationByte(b3, 0);

    if (0xD800 <= ret && ret <= 0xDFFF) {
      err(UTF8ReaderException::K_SURROGATE_PAIR, 3 /*adjust*/,
        stringf("The decoded code point is U+%04X, which is in the "
                "surrogate pair region.", ret));
    }

    return ret;
  }

  else if (b1 <= 0xF4) {
    unsigned char b2 = readContinuationByte();
    unsigned char b3 = readContinuationByte();
    unsigned char b4 = readContinuationByte();

    int ret = startByte(b1, 0x07, 18) |
              continuationByte(b2, 12) |
              continuationByte(b3, 6) |
              continuationByte(b4, 0);

    // Given that `b1` was at most 0xF4, it should not be possible for
    // the arithmetic to yield a value beyond 0x10FFFF.
    xassert(ret <= 0x10FFFF);

    return ret;
  }

  else {
    err(UTF8ReaderException::K_BYTE_TOO_LARGE, 1 /*adjust*/,
      stringf("The byte value 0x%02X is too large.", b1));
    return 0;      // Not reached.
  }
}


CLOSE_NAMESPACE(smbase)


// EOF
