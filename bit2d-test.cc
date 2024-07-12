// bit2d-test.cc
// Tests for bit2d.

#include "bit2d.h"                     // module under test

#include "bflatten.h"                  // BFlatten
#include "point.h"                     // point
#include "sm-test.h"                   // verbose
#include "xassert.h"                   // xassert


// Called from unit-tests.cc.
void test_bit2d()
{
  Bit2d bits(point(17,3));
  xassert(bits.okpt(point(16,2)) &&
         !bits.okpt(point(17,3)) &&
         !bits.okpt(point(2,16)));

  bits.setall(0);
  xassert(!bits.testAndSet(point(9,1)));
  xassert(bits.testAndSet(point(9,1)));

  xassert(!bits.testAndSet(point(2,0)));
  xassert(bits.testAndSet(point(2,0)));

  xassert(!bits.testAndSet(point(16,2)));
  xassert(bits.testAndSet(point(16,2)));

  bits.toggle(point(3,2));
  xassert(bits.get(point(3,2)));

  if (verbose) {
    bits.print();
  }

  // test read/write
  {
    Bit2d *another = writeThenRead(bits);
    xassert(*another == bits);
    delete another;
  }

  // test set8 and get8
  xassert(bits.get8(point(8,0)) == 0);
  xassert(bits.get8(point(0,0)) == 0x04);  // 00000100
  xassert(bits.get8(point(0,2)) == 0x08);  // 00001000, bit 3 from right is set

  xassert(bits.get8(point(16,0)) == 0);
  bits.setall(1);
  xassert(bits.get8(point(16,0)) == 0x01); // 00000001, pad bits cleared

  bits.set8(point(16,0), 0xFE);            // all high bits ignored; LSB is 0
  xassert(bits.get8(point(16,0)) == 0x00);

                                           // 76543210
  bits.set8(point(0,0), 0x6C);             // 01101100
  xassert(bits.get(point(0,0)) == 0);
  xassert(bits.get(point(1,0)) == 0);
  xassert(bits.get(point(2,0)) == 1);
  xassert(bits.get(point(3,0)) == 1);
  xassert(bits.get(point(4,0)) == 0);
  xassert(bits.get(point(5,0)) == 1);
  xassert(bits.get(point(6,0)) == 1);
  xassert(bits.get(point(7,0)) == 0);

  for (int w=1; w <= 8; w++) {
    Bit2d bits(point(w,1));

    bits.set8(point(0,0), 0);
    xassert(bits.get8(point(0,0)) == 0);

    bits.set8(point(0,0), 0xFF);
    xassert(bits.get8(point(0,0)) == ((1 << w) - 1));
  }

  // test byteBitSwapLsbMsb (exhaustively)
  for (int i=0; i < 256; i++) {
    unsigned char input = i;

    // naively bit swap it
    unsigned char output = 0;
    for (int bit = 0; bit < 8; bit++) {
      if (input & (1 << bit)) {
        output |= (1 << (7 - bit));
      }
    }

    // compare to the faster function
    unsigned char actual = byteBitSwapLsbMsb(input);
    xassert(actual == output);
  }

  // one concrete vector to make sure the above test is not
  // totally borked
  xassert(byteBitSwapLsbMsb(0xC7) == 0xE3);
}


// EOF
