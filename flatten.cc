// flatten.cc            see license.txt for copyright and terms of use
// code for flatten.h

// basically, this file provides some reasonable defaults
// assuming we are reading/writing binary files

#include "flatten.h"                   // this module

#include "exc.h"                       // formatAssert

#include <limits.h>                    // INT_MAX
#include <string.h>                    // strlen


Flatten::Flatten()
  : version(0)
{}

Flatten::~Flatten()
{}


void Flatten::xferChar(char &c)
{
  static_assert(sizeof(c) == 1, "");
  xferSimple(&c, sizeof(c));
}


void Flatten::xferBool(bool &b)
{
  static_assert(sizeof(b) == 1, "");
  xferSimple(&b, sizeof(b));
}


template <class DEST, class SRC>
void convertOrXFormat(DEST &dest, SRC const &src)
{
  dest = static_cast<DEST>(src);
  if (static_cast<SRC>(dest) != src) {
    xformat(stringb("convertOrXFormat: value " << src <<
                    " is outside representable range"));
  }
}


void Flatten::xferInt32(int &intValue)
{
  int32_t i32;
  if (reading()) {
    xfer_int32_t(i32);
    convertOrXFormat(intValue, i32);
  }
  else {
    convertOrXFormat(i32, intValue);
    xfer_int32_t(i32);
  }
}


void Flatten::xferLong64(long &intValue)
{
  int64_t i64;
  if (reading()) {
    xfer_int64_t(i64);
    convertOrXFormat(intValue, i64);
  }
  else {
    convertOrXFormat(i64, intValue);
    xfer_int64_t(i64);
  }
}


// Transfer 'intValue' in big-endian byte order.
template <class T>
void xferIntBigEndian(Flatten &flat, T &intValue)
{
  unsigned char bytes[sizeof(T)];
  if (flat.reading()) {
    flat.xferSimple(bytes, sizeof(T));

    intValue = 0;
    for (unsigned index=0; index < sizeof(T); index++) {
      intValue |= (T)(bytes[index]) << ((sizeof(T) - 1 - index) * 8);
    }
  }
  else {
    for (unsigned index=0; index < sizeof(T); index++) {
      bytes[index] = (unsigned char)(intValue >> ((sizeof(T) - 1 - index) * 8));
    }

    flat.xferSimple(bytes, sizeof(T));
  }
}


void Flatten::xfer_int64_t(int64_t &intValue)
{
  xfer_uint64_t((uint64_t&)intValue);
}


void Flatten::xfer_uint64_t(uint64_t &intValue)
{
  xferIntBigEndian(*this, intValue);
}


void Flatten::xfer_int32_t(int32_t &intValue)
{
  xfer_uint32_t((uint32_t&)intValue);
}


void Flatten::xfer_uint32_t(uint32_t &intValue)
{
  xferIntBigEndian(*this, intValue);
}


void Flatten::xferHeapBuffer(void *&buf, size_t len)
{
  if (reading()) {
    buf = new unsigned char[len];
  }
  xferSimple(buf, len);
}


void Flatten::xferCharString(char *&str)
{
  if (writing()) {
    if (!str) {
      writeInt32(-1);     // representation of NULL
      return;
    }

    size_t len = strlen(str);

    // Disallow INT_MAX since we add one below.
    if (len >= INT_MAX) {
      xformat(stringb("xferCharString: string length " << len <<
                      " is too large to serialize"));
    }

    writeInt32((int)len);

    // write the null terminator too, as a simple
    // sanity check when reading
    xferSimple(str, len+1);
  }
  else {
    int len = readInt32();
    if (len == -1) {
      str = NULL;
      return;
    }

    if (len < 0) {
      xformat("xferCharString: length is negative");
    }
    if (len == INT_MAX) {
      xformat("xferCharString: length is INT_MAX");
    }

    str = new char[len+1];
    xferSimple(str, len+1);
    formatAssert(str[len] == '\0');
  }
}


void Flatten::checkpoint32(uint32_t code)
{
  uint32_t c = code;
  xfer_uint32_t(c);

  if (reading()) {
    formatAssert(c == code);
  }
}


void Flatten::writeInt32(int i)
{
  xassert(writing());
  xferInt32(i);
}

int Flatten::readInt32()
{
  xassert(reading());
  int i;
  xferInt32(i);
  return i;
}


// EOF
