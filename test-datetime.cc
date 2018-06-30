// test-datetime.cc
// Tests for 'datetime' module.

#include "datetime.h"                  // module to test

#include "sm-iostream.h"               // cout, endl
#include "test.h"                      // PVAL
#include "xassert.h"                   // xassert


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


void entry()
{
  testFromUnixTime();

  DateTimeSeconds d;
  cout << "Default-constructed: " << d << endl;

  d.fromUnixTime(1456746400, -8*60);
  xassert(d.toString() == "2016-02-29 03:46:40 -08:00");

  d.fromUnixTime(1456746400, +8*60);
  xassert(d.toString() == "2016-02-29 19:46:40 +08:00");

  d.month = 13;
  try {
    cout << "Expecting an exception here:" << endl;
    d.validateFields();
    xfailure("that should have failed!");
  }
  catch (xFormat &x) {
    // As expected.
  }

  cout << "Now: " << localTimeString() << endl;

  cout << "test-datetime PASSED\n";
}

USUAL_MAIN


// EOF
