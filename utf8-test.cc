// utf8-test.cc
// Tests for utf8-writer and utf8-reader.

// This file is in the public domain.

#include "utf8-writer.h"               // module under test #1

#include "sm-test.h"                   // EXPECT_EQ
#include "vector-utils.h"              // operator<< (std::vector)

#include <sstream>                     // std::ostringstream
#include <vector>                      // std::vector

using namespace smbase;


static void testWriterFixed(std::vector<int> const &input,
                            std::vector<int> const &expect)
{
  std::ostringstream oss;
  UTF8Writer writer(oss);
  for (int c : input) {
    writer.writeCodePoint(c);
  }

  std::string actualChars = oss.str();

  std::vector<int> actual;
  for (char c : actualChars) {
    actual.push_back((unsigned char)c);
  }

  EXPECT_EQ(actual, expect);
}


static void testWriterFixedOneChar(int c, std::vector<int> const &expect)
{
  testWriterFixed({c}, expect);
}


static void testWriterFixed()
{
  testWriterFixedOneChar(0x00, {0x00});
  testWriterFixedOneChar(0x7F, {0x7F});

  testWriterFixedOneChar(0x080, {0xC2, 0x80});
  testWriterFixedOneChar(0x0BF, {0xC2, 0xBF});
  testWriterFixedOneChar(0x0C0, {0xC3, 0x80});
  testWriterFixedOneChar(0x100, {0xC4, 0x80});
  testWriterFixedOneChar(0x7FF, {0xDF, 0xBF});

  testWriterFixedOneChar(0x0800, {0xE0, 0xA0, 0x80});
  testWriterFixedOneChar(0x083F, {0xE0, 0xA0, 0xBF});
  testWriterFixedOneChar(0x0840, {0xE0, 0xA1, 0x80});
  testWriterFixedOneChar(0x0880, {0xE0, 0xA2, 0x80});
  testWriterFixedOneChar(0x0FC0, {0xE0, 0xBF, 0x80});
  testWriterFixedOneChar(0x1000, {0xE1, 0x80, 0x80});
  testWriterFixedOneChar(0xFFFF, {0xEF, 0xBF, 0xBF});

  testWriterFixedOneChar(0x010000, {0xF0, 0x90, 0x80, 0x80});
  testWriterFixedOneChar(0x01003F, {0xF0, 0x90, 0x80, 0xBF});
  testWriterFixedOneChar(0x010040, {0xF0, 0x90, 0x81, 0x80});
  testWriterFixedOneChar(0x011000, {0xF0, 0x91, 0x80, 0x80});
  testWriterFixedOneChar(0x10FFFF, {0xF4, 0x8F, 0xBF, 0xBF});

  // One of each length.
  testWriterFixed({0x20, 0x0C0,      0x0FC0,           0x011000},
                  {0x20, 0xC3, 0x80, 0xE0, 0xBF, 0x80, 0xF0, 0x91, 0x80, 0x80});

  // Examples from RFC 3629.
  testWriterFixed({0x0041, 0x2262, 0x0391, 0x002E},
                  {0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E});

  testWriterFixed({0xD55C, 0xAD6D, 0xC5B4},
                  {0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4});

  testWriterFixed({0x65E5, 0x672C, 0x8A9E},
                  {0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E});

  testWriterFixed({0xFEFF, 0x233B4},
                  {0xEF, 0xBB, 0xBF, 0xF0, 0xA3, 0x8E, 0xB4});
}


// Called from unit-tests.cc.
void test_utf8()
{
  testWriterFixed();
}


// EOF
