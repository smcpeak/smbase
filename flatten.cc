// flatten.cc            see license.txt for copyright and terms of use
// code for flatten.h

// basically, this file provides some reasonable defaults
// assuming we are reading/writing binary files

#include "flatten.h"     // this module
#include "exc.h"         // formatAssert
#include <string.h>      // strlen

Flatten::Flatten()
  : version(0)
{}

Flatten::~Flatten()
{}


void Flatten::xferChar(char &c)
{
  xferSimple(&c, sizeof(c));
}

void Flatten::xferInt(int &i)
{
  xferSimple(&i, sizeof(i));
}

void Flatten::xferLong(long &l)
{
  xferSimple(&l, sizeof(l));
}

void Flatten::xferBool(bool &b)
{
  xferSimple(&b, sizeof(b));
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


void Flatten::xferHeapBuffer(void *&buf, int len)
{
  if (reading()) {
    buf = new unsigned char[len];
  }
  xassert(len >= 0);
  xferSimple(buf, len);
}


void Flatten::xferCharString(char *&str)
{
  if (writing()) {
    if (!str) {
      writeInt(-1);     // representation of NULL
      return;
    }

    int len = strlen(str);
    writeInt(len);

    // write the null terminator too, as a simple
    // sanity check when reading
    xferSimple(str, len+1);
  }
  else {
    int len = readInt();
    if (len == -1) {
      str = NULL;
      return;
    }

    str = new char[len+1];
    xferSimple(str, len+1);
    formatAssert(str[len] == '\0');
  }
}


void Flatten::checkpoint(int code)
{
  if (writing()) {
    writeInt(code);
  }
  else {
    int c = readInt();
    formatAssert(c == code);
  }
}


void Flatten::writeInt(int i)
{
  xassert(writing());
  xferInt(i);
}

int Flatten::readInt()
{
  xassert(reading());
  int i;
  xferInt(i);
  return i;
}

