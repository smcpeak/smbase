// bitarray.cc            see license.txt for copyright and terms of use
// code for bitarray.h

#include "bitarray.h"     // this module
#include "flatten.h"      // Flatten

#include <string.h>       // memset


BitArray::BitArray(int n)
  : numBits(n)
{
  allocBits();
  clearAll();
}

void BitArray::allocBits()
{
  bits = new unsigned char[allocdBytes()];
}


BitArray::~BitArray()
{
  delete[] bits;
}


BitArray::BitArray(Flatten&)
  : bits(NULL)
{}

void BitArray::xfer(Flatten &flat)
{
  flat.xferInt32(numBits);

  if (flat.reading()) {
    allocBits();
  }
  flat.xferSimple(bits, allocdBytes());
}


BitArray::BitArray(BitArray const &obj)
  : numBits(obj.numBits)
{
  allocBits();
  memcpy(bits, obj.bits, allocdBytes());
}

void BitArray::operator=(BitArray const &obj)
{
  if (numBits != obj.numBits) {
    delete[] bits;
    numBits = obj.numBits;
    allocBits();
  }
  memcpy(bits, obj.bits, allocdBytes());
}


bool BitArray::operator== (BitArray const &obj) const
{
  if (numBits != obj.numBits) {
    return false;
  }

  // this relies on the invariant that the unused trailing
  // bits are always set to 0
  return 0==memcmp(bits, obj.bits, allocdBytes());
}


void BitArray::clearAll()
{
  memset(bits, 0, allocdBytes());
}


void BitArray::invert()
{
  int allocd = allocdBytes();
  for (int i=0; i<allocd; i++) {
    bits[i] = ~(bits[i]);
  }

  if (numBits & 7) {
    // there are some trailing bits that I need to flip back
    unsigned char mask = (1 << (numBits & 7)) - 1;     // bits to *not* flip
    bits[allocd-1] ^= ~mask;
  }
}


void BitArray::selfCheck() const
{
  if (numBits & 7) {
    // there are some trailing bits that I need to check
    unsigned char mask = (1 << (numBits & 7)) - 1;     // bits to *not* check
    unsigned char zero = bits[allocdBytes()-1] & ~mask;
    xassert(zero == 0);
  }
}


void BitArray::unionWith(BitArray const &obj)
{
  xassert(numBits == obj.numBits);

  int allocd = allocdBytes();
  for (int i=0; i<allocd; i++) {
    bits[i] |= obj.bits[i];
  }
}


void BitArray::intersectWith(BitArray const &obj)
{
  xassert(numBits == obj.numBits);

  int allocd = allocdBytes();
  for (int i=0; i<allocd; i++) {
    bits[i] &= obj.bits[i];
  }
}


// it's a little strange to export this function, since it is not
// very general-purpose, but that is the price of encapsulation
bool BitArray::anyEvenOddBitPair() const
{
  int allocd = allocdBytes();
  for (int i=0; i<allocd; i++) {
    unsigned char b = bits[i];
    if (b & (b >> 1) & 0x55) {        // 01010101
      return true;
    }
  }

  return false;    // no such pair
}


BitArray stringToBitArray(char const *src)
{
  int len = strlen(src);
  BitArray ret(len);
  for (int i=0; i<len; i++) {
    if (src[i]=='1') {
      ret.set(i);
    }
  }
  return ret;
}

OldSmbaseString toString(BitArray const &b)
{
  int len = b.length();
  stringBuilder ret(len);
  for (int i=0; i<len; i++) {
    ret[i] = b.test(i)? '1' : '0';
  }
  return ret;
}


// ----------------------- BitArray::Iter ------------------------
void BitArray::Iter::adv()
{
  curBit++;

  while (curBit < arr.numBits) {
    if ((curBit & 7) == 0) {
      // beginning a new byte; is it entirely empty?
      while (arr.bits[curBit >> 3] == 0) {
        // yes, skip to next
        curBit += 8;

        if (curBit >= arr.numBits) {
          return;     // done iterating
        }
      }
    }

    // this could be made a little faster by using the trick to scan
    // for the first nonzero bit.. but since I am only going to scan
    // within a single byte, it shouldn't make that much difference
    if (arr.test(curBit)) {
      return;         // found element
    }

    curBit++;
  }
}


// EOF
