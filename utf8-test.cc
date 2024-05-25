// utf8-test.cc
// Tests for utf8-writer and utf8-reader.

// This file is in the public domain.

#include "utf8-reader.h"               // module under test #1
#include "utf8-writer.h"               // module under test #2

#include "sm-test.h"                   // EXPECT_EQ
#include "vector-utils.h"              // operator<< (std::vector)

#include <sstream>                     // std::{istringstream, ostringstream}
#include <vector>                      // std::vector

using namespace smbase;


// Decode `input` octets into a code point vector.
static std::vector<int> decodeVector(std::vector<int> const &input)
{
  std::vector<char> inputBytes;
  for (int c : input) {
    inputBytes.push_back((char)(unsigned char)c);
  }

  // Set up a reader for `inputBytes`.
  std::string inputBytesString(inputBytes.begin(), inputBytes.end());
  std::istringstream iss(inputBytesString);
  UTF8Reader reader(iss);

  std::vector<int> decoded;
  while (true) {
    int cp = reader.readCodePoint();
    if (cp < 0) {
      break;
    }
    decoded.push_back(cp);
  }

  return decoded;
}


// Test that the reader decodes `input` octets as `expect` code points.
static void testReaderFixed(std::vector<int> const &input,
                            std::vector<int> const &expect)
{
  std::vector<int> actual = decodeVector(input);

  EXPECT_EQ(actual, expect);
}


// Encode `input` code points as an octet value vector.
static std::vector<int> encodeVector(std::vector<int> const &input)
{
  std::ostringstream oss;
  UTF8Writer writer(oss);
  for (int c : input) {
    writer.writeCodePoint(c);
  }

  std::string chars = oss.str();

  std::vector<int> octets;
  for (char c : chars) {
    octets.push_back((unsigned char)c);
  }

  return octets;
}


// Test that the writer encodes `input` code points as `expect` octets,
// then swap their roles and call `testReaderFixed`.
static void testWriterFixed(std::vector<int> const &input,
                            std::vector<int> const &expect)
{
  std::vector<int> actual = encodeVector(input);

  EXPECT_EQ(actual, expect);

  // Deliberately swap input/expect in this call since we are reversing
  // the encoding direction.
  testReaderFixed(expect, input);
}


static void testWriterFixedOneChar(int c, std::vector<int> const &expect)
{
  testWriterFixed({c}, expect);
}


// Test the writer with some fixed inputs.
//
// This also tests the reader because of what `testWriterFixed` does.
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


static void testAllPointsRoundtrip()
{
  // I've run this test incrementing by 1, but for normal use that is
  // a bit too slow, so we go in semi-arbitrary jumps by default.
  int increment = 55;
  if (char const *incStr = std::getenv("UTF8_TEST_RT_INC")) {
    increment = std::atoi(incStr);
    std::cout << "Using increment of " << increment << ".\n";
  }

  for (int i=0; i <= 0x10FFFF; i += increment) {
    if (0xD800 <= i && i <= 0xDFFF) {
      // Skip surrogate pair region.
    }
    else {
      std::vector<int> expect{i};
      std::vector<int> actual(decodeVector(encodeVector(expect)));

      EXPECT_EQ(actual, expect);
    }
  }
}


static void testOneError(
  std::vector<int> encoding,
  UTF8ReaderException::Kind kind,
  char const *regex)
{
  try {
    decodeVector(encoding);
    xfailure("that should have failed");
  }
  catch (UTF8ReaderException &e) {
    EXPECT_EQ(e.m_kind, kind);
    EXPECT_MATCHES_REGEX(e.why(), regex);
  }
}


static void testErrors()
{
  testOneError({0xC2}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");

  testOneError({0xE0}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");
  testOneError({0xEF, 0x80}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");

  testOneError({0xF0}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");
  testOneError({0xF1, 0x90}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");
  testOneError({0xF4, 0x90, 0x80}, UTF8ReaderException::K_TRUNCATED_STREAM,
    "stops in the middle");

  testOneError({0xEF, 0xC0}, UTF8ReaderException::K_INVALID_CONTINUATION,
    "byte 0xC0 is");
  testOneError({0xF4, 0x90, 0x80, 0xCF}, UTF8ReaderException::K_INVALID_CONTINUATION,
    "byte 0xCF is");

  // I do not currently prohibit *encoding* surrogate pair values ...
  testOneError(encodeVector({0xD800}), UTF8ReaderException::K_SURROGATE_PAIR,
    "is U.D800,");
  testOneError(encodeVector({0xDFFF}), UTF8ReaderException::K_SURROGATE_PAIR,
    "is U.DFFF,");

  testOneError({0xF5}, UTF8ReaderException::K_BYTE_TOO_LARGE,
    "0xF5 is too large");
}


// Called from unit-tests.cc.
void test_utf8()
{
  testWriterFixed();
  testAllPointsRoundtrip();
  testErrors();
}


// EOF
