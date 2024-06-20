// utf8-writer.cc
// Code for utf8-writer.h.

// This file is in the public domain.

#include "utf8-writer.h"               // this module

#include <iostream>                    // std::ostream [h]
#include <sstream>                     // std::ostringstream
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


static inline char startByte(
  int c,
  unsigned char fixedHighBits,
  unsigned char lowBitsMask,
  int shiftAmount)
{
  int v = ((c >> shiftAmount) & lowBitsMask) | fixedHighBits;
  return (char)(unsigned char)v;
}


static inline char continuationByte(
  int c,
  int shiftAmount)
{
  int v = ((c >> shiftAmount) & 0x3F) | 0x80;
  return (char)(unsigned char)v;
}


void UTF8Writer::writeCodePointSlow(int c)
{
  xassert(0x80 <= c && c <= 0x10FFFF);

  if (c <= 0x7FF) {
    m_os << startByte(c, 0xC0, 0x1F, 6);
    m_os << continuationByte(c, 0);
  }
  else if (c <= 0xFFFF) {
    m_os << startByte(c, 0xE0, 0x0F, 12);
    m_os << continuationByte(c, 6);
    m_os << continuationByte(c, 0);
  }
  else {
    m_os << startByte(c, 0xF0, 0x07, 18);
    m_os << continuationByte(c, 12);
    m_os << continuationByte(c, 6);
    m_os << continuationByte(c, 0);
  }
}


std::string utf8EncodeVector(std::vector<int> codePoints)
{
  std::ostringstream oss;
  UTF8Writer writer(oss);
  for (int i : codePoints) {
    writer.writeCodePoint(i);
  }
  return oss.str();
}


CLOSE_NAMESPACE(smbase)


// EOF
