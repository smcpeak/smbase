// srcloc-test.cc
// Tests for srcloc.h

#include "srcloc.h"                    // module under test

#include "autofile.h"                  // AutoFILE
#include "exc.h"                       // xbase
#include "strtokp.h"                   // StrtokParse
#include "trace.h"                     // traceProgress

#include <stdlib.h>                    // rand, exit, system, getenv


namespace {


SourceLocManager mgr;
int longestLen=0;

// given a location, decode it into line/col and then re-encode,
// and check that the new encoding matches the old
void testRoundTrip(SourceLoc loc)
{
  char const *fname;
  int line, col;
  mgr.decodeLineCol(loc, fname, line, col);

  if (col > longestLen) {
    longestLen = col;
  }

  SourceLoc loc2 = mgr.encodeLineCol(fname, line, col);

  xassert(loc == loc2);
}


// location in SourceLoc and line/col
class BiLoc {
public:
  int line, col;
  SourceLoc loc;
};


// given a file, compute SourceLocs throughout it and verify
// that round-trip encoding works
void testFile(char const *fname)
{
  // dsw: I want a way to make sure that we never look for a file.
  xassert(sourceLocManager->mayOpenFiles);

  // find the file's length
  int len;
  {
    AutoFILE fp(fname, "rb");

    fseek(fp, 0, SEEK_END);
    len = (int)ftell(fp);
    cout << "length of " << fname << ": " << len << endl;
  }

  // get locations for the start and end
  SourceLoc start = mgr.encodeOffset(fname, 0);
  SourceLoc end = mgr.encodeOffset(fname, len);

  // check expectations for start
  xassert(mgr.getLine(start) == 1);
  xassert(mgr.getCol(start) == 1);

  // test them
  testRoundTrip(start);
  testRoundTrip(end);

  // temporary
  //testRoundTrip((SourceLoc)11649);

  BiLoc *bi = new BiLoc[len+1];
  char const *dummy;

  // test all positions, forward sequential; also build the
  // map for the random test; note that 'len' is considered
  // a valid source location even though it corresponds to
  // the char just beyond the end
  int i;
  for (i=0; i<=len; i++) {
    SourceLoc loc = mgr.encodeOffset(fname, i);
    testRoundTrip(loc);

    bi[i].loc = loc;
    mgr.decodeLineCol(loc, dummy, bi[i].line, bi[i].col);
  }

  // backward sequential
  for (i=len; i>0; i--) {
    SourceLoc loc = mgr.encodeOffset(fname, i);
    testRoundTrip(loc);
  }

  // random access, both mapping directions
  for (i=0; i<=len; i++) {
    int j = rand()%(len+1);
    int dir = rand()%2;

    if (dir==0) {
      // test loc -> line/col map
      int line, col;
      mgr.decodeLineCol(bi[j].loc, dummy, line, col);
      xassert(line == bi[j].line);
      xassert(col == bi[j].col);
    }
    else {
      // test line/col -> loc map
      SourceLoc loc = mgr.encodeLineCol(fname, bi[j].line, bi[j].col);
      xassert(loc == bi[j].loc);
    }
  }

  delete[] bi;
}


// write a file with the given contents, and call 'testFile' on it
void testFileString(char const *contents)
{
  {
    AutoFILE fp("srcloc.tmp", "w");
    int written = fwrite(contents, 1, strlen(contents), fp);
    xassert(written == (int)strlen(contents));
  }

  testFile("srcloc.tmp");

  // since I keep using "srcloc.tmp" over and over, I need to reset
  // the manager between attempts since otherwise it thinks it already
  // knows the line lengths
  mgr.reset();
}


#if 0      // call site is commented
// decode with given expectation, complain if it doesn't match
void expect(SourceLoc loc, char const *expFname, int expLine, int expCol)
{
  char const *fname;
  int line, col;
  mgr.decodeLineCol(loc, fname, line, col);

  if (0!=strcmp(fname, expFname) ||
      line != expLine ||
      col != expCol) {
    printf("expected %s:%d:%d, but got %s:%d:%d\n",
           expFname, expLine, expCol,
           fname, line, col);
    exit(2);
  }
}
#endif // 0


// should this be exported?
OldSmbaseString locString(char const *fname, int line, int col)
{
  return stringc << fname << ":" << line << ":" << col;
}


void buildHashMap(SourceLocManager::File *pp, char const *fname, int &expanderLine)
{
  expanderLine = 0;

  // dsw: I want a way to make sure that we never look for a file.
  xassert(sourceLocManager->mayOpenFiles);

  AutoFILE fp(fname, "rb");

  enum { SZ=256 };
  char buf[SZ];
  int ppLine=0;
  while (fgets(buf, SZ, fp)) {
    if (buf[strlen(buf)-1] == '\n') {
      ppLine++;
    }

    if (0==memcmp(buf, "int blah_de_blah", 16)) {
      expanderLine = ppLine;
    }

    if (buf[0]!='#') continue;

    // break into tokens at whitespace (this isn't exactly
    // right, because the file names can have quoted spaces,
    // but it will do for testing purposes)
    StrtokParse tok(buf, " \r\n");
    if (tok < 3) continue;

    int origLine = atoi(tok[1]);
    char const *tok2 = tok[2];
    OldSmbaseString origFname = substring(tok2+1, strlen(tok2)-2);  // remove quotes
    pp->addHashLine(ppLine, origLine, origFname.c_str());
  }
  pp->doneAdding();
}


void testHashMap()
{
  // run the preprocessor
  if (0!=system("cpp -DTEST_SRCLOC srcloc.test.cc >srcloc.tmp")) {
    xbase("failed to preprocess srcloc.test.cc; the command that failed was:\n"
          "  cpp -DTEST_SRCLOC srcloc.test.cc >srcloc.tmp");
  }

  SourceLocManager::File *pp = mgr.getInternalFile("srcloc.tmp");
  SourceLocManager::File *orig = mgr.getInternalFile("srcloc.test.cc");

  // read srcloc.tmp and install the hash maps
  int expanderLine=0;
  buildHashMap(pp, "srcloc.tmp", expanderLine);

  // the 2nd line in the pp source should correspond to the
  // first line in the orig src
  // update: this doesn't work with all preprocessors, and I'm
  // confident in the implementation now, so I'll turn this off
  //SourceLoc lineTwo = mgr.encodeLineCol("srcloc.tmp", 2, 1);
  //expect(lineTwo, "srcloc.cc", 1,1);

  // print decodes of first several lines (including those that
  // are technically undefined because they occur on #line lines)
  int ppLine;
  for (ppLine = 1; ppLine < 10; ppLine++) {
    SourceLoc loc = mgr.encodeLineCol("srcloc.tmp", ppLine, 1);
    cout << "ppLine " << ppLine << ": " << toString(loc) << endl;
  }

  // similar for last few lines
  for (ppLine = pp->numLines - 4; ppLine <= pp->numLines; ppLine++) {
    SourceLoc loc = mgr.encodeLineCol("srcloc.tmp", ppLine, 1);
    cout << "ppLine " << ppLine << ": " << toString(loc) << endl;
  }

  // see how the expander line behaves
  if (!expanderLine) {
    cout << "didn't find expander line!\n";
    exit(2);
  }
  else {
    SourceLoc loc = mgr.encodeLineCol("srcloc.tmp", expanderLine, 1);
    cout << "expander column 1: " << toString(loc) << endl;

    // in the pp file, I can advance the expander horizontally a long ways;
    // this should truncate to column 9
    loc = advCol(loc, 20);

    char const *fname;
    int offset;
    mgr.decodeOffset(loc, fname, offset);
    cout << "expander column 21: " << fname << ", offset " << offset << endl;
    xassert(0==strcmp(fname, "srcloc.test.cc"));

    // map that to line/col, which should show the truncation
    int line, col;
    orig->charToLineCol(offset, line, col);
    cout << "expander column 21: " << locString(fname, line, col) << endl;
    if (col != 9 && col != 10) {
      // 9 is for LF line endings, 10 for CRLF
      cout << "expected column 9 or 10!\n";
      exit(2);
    }
  }
}


void testHashMap2()
{
  SourceLocManager::File *pp = mgr.getInternalFile("srcloc.test2.cc");

  int expanderLine=0;
  buildHashMap(pp, "srcloc.test2.cc", expanderLine);

  for (int ppLine = 1; ppLine <= pp->numLines; ppLine++) {
    SourceLoc loc = mgr.encodeLineCol("srcloc.test2.cc", ppLine, 1);
    cout << "ppLine " << ppLine << ": " << toString(loc) << endl;
  }
}


} // anonymous namespace


// Called from unit-tests.cc.
void test_srcloc()
{
  xBase::logExceptions = false;
  traceAddSys("progress");
  traceProgress() << "begin" << endl;

  if (getenv("TEST_SRCLOC_MAX_STATIC_LOCS")) {
    // set maxStaticLocs low to test the warning
    mgr.maxStaticLocs = 1;
  }

  // test with some special files
  testFileString("first\nsecond\nthird\n");      // ordinary
  testFileString("first\nsecond\nthird no nl");  // no final newline
  testFileString("");                            // empty
  testFileString("x");                           // one char
  testFileString("\n");                          // one newline

  // test my source code
  testFile("srcloc.cc");
  testFile("srcloc.h");

  // do it again, so at least one won't be the just-added file;
  // in fact do it many times so I can see results in a profiler
  for (int i=0; i<1; i++) {
    testFile("srcloc.cc");
    testFile("srcloc.h");
  }

  traceProgress() << "end" << endl;

  // protect against degeneracy by printing the length of
  // the longest line
  cout << "\n";
  cout << "long line len: " << longestLen << endl;

  // test the statics
  cout << "invalid: " << toString(SL_UNKNOWN) << endl;
  cout << "here: " << toString(HERE_SOURCELOC) << endl;

  cout << "\n";
  testHashMap();
  testHashMap2();

  cout << "srcloc is ok\n";
}


// EOF
