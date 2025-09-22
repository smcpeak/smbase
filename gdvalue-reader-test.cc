// gdvalue-reader-test.cc
// Tests for `gdvalue-reader` module.

#include "smbase/gdvalue-reader.h"     // module under test

#include "smbase/exc.h"                // EXN_CONTEXT_EXPR
#include "smbase/gdvalue.h"            // GDValue
#include "smbase/reader.h"             // XReader
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, TABLESIZE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_FALSE
#include "smbase/xassert.h"            // xassert

#include <optional>                    // std::optional [h]
#include <sstream>                     // std::istringstream
#include <string>                      // std::string

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void expectLoc(
  GDValue const &actual,
  std::optional<std::string> fnameOpt,
  int line,
  int col)
{
  EXN_CONTEXT_EXPR(line);
  EXN_CONTEXT_EXPR(col);

  auto indexOpt = GDValueSourceLocation::fileIndexOfNameOpt(fnameOpt);

  EXPECT_TRUE(actual.hasSourceLocation());
  EXPECT_EQ(actual.sourceLocation(),
            GDValueSourceLocation(indexOpt, line, col));
}


void expectEqLoc(
  GDValue const &actual,
  GDValue const &expect,
  std::optional<std::string> fnameOpt,
  int line,
  int col)
{
  EXPECT_EQ(actual, expect);
  expectLoc(actual, fnameOpt, line, col);
}


void test_basics()
{
  TEST_CASE(__func__);

  std::istringstream iss;
  iss.str("1 2 3");

  std::string const fname("fn");
  GDValueReader reader(iss, fname);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 1, 0));
  expectEqLoc(reader.readNextValue().value(), GDValue(1), fname, 1,1);

  // We stop at the separating space, since seeing that is enough to
  // know that the value "1" is complete.
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 2, 1));

  expectEqLoc(reader.readNextValue().value(), GDValue(2), fname, 1,3);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 4, 3));

  expectEqLoc(reader.readNextValue().value(), GDValue(3), fname, 1,5);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 6, 5));

  EXPECT_FALSE(reader.readNextValue().has_value());
}


void test_error()
{
  TEST_CASE(__func__);

  std::istringstream iss;
  iss.str("1 : 2 3");

  std::string const fname("fn");
  GDValueReader reader(iss, fname);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 1, 0));

  expectEqLoc(reader.readNextValue().value(), GDValue(1), fname, 1,1);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 2, 1));

  EXPECT_EXN_SUBSTR(reader.readNextValue(),
    XReader,
    "fn:1:3: Unexpected ':' while looking for the start of a value.");
}


void test_locations()
{
  GDValue value = fromGDVN(R"(
    [
      null true false foo `a symbol`
      "a string"
      1 23 456 -7
      12.5 -9.0e4 1e3
      [1]
      (1)
      {1}
      {1:2}
      [1:2]
      tag[1]
      tag(1)
      tag{1}
      tag{1:2}
      tag[1:2]
    ]
  )");

  expectLoc(value, {}, 2,5);

  EXPECT_EQ(value.dumpToString(), R"(/*2:5*/[
  /*3:7*/null
  /*3:12*/true
  /*3:17*/false
  /*3:23*/foo
  /*3:27*/`a symbol`
  /*4:7*/"a string"
  /*5:7*/1
  /*5:9*/23
  /*5:12*/456
  /*5:16*/-7
  /*6:7*/12.5
  /*6:12*/-90000.0
  /*6:19*/1000.0
  /*7:7*/[/*7:8*/1]
  /*8:7*/(/*8:8*/1)
  /*9:7*/{/*9:8*/1}
  /*10:7*/{/*10:8*/1:/*10:10*/2}
  /*11:7*/[/*11:8*/1:/*11:10*/2]
  /*12:7*/tag[/*12:11*/1]
  /*13:7*/tag(/*13:11*/1)
  /*14:7*/tag{/*14:11*/1}
  /*15:7*/tag{/*15:11*/1:/*15:13*/2}
  /*16:7*/tag[/*16:11*/1:/*16:13*/2]
]
)");
}


void test_skipWhitespaceAndComments()
{
  TEST_CASE(__func__);

  std::istringstream iss;
  iss.str(
    "1\n"
    "  2\n"
    "3  // comment\n"
    "4  // comment\n"
    "   // comment\n"
    "  6 /*comment*/ 7\n"
  );

  std::string const fname("fn");
  GDValueReader reader(iss, fname);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 1, 0));

  // Expect to find `n` at `line/col/byte`.
  auto testOne = [&](int line, int col, int byteOffset, int n)
  {
    reader.skipWhitespaceAndComments();

    // Use GDVSER here to include the byte offset in a discrepancy
    // printout.
    EXPECT_EQ_GDVSER(reader.getLocation(),
                     FileLineCol(fname, line, col, byteOffset));

    EXPECT_EQ(reader.readNextValue().value(), GDValue(n));
  };

  testOne(1, 1, 0, 1);
  testOne(2, 3, 4, 2);
  testOne(3, 1, 6, 3);
  testOne(4, 1, 20, 4);
  testOne(6, 3, 50, 6);
  testOne(6, 17, 64, 7);

  // Skip to EOF.
  reader.skipWhitespaceAndComments();
  EXPECT_EQ_GDVSER(reader.getLocation(),
                   FileLineCol(fname, 7, 1, 66));

  EXPECT_FALSE(reader.readNextValue().has_value());
}


void test_fileLoc()
{
  EXPECT_EQ(fromGDVN_asIfFile("somefile.gdvn", "[1 2 3]").dumpToString(),
    "/*somefile.gdvn:1:1*/[\n"
    "  /*somefile.gdvn:1:2*/1\n"
    "  /*somefile.gdvn:1:4*/2\n"
    "  /*somefile.gdvn:1:6*/3\n"
    "]\n");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_reader()
{
  test_basics();
  test_error();
  test_locations();
  test_skipWhitespaceAndComments();
  test_fileLoc();
}


// EOF
