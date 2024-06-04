// c-string-reader-test.cc
// Tests for `c-string-reader` module.

#include "c-string-reader.h"           // module under test

#include "exc.h"                       // smbase::XFormat
#include "sm-test.h"                   // EXPECT_EQ, tprintf
#include "reader.h"                    // smbase::ReaderException

using namespace smbase;


static void decodeVector(char const *in, char const *out, int outLen)
{
  tprintf("decodeVector: \"%s\"\n", in);

  std::string expect(out, outLen);
  std::string actual =
    decodeCStringEscapesToString(in, 0 /*delim*/, false /*allowNewlines*/);

  EXPECT_EQ(actual, expect);
}


static void testDecodeEscapes()
{
  decodeVector("\\r\\n", "\r\n", 2);
  decodeVector("abc\\0def", "abc\0def", 7);
  decodeVector("\\033", "\033", 1);
  decodeVector("\\x33", "\x33", 1);
  decodeVector("\\?", "?", 1);

  // Fail due to unescaped delimiter.
  try {
    decodeCStringEscapesToString("\"", '"');
    xfailure("should have failed");
  }
  catch (ReaderException &x) {
    EXPECT_HAS_SUBSTRING(x.getMessage(), "delimiter");
  }

  // Succeed with unescaped delimiter when allowed.
  EXPECT_EQ(decodeCStringEscapesToString("\""),
            "\"");

  // Fail due to unescaped newline.
  try {
    decodeCStringEscapesToString("a\nb");
    xfailure("should have failed");
  }
  catch (ReaderException &x) {
    EXPECT_HAS_SUBSTRING(x.getMessage(), "newline");
  }

  // Succeed with unescaped newline when allowed.
  EXPECT_EQ(decodeCStringEscapesToString("a\nb", 0, true /*allowNewlines*/),
            "a\nb");
}


static void testParseQuotedCString()
{
  EXPECT_EQ(parseQuotedCString("\"\""), "");
  EXPECT_EQ(parseQuotedCString("\"x\""), "x");
  EXPECT_EQ(parseQuotedCString("\"x\\\"\""), "x\"");

  try {
    parseQuotedCString("");
    xfailure("should have failed");
  }
  catch (XFormat &x) {
    // As expected.
  }
}


// Called from unit-tests.cc.
void test_c_string_reader()
{
  testDecodeEscapes();
  testParseQuotedCString();
}


// EOF
