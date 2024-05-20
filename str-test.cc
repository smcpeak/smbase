// str-test.cc
// Tests for str.

#include "str.h"                       // module under test

#include "sm-iostream.h"               // cout
#include "strutil.h"                   // stringf

#include <iomanip>                     // std::hex


static void test(unsigned long val)
{
  //cout << stringb(val << " in hex: 0x" << stringBuilder::Hex(val)) << endl;

  cout << stringb(val << " in hex: " << std::hex << val) << endl;
}


// Called from unit-tests.cc.
void test_str()
{
  // for the moment I just want to test the hex formatting
  test(64);
  test(0xFFFFFFFF);
  test(0);
  test((unsigned long)(-1));
  test(1);

  cout << "stringf: " << stringf("int=%d hex=%X str=%s char=%c float=%f",
                                 3, 0xAA, "hi", 'f', 3.4) << endl;

  cout << "ptr: " << stringb((void*)&test_str) << endl;

  cout << "tests passed\n";
}


// EOF
