// pprint-test.cc
// Tests for pprint.

#include "pprint.h"                    // module under test

#include <iostream>                    // std::cout

using std::cout;


static PPrintToString pp;


// Called by unit-tests.cc.
void test_pprint()
{
  pp.margin = 30;
  pp.startText = "; ";

  cout << "         1    1    2    2    3\n";
  cout << "1---5----0----5----0----5----0\n";

  pp << "int foo()\n"
        "{\n"
        ;
  pp.ind(+2);
  pp << "printf(\b\"hello there %d!\\n\",\r123456789\f);\n";
  pp << "bar(\b1 +\r2 +\r3 +\r4 +\r5 +\r6 +\r7 +\r8 +\r9 +\r10\f);\n";
  pp << "baz(\b\"a really long line that has no optional breaks at all\"\f);\n";
  pp << "zoo(\b\"one break is here, but it is very\",\r\"far from the start\"\f);\n";
  pp << "assert(\bx ==\ry &&\rz ==\rw &&\r"
               "(\bmoron !=\rfool ||\rtaxes->theRich\f)\f);\n";
  pp << "\aforall(x, y, z). if {\r"
          "x == yooey_more;\r"
          "yowza != fooey;\f\r"
        "} {\a\r"
          "z(x,y,z)==3;\r"
          "ay_caramba;\f\r"
        "}\n";
  pp.ind(-2);
  pp << "}\n";

  cout << pp.sb;
}


// EOF
