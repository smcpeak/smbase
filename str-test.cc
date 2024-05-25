// str-test.cc
// Tests for str.

#include "str.h"                       // module under test

#include "sm-iostream.h"               // cout
#include "sm-test.h"                   // EXPECT_EQ

#include <iomanip>                     // std::hex



static void test(unsigned long val, char const * NULLABLE expect)
{
  std::string actual = stringb(val << " in hex: " << std::hex << val);

  if (expect) {
    EXPECT_EQ(actual, expect);
  }
}


// Called from unit-tests.cc.
void test_str()
{
  // for the moment I just want to test the hex formatting
  test(64, "64 in hex: 40");
  test(0xFFFFFFFF, "4294967295 in hex: ffffffff");
  test(0, "0 in hex: 0");
  test(1, "1 in hex: 1");

  // Don't check the output on this one because it is dependent on the
  // platform.
  test((unsigned long)(-1), nullptr);

  {
    std::string actual = stringb((void*)&test_str);

    // Make sure there are some hex digits in there.
    EXPECT_MATCHES_REGEX(actual, "[0-9A-Fa-f]{4}");
  }

  {
    std::string actual = (stringc << "hi " << 3);
    EXPECT_EQ(actual, "hi 3");
  }
}


// EOF
