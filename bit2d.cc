// bit2d.cc            see license.txt for copyright and terms of use
// code for bit2d.h

#include "bit2d.h"      // this module
#include "xassert.h"    // xassert
#include "flatten.h"    // Flatten

#include <string.h>     // memset, memcpy
#include <stdio.h>      // printf


Bit2d::Bit2d(point const &aSize)
  : owning(true),
    size(aSize)
{
  xassert(size.x > 0 && size.y > 0);
  stride = (size.x+7)/8;
  data = new byte[datasize()];
}


Bit2d::~Bit2d()
{
  if (owning) {
    delete[] data;
  }
}


Bit2d::Bit2d(Bit2d const &obj)
{
  size = obj.size;
  stride = obj.stride;
  data = new byte[datasize()];
  owning = true;
  memcpy(data, obj.data, datasize());
}


Bit2d& Bit2d::operator= (Bit2d const &obj)
{
  if (this != &obj) {
    xassert(size == obj.size);
    memcpy(data, obj.data, datasize());
  }
  return *this;
}


bool Bit2d::operator== (Bit2d const &obj) const
{
  return (size == obj.size) &&
         (0==memcmp(data, obj.data, datasize()));
}


Bit2d::Bit2d(Flatten &)
  : data(NULL),
    owning(true),
    size(),
    stride(0)
{}

void Bit2d::xfer(Flatten &flat)
{
  flat.xferInt32(size.x);
  flat.xferInt32(size.y);
  flat.xferInt32(stride);

  flat.xferHeapBuffer((void*&)data, datasize());
}


void Bit2d::setall(int val)
{
  memset(data, val? 0xFF : 0, datasize());
}


int Bit2d::get(point const &p) const
{
  xassert(okpt(p));
  return ( *(byteptrc(p)) >> (p.x&7) ) & 1;
}

void Bit2d::set(point const &p)
{
  xassert(okpt(p));
  *(byteptr(p)) |= (byte)  ( 1 << (p.x&7) ) ;
}

void Bit2d::reset(point const &p)
{
  xassert(okpt(p));
  *(byteptr(p)) &= (byte)(~( 1 << (p.x&7) ));
}

void Bit2d::setto(point const &p, int val)
{
  if (val) { set(p); }
  else { reset(p); }
}

int Bit2d::testAndSet(point const &p)
{
  byte *b = byteptr(p);
  int ret = (*b >> (p.x&7)) & 1;
  *b |= (byte)( 1 << (p.x&7) );
  return ret;
}

void Bit2d::toggle(point const &p)
{
  xassert(okpt(p));
  *(byteptr(p)) ^= (byte) ( 1 << (p.x&7) );
}


void Bit2d::set8(point const &p, byte val)
{
  xassert(okpt(p));
  xassert(p.x % 8 == 0);

  *(byteptr(p)) = val;
}


byte Bit2d::get8(point const &p) const
{
  xassert(okpt(p));
  xassert(p.x % 8 == 0);

  byte ret = *(byteptrc(p));

  // Zero any padding bits
  if (p.x + 8 > size.x) {
    int numPadBits = (p.x + 8) - size.x;
    xassert(0 < numPadBits && numPadBits < 8);

    byte padMask = 0xFF << (8 - numPadBits);
    ret &= ~padMask;
  }

  return ret;
}


// Count the number of digits required to represent a non-negative
// integer in base 10.
static int digits(int value)
{
  xassert(value >= 0);

  if (value == 0) {
    return 1;
  }

  int ct=0;
  while (value > 0) {
    ct++;
    value /= 10;
  }
  return ct;
}


/*
 * Goal is to draw something like this:
 *
 *     	 0  1  2
 *  0 [	 0  0  0  ]
 *  1 [	 0  1  1  ]
 *  2 [	 0  1  0  ]
 *
 */
void Bit2d::print() const
{
  if (size.x <= 0 || size.y <= 0) {
    printf("Degenerate Bit2d with dimensions (%d,%d)\n", size.x, size.y);
    fflush(stdout);
    return;
  }

  // compute column widths
  int rowLabelWidth = digits(size.y-1);
  int colLabelWidth = digits(size.x-1);

  // column legend
  printf("%*s   ", rowLabelWidth, "");
  loopi(size.x) {
    printf("%*d ", colLabelWidth, i);
  }
  printf("\n");

  for (int row=0; row<size.y; row++) {
    printf("%*d [ ", rowLabelWidth, row);
    loopi(size.x) {
      printf("%*s ", colLabelWidth,
                     get(point(i, row))? "1" : ".");    // "." so easier to see patterns
    }
    printf("]\n");
  }

  fflush(stdout);
}


// hack
Bit2d::Bit2d(byte * /*serf*/ d, point const &sz, int str)
  : data(d),
    owning(false),    // since it's a serf ptr
    size(sz),
    stride(str)
{}


byte byteBitSwapLsbMsb(byte b)
{
  // Map from [0,15] to the result of swapping the bit order.
  static const byte swapMap[] = {
    // input   output   hex-output
    /* 0000    0000 */  0x0,
    /* 0001    1000 */  0x8,
    /* 0010    0100 */  0x4,
    /* 0011    1100 */  0xC,
    /* 0100    0010 */  0x2,
    /* 0101    1010 */  0xA,
    /* 0110    0110 */  0x6,
    /* 0111    1110 */  0xE,
    /* 1000    0001 */  0x1,
    /* 1001    1001 */  0x9,
    /* 1010    0101 */  0x5,
    /* 1011    1101 */  0xD,
    /* 1100    0011 */  0x3,
    /* 1101    1011 */  0xB,
    /* 1110    0111 */  0x7,
    /* 1111    1111 */  0xF,
  };
  ASSERT_TABLESIZE(swapMap, 16);

  // divide into two 4-bit nibbles
  byte hi = b >> 4;
  byte lo = b & 0xF;

  // swap each nibble
  hi = swapMap[hi];
  lo = swapMap[lo];

  // combine into a byte in opposite nibble order
  return (lo << 4) | hi;
}


// EOF
