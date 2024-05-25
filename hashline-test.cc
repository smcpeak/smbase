// hashline-test.cc
// Tests for hashline.

#include "hashline.h"                  // module under test

#include <stdio.h>                     // printf
#include <stdlib.h>                    // exit


static void query(HashLineMap &hl, int ppLine,
                  int expectOrigLine, char const *expectOrigFname)
{
  int origLine;
  char const *origFname;
  hl.map(ppLine, origLine, origFname);

  if (origLine != expectOrigLine ||
      0!=strcmp(origFname, expectOrigFname)) {
    printf("map(%d) yielded %s:%d, but I expected %s:%d\n",
           ppLine, origFname, origLine,
           expectOrigFname, expectOrigLine);
    exit(2);
  }
}


// Called from unit-tests.cc.
void test_hashline()
{
  // insert #line directives:
  //    foo.i
  //    +----------
  //   1|// nothing; it's in the pp file
  //   2|#line 1 foo.cc
  //   3|
  //   4|
  //   5|#line 1 foo.h
  //   ..
  //  76|#line 5 foo.cc
  //   ..
  // 100|#line 101 foo.i

  HashLineMap hl("foo.i");
  hl.addHashLine(2, 1, "foo.cc");
  hl.addHashLine(5, 1, "foo.h");
  hl.addHashLine(76, 5, "foo.cc");
  hl.addHashLine(100, 101, "foo.i");
  hl.doneAdding();

  // make queries, and check for expected results
  query(hl, 1, 1, "foo.i");

  query(hl, 3, 1, "foo.cc");
  query(hl, 4, 2, "foo.cc");

  query(hl, 6, 1, "foo.h");
  query(hl, 7, 2, "foo.h");
  // ...
  query(hl, 75, 70, "foo.h");

  query(hl, 77, 5, "foo.cc");
  query(hl, 78, 6, "foo.cc");
  // ...
  query(hl, 99, 27, "foo.cc");

  query(hl, 101, 101, "foo.i");
  query(hl, 102, 102, "foo.i");
  // ...

  // printf("unique filenames: %d\n", hl.numUniqueFilenames());
}


// EOF
