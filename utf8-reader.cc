// utf8-reader.cc
// Code for utf8-reader.h.

// This file is in the public domain.

#include "utf8-reader.h"               // this module

#include "exc.h"                       // THROW


OPEN_NAMESPACE(smbase)


// ------------------------ UTF8ReaderException ------------------------
/*static*/ std::string UTF8ReaderException::makeCondition(
  std::string const &utf8Details,
  std::ptrdiff_t byteOffset)
{
  return stringb(
    "Invalid UTF-8 encoding at byte offset " << byteOffset <<
    ": " << utf8Details);
}


UTF8ReaderException::UTF8ReaderException(
  Kind kind,
  std::string const &utf8Details,
  std::ptrdiff_t byteOffset)
  : xFormat(makeCondition(utf8Details, byteOffset)),
    m_kind(kind),
    m_utf8Details(utf8Details),
    m_byteOffset(byteOffset)
{}


// ---------------------------- UTF8Reader -----------------------------
void UTF8Reader::err(UTF8ReaderException::Kind kind,
                     std::string const &utf8Details)
{
  THROW(UTF8ReaderException(kind, utf8Details, m_current - m_start));
}


unsigned char UTF8Reader::readNextByte()
{
  if (m_current >= m_end) {
    err(UTF8ReaderException::K_TRUNCATED_STREAM,
      "The byte stream stops in the middle of a character encoding.");
  }
  char c = *m_current;
  ++m_current;
  return (unsigned char)c;
}


unsigned char UTF8Reader::readContinuationByte()
{
  unsigned char c = readNextByte();
  if ((c & 0xC0) != 0x80) {
    err(UTF8ReaderException::K_INVALID_CONTINUATION,
      stringf("The byte 0x%02X is supposed to be a continuation byte "
              "but its 7th bit is set.", c));
  }
  return c;
}


static inline int startByte(
  unsigned char b,
  unsigned char mask,
  int shiftAmount)
{
  return ((int)(b & mask)) << shiftAmount;
}


static inline int continuationByte(
  unsigned char b,
  int shiftAmount)
{
  return ((int)(b & 0x3F)) << shiftAmount;
}


int UTF8Reader::readCodePointSlow()
{
  unsigned char b1 = readNextByte();
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
      err(UTF8ReaderException::K_SURROGATE_PAIR,
        stringf("The decoded code point is U+%04X, which is in the "
                "surrogate pair region.", ret));
    }

    return ret;
  }

  else if (b1 <= 0xF4) {
    unsigned char b2 = readContinuationByte();
    unsigned char b3 = readContinuationByte();
    unsigned char b4 = readContinuationByte();

    return startByte(b1, 0x07, 18) |
           continuationByte(b2, 12) |
           continuationByte(b3, 6) |
           continuationByte(b4, 0);
  }

  else {
    err(UTF8ReaderException::K_BYTE_TOO_LARGE,
      stringf("The byte value 0x%02X is too large.", b1));
    return 0;      // Not reached.
  }
}



void UTF8Reader::selfCheck() const
{
  xassert(m_start <= m_current);
  xassert(           m_current <= m_end);
}


// ------------------------- UTF8StringReader --------------------------
UTF8StringReader::UTF8StringReader(std::string const &encodedInput)
  : m_encodedInput(encodedInput),
    m_reader(m_encodedInput.c_str(),
             m_encodedInput.c_str() + m_encodedInput.length())
{}


CLOSE_NAMESPACE(smbase)


// EOF
