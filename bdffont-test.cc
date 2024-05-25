// bdffont-test.cc
// Tests for bdffont.

#include "bdffont.h"                   // module under test

#include "exc.h"                       // xfatal
#include "strutil.h"                   // readStringFromFile

#include <iostream>                    // std::{cout, endl}

#include <unistd.h>                    // unlink

using std::cout;
using std::endl;


// Called from unit-tests.cc.
void test_bdffont()
{
  // parse a file
  //
  // Amusingly, the actual sample input in the spec is missing a
  // bitmap line for the "quoteright" character!  I have repaired it
  // in my version of the input.
  //
  // I've made some other changes as well to test some syntax
  // variations and another anomalies.
  BDFFont font;
  parseBDFFile(font, "fonts/sample1.bdf");

  // write it out
  writeBDFFile("tmp.bdf", font);

  // The output should match sample1out.bdf, which is the same as sample1
  // except that "j" comes after "quoteright" and METRICSSET is
  // explicit.
  if (readStringFromFile("fonts/sample1out.bdf") != readStringFromFile("tmp.bdf")) {
    xfatal("fonts/sample1out.bdf and tmp.bdf differ!");
  }

  (void)unlink("tmp.bdf");

  if (char const *otherTest = getenv("BDFFONT_OTHERTEST")) {
    cout << "testing " << otherTest << endl;

    BDFFont otherFont;
    parseBDFFile(otherFont, otherTest);
    writeBDFFile("tmp.bdf", otherFont);
  }
}


// EOF
