// bit2d.h            see license.txt for copyright and terms of use
// Two-dimensional array of bits.

#ifndef SMBASE_BIT2D_H
#define SMBASE_BIT2D_H

#include "point.h"           // point

class Flatten;

class Bit2d {
private:     // data
  unsigned char *data;  // bits; [0..stride-1] is first row, etc.
  bool owning;          // when false, 'data' is not owned by this object
  point size;           // size.x is # of cols, size.y is # of rows
  int stride;           // bytes between starts of adjacent rows;
                        // computable from size.x but stored for quick access

private:     // funcs
  unsigned char *byteptr(point const &p)               { return data + p.y * stride + (p.x>>3); }
  unsigned char const *byteptrc(point const &p) const  { return data + p.y * stride + (p.x>>3); }

  // this is the number of bytes allocated in 'data'
  int datasize() const                        { return size.y * stride; }

public:      // funcs
  // NOTE: does *not* clear the bitmap!  use 'setall' to do that
  Bit2d(point const &aSize);
  Bit2d(Bit2d const &obj);

  Bit2d& operator= (Bit2d const &obj);     // sizes must be equal already
  ~Bit2d();

  Bit2d(Flatten&);
  void xfer(Flatten &flat);

  bool okpt(point const &p) const    { return p.gtez() && p < size; }
  point const &Size() const          { return size; }

  bool operator== (Bit2d const &obj) const;     // compare sizes and data

  // bit access (these were inline earlier, but they expand to a huge amount
  // of code (more than 100 bytes), so I've un-inlined them)
  int get(point const &p) const;
  void set(point const &p);     // to 1
  void reset(point const &p);   // to 0
  void setto(point const &p, int val);
  void toggle(point const &p);

  // set the bit, but return what it was previously
  int testAndSet(point const &p);

  // set everything
  void setall(int val);

  // Set 8 bits at a time.  p.x must be a multiple of 8.  The
  // least significant bit is at 'p', the next significant bit
  // is at 'p+(1,0)', etc.
  //
  // If 'Size().x - p.x' is less than 8, the high bits of 'val'
  // are discarded.
  void set8(point const &p, unsigned char val);

  // Retrieve 8 bits at a time.  Same restrictions and interpretation
  // as with 'set8'.
  //
  // If 'Size().x - p.x' is less than 8, the high bits of 'val' are
  // zero.
  unsigned char get8(point const &p) const;

  // debugging
  void print() const;

  // bit of a hack: I want to be able to save the data as code which,
  // when compiled, will build a bit2d from static data.. for this
  // I need access to some private fields and a special ctor
  Bit2d(unsigned char * /*serf*/ data, point const &size, int stride);
  unsigned char *private_data() { return data; }
  unsigned char const *private_dataC() const { return data; }
  int private_datasize() const { return datasize(); }
  int private_stride() const { return stride; }
};


// Swap 8 bits around so that the least significant bit becomes the
// most significant, and vice-versa, and so on for the other bits.
// For example, 01101101 becomes 10110110.  This function is its own
// inverse.
unsigned char byteBitSwapLsbMsb(unsigned char b);


#endif // SMBASE_BIT2D_H
