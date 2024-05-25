// bflatten-test.cc
// Tests for flatten, flatutil, and bflatten.

#include "bflatten.h"                  // module under test

#include "flatutil.h"                  // xferEnum, xferVectorBytewise
#include "sm-macros.h"                 // EMEMB, OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // USUAL_MAIN

#include <sstream>                     // std::i/ostringstream
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_ANONYMOUS_NAMESPACE


enum SomeEnum {
  SE0,
  SE1,
  SE2
};


// Some data members to de/serialize.
class SomeData {
public:      // data
  int x;
  int y;
  string s;
  string s2;
  int *px;
  int *py;
  uint64_t u64;
  int64_t i64;
  uint32_t u32;
  int32_t i32;
  SomeEnum e;
  std::vector<unsigned char> uc_vec;
  std::vector<int32_t> i32_vec;
  std::vector<int32_t> i32_vec2;

public:      // methods
  void init();
  void xfer(Flatten &flat);
  void checkEqual(SomeData const &obj) const;
};


void SomeData::init()
{
  x = 9;
  y = 22;
  s = "foo bar";

  // Test with a string containing both kinds of line endings, in order
  // to verify that no line ending translation is happening.
  s2 = "one\ntwo\r\n";

  px = &x;
  py = &y;
  u64 = (((uint64_t)0x12345678) << 32) | 0x90ABCDEF;
  i64 = -((int64_t)u64);
  u32 = 0x21436587;
  i32 = -((int32_t)u32);
  e = SE2;

  uc_vec = std::vector<unsigned char>{'h','e','l','l','o'};
  i32_vec = std::vector<int32_t>{1,2,0x12345678,-0x12345678};
  i32_vec2 = i32_vec;
}


void SomeData::xfer(Flatten &flat)
{
  flat.xferInt32(x);
  flat.noteOwner(&x);
  stringXfer(s, flat);
  stringXfer(s2, flat);
  flat.xferSerf((void*&)px);
  flat.xferInt32(y);
  flat.noteOwner(&y);
  flat.xferSerf((void*&)py);
  flat.xfer_uint64_t(u64);
  flat.xfer_int64_t(i64);
  flat.xfer_uint32_t(u32);
  flat.xfer_int32_t(i32);
  xferEnum(flat, e);
  xferVectorBytewise(flat, uc_vec);

  // This would not be a good way to do this for production use, since
  // it would serialize the integers in a way that depends on host
  // endianness, but it will suffice for testing.
  xferVectorBytewise(flat, i32_vec);

  // This is how to do it safely.
  ::xfer(flat, i32_vec2);
}


void SomeData::checkEqual(SomeData const &obj) const
{
  xassert(EMEMB(x));
  xassert(EMEMB(y));
  xassert(EMEMB(s));
  xassert(EMEMB(s2));
  xassert(EMEMB(u64));
  xassert(EMEMB(i64));
  xassert(EMEMB(u32));
  xassert(EMEMB(i32));
  xassert(EMEMB(e));
  xassert(EMEMB(uc_vec));
  xassert(EMEMB(i32_vec));
  xassert(EMEMB(i32_vec2));

  // This does not compare to 'obj', rather it checks a condition that I
  // know 'init' created in 'obj', and should be re-created by
  // deserialization.
  xassert(px == &x);
  xassert(py == &y);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called by unit-tests.cc.
void test_bflatten()
{
  // make up some data
  SomeData d1;
  d1.init();

  // open a file for writing them
  {
    BFlatten flat("bflat.tmp", false /*reading*/);
    d1.xfer(flat);
  }

  // Also save to an in-memory string.
  std::string serializedString;
  {
    std::ostringstream oss;
    StreamFlatten flat(&oss);
    d1.xfer(flat);
    serializedString = oss.str();
  }

  // place to put the data we read
  SomeData d2;

  // read them back
  {
    BFlatten flat("bflat.tmp", true /*reading*/);
    d2.xfer(flat);
  }

  // compare
  d2.checkEqual(d1);

  // delete the temp file
  remove("bflat.tmp");

  // Deserialize the string.
  SomeData d3;
  {
    std::istringstream iss(serializedString);
    StreamFlatten flat(&iss);
    d3.xfer(flat);
  }
  d3.checkEqual(d1);
}


// EOF
