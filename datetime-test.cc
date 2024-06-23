// datetime-test.cc
// Tests for 'datetime' module.

#include "datetime.h"                  // module to test

#include "exc.h"                       // smbase::XFormat
#include "sm-iostream.h"               // cout, endl
#include "sm-test.h"                   // PVAL, DIAG, verbose
#include "xassert.h"                   // xassert

using namespace smbase;


static struct UnixTimeAndISO {
  UnixTime unixTime;
  char const *iso;
} const unixTimeTestcases[] = {
  {      44567464 * (UnixTime)100,
                     "2111-03-25 17:06:40" },
  {      1530315832, "2018-06-29 23:43:52" },
  {      1456746400, "2016-02-29 11:46:40" },
  {      1456646400, "2016-02-28 08:00:00" },
  {      1030315832, "2002-08-25 22:50:32" },
  {               0, "1970-01-01 00:00:00" },
  {      2147483647, "2038-01-19 03:14:07" },
  { -2147483647 - 1, "1901-12-13 20:45:52" },
};

static void testFromUnixTime()
{
  for (int i=0; i < TABLESIZE(unixTimeTestcases); i++) {
    UnixTime utInput = unixTimeTestcases[i].unixTime;

    DateTimeSeconds d;
    d.fromUnixTime(utInput, 0 /*tzOffset*/);
    string expect = stringb(unixTimeTestcases[i].iso << " +00:00");
    string actual = d.toString();
    if (!( actual == expect )) {
      PVAL(utInput);
      PVAL(expect);
      PVAL(actual);
    }
    xassert(actual == expect);

    d.validateFields();

    UnixTime utOutput = d.toUnixTime();
    if (utOutput != utInput) {
      PVAL(utInput);
      PVAL(utOutput);
    }
    xassert(utOutput == utInput);
  }
}


// Called from unit-test.cc.
void test_datetime()
{
  testFromUnixTime();

  DateTimeSeconds d;
  DIAG("Default-constructed: " << d);

  d.fromCurrentTime();
  DIAG("Now: " << d);
  DIAG("  dateTimeString: " << d.dateTimeString());
  DIAG("  dateString    : " << d.dateString());
  DIAG("  timeString    : " << d.timeString());
  DIAG("  zoneString    : " << d.zoneString());

  FixedDateTimeProvider fdtp(1000000000 + 83*60, -(1*60 + 23));
  d.fromCurrentTime(&fdtp);
  xassert(d.toString() == "2001-09-09 01:46:40 -01:23");

  d.fromUnixTime(1456746400, -8*60);
  xassert(d.toString() == "2016-02-29 03:46:40 -08:00");

  d.fromUnixTime(1456746400, +8*60);
  xassert(d.toString() == "2016-02-29 19:46:40 +08:00");

  d.fromUnixTime(1456746400 - 30*60, -(7*60 + 30));
  xassert(d.toString() == "2016-02-29 03:46:40 -07:30");

  d.month = 13;
  try {
    DIAG("Expecting an exception here:");
    d.validateFields();
    xfailure("that should have failed!");
  }
  catch (XFormat &x) {
    // As expected.
  }

  DIAG("localTimeString: " << localTimeString());
}


// EOF
