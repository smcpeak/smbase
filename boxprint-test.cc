// boxprint-test.cc
// Tests for boxprint.

#include "boxprint.h"                  // module under test

#include "sm-test.h"                   // DIAG

#include <stdlib.h>                    // atoi, getenv


static bool verbose = false;


// Called from unit-tests.cc.
void test_boxprint()
{
  verbose = !!getenv("VERBOSE");

  BoxPrint bp;

  bp << "int foo()" << bp.br
     << "{" << bp.ind;

  bp << bp.lineStart
     << "// wazoo"
     << bp.fbr;

  bp << "printf(" << bp.seq
        << "\"hello there %d!\\n\"," << bp.br
        << "123456789"
     << bp.end << ");" << bp.br;

  bp << "bar(" << bp.seq
        << "1" << bp.op("+")
        << "2" << bp.op("+")
        << "3" << bp.op("+")
        << "4" << bp.op("+")
        << "5" << bp.op("+")
        << "6" << bp.op("+")
        << "7" << bp.op("+")
        << "8" << bp.op("+")
        << "9" << bp.op("+")
        << "10"
     << bp.end << ");" << bp.br;

  bp << "baz(" << bp.seq
        << "\"a really long line that has no optional breaks at all\""
     << bp.end << ");" << bp.br;

  bp << "zoo(" << bp.seq
        << "\"one break is here, but it is very\"," << bp.br
        << "\"far from the start\""
     << bp.end << ");" << bp.br;

  bp << "assert(" << bp.seq
        << bp.seq << "x" << bp.op("=") << "y" << bp.end << bp.op("&&")
        << bp.seq << "z" << bp.op("=") << "w" << bp.end << bp.op("&&")
        << "(" << bp.seq
           << bp.seq << "moron" << bp.op("!=") << "fool" << bp.end << bp.op("||")
           << "taxes->theRich"
        << bp.end << ")"
     << bp.end << ")" << bp.br;

  bp << bp.hv
        << "forall(" << bp.seq
           << "x," << bp.br << "y," << bp.br << "z"
        << bp.end << "). if {" << bp.ind
        << bp.seq << "x" << bp.op("==") << "yooey_more" << bp.end << ";" << bp.br
        << bp.seq << "yowza" << bp.op("!=") << "fooey" << bp.end << ";" << bp.br
        << bp.und << "} /*==>*/ {" << bp.ind
        << bp.seq << "z(x,y,z)" << bp.op("==") << "3" << bp.end << ";" << bp.br
        << "ay_caramba" << ";" << bp.br
        << bp.und << "};"
     << bp.end << bp.br;

  // here is a 'forall' with a comment surrounded by forced breaks
  bp << bp.hv
        << bp.lineStart
        << "// forced break comment"
        << bp.fbr
        << "forall(" << bp.seq
           << "x," << bp.br << "y," << bp.br << "z"
        << bp.end << "). if {" << bp.ind
        << bp.seq << "x" << bp.op("==") << "yooey_more" << bp.end << ";" << bp.br
        << bp.seq << "yowza" << bp.op("!=") << "fooey" << bp.end << ";" << bp.br
        << bp.und << "} /*==>*/ {" << bp.ind
        << bp.seq << "z(x,y,z)" << bp.op("==") << "3" << bp.end << ";" << bp.br
        << "ay_caramba" << ";" << bp.br
        << bp.und << "};"
     << bp.end << bp.br;

  // here is a 'forall' with a comment surrounded by forced breaks
  bp << bp.hv
        << bp.lineStart
        << "// forced break comment"
        << bp.fbr
        << "// character: " << 'c' << bp.fbr
        << "forall(" << bp.seq
           << "x," << bp.br << "y," << bp.br << "z"
        << bp.end << "). if {" << bp.ind
        << bp.seq << "x" << bp.op("==") << "yooey_more" << bp.end << ";" << bp.br
        << bp.seq << "yowza" << bp.op("!=") << "fooey" << bp.end << ";" << bp.br
        << bp.und << "} /*==>*/ {" << bp.ind
        << bp.seq << "z(x,y,z)" << bp.op("==") << "3" << bp.end << ";" << bp.br
        << "ay_caramba" << ";" << bp.br
        << bp.und << "};"
     << bp.end << bp.br;

  bp << bp.und << "}" << bp.br;

  bp << bp.fbr;
  bp << bp.vert
        << "int main()" << bp.br
        << "{" << bp.ind
        <<   "return 0;" << bp.br
        << bp.und << "}" << bp.br
     << bp.end;

  BPBox *tree = bp.takeTree();

  BPRender ren;
  ren.margin = 30;
  if (char const *marginStr = getenv("BOXPRINT_TEST_MARGIN")) {
    ren.margin = atoi(marginStr);
  }
  DIAG("margin: " << ren.margin);

  tree->render(ren);
  delete tree;

  DIAG("         1    1    2    2    3    3    4    4    5    5    6    6    7");
  DIAG("1---5----0----5----0----5----0----5----0----5----0----5----0----5----0");
  if (verbose) {
    // Print without an additional newline.
    cout << ren.takeString();
  }
}


// EOF
