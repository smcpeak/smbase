// bflatten-test.cc
// Tests for bflatten module.

#include "bflatten.h"                  // module under test

#include "flatutil.h"                  // xferEnum
#include "sm-macros.h"                 // EMEMB
#include "sm-test.h"                   // USUAL_MAIN

#include <sstream>                     // std::i/ostringstream
#include <string>                      // std::string


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
}


void SomeData::xfer(Flatten &flat)
{
  flat.xferInt32(x);
  flat.noteOwner(&x);
  s.xfer(flat);
  s2.xfer(flat);
  flat.xferSerf((void*&)px);
  flat.xferInt32(y);
  flat.noteOwner(&y);
  flat.xferSerf((void*&)py);
  flat.xfer_uint64_t(u64);
  flat.xfer_int64_t(i64);
  flat.xfer_uint32_t(u32);
  flat.xfer_int32_t(i32);
  xferEnum(flat, e);
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

  // This does not compare to 'obj', rather it checks a condition that I
  // know 'init' created in 'obj', and should be re-created by
  // deserialization.
  xassert(px == &x);
  xassert(py == &y);
}


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

  printf("bflatten works\n");
}


// EOF
