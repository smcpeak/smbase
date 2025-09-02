// gdvalue-reader-test.cc
// Tests for `gdvalue-reader` module.

#include "smbase/gdvalue-reader.h"     // module under test

#include "smbase/gdvalue.h"            // GDValue
#include "smbase/reader.h"             // XReader
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_FALSE

#include <optional>                    // std::optional [h]
#include <sstream>                     // std::istringstream
#include <string>                      // std::string

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  TEST_CASE(__func__);

  std::istringstream iss;
  iss.str("1 2 3");

  std::string const fname("fn");
  GDValueReader reader(iss, fname);
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 1, 0));

  EXPECT_EQ(reader.readNextValue().value(), GDValue(1));

  // We stop at the separating space, since seeing that is enough to
  // know that the value "1" is complete.
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 2, 1));

  EXPECT_EQ(reader.readNextValue().value(), GDValue(2));
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 4, 3));

  EXPECT_EQ(reader.readNextValue().value(), GDValue(3));
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

  EXPECT_EQ(reader.readNextValue().value(), GDValue(1));
  EXPECT_EQ(reader.getLocation(), FileLineCol(fname, 1, 2, 1));

  EXPECT_EXN_SUBSTR(reader.readNextValue(),
    XReader,
    "fn:1:3: Unexpected ':' while looking for the start of a value.");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_reader()
{
  test_basics();
  test_error();
}


// EOF
