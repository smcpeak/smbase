// test-sm-file-util.cc
// Tests for 'sm-file-util' module.

// Currently these "tests" are quite bad, mostly just printing things
// and relying on me to manually validate them, although I'm slowly
// adding tests with greater diagnostic value.  The main difficulty is
// that some of the behavior is inherently platform-dependent.

#include "sm-file-util.h"              // module to test

#include "test.h"                      // USUAL_MAIN, PVAL


static void printSomeStuff()
{
  SMFileUtil sfu;

  PVAL(sfu.windowsPathSemantics());

  PVAL(sfu.normalizePathSeparators("a/b\\c"));
  PVAL(sfu.normalizePathSeparators("a/b/c/d/e/f/g/h"));
  PVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));
  PVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));
  PVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));

  PVAL(sfu.currentDirectory());

  PVAL(sfu.isDirectorySeparator('x'));
  PVAL(sfu.isDirectorySeparator('/'));
  PVAL(sfu.isDirectorySeparator('\\'));

  PVAL(sfu.isAbsolutePath("/a/b"));
  PVAL(sfu.isAbsolutePath("/"));
  PVAL(sfu.isAbsolutePath("d:/a/b"));
  PVAL(sfu.isAbsolutePath("//server/share/a/b"));
  PVAL(sfu.isAbsolutePath("\\a\\b"));
  PVAL(sfu.isAbsolutePath("a/b"));
  PVAL(sfu.isAbsolutePath("b"));
  PVAL(sfu.isAbsolutePath("."));
  PVAL(sfu.isAbsolutePath("./a"));

  PVAL(sfu.getAbsolutePath("a"));
  PVAL(sfu.getAbsolutePath("/a"));
  PVAL(sfu.getAbsolutePath("d:/a/b"));

  PVAL(sfu.absolutePathExists("d:/wrk/editor"));
  PVAL(sfu.absoluteFileExists("d:/wrk/editor"));
  PVAL(sfu.absolutePathExists("d:/wrk/editor/main.h"));
  PVAL(sfu.absoluteFileExists("d:/wrk/editor/main.h"));
}


static void expectJoin(char const *a, char const *b, char const *expect)
{
  SMFileUtil sfu;
  EXPECT_EQ(sfu.joinFilename(a, b), string(expect));
}


static void testJoinFilename()
{
  expectJoin("", "", "");
  expectJoin("a", "", "a");
  expectJoin("", "b", "b");
  expectJoin("a", "b", "a/b");
  expectJoin("a/", "b", "a/b");
  expectJoin("a", "/b", "a/b");
  expectJoin("a/", "/b", "a/b");
  expectJoin("a", "b/", "a/b/");

  SMFileUtil sfu;
  if (sfu.isDirectorySeparator('\\')) {
    expectJoin("a\\", "/b", "a/b");
  }
  else {
    expectJoin("a\\", "/b", "a\\/b");
  }
}


static void expectRelExists(char const *fname, bool expect)
{
  SMFileUtil sfu;
  string wd = sfu.currentDirectory();
  EXPECT_EQ(sfu.absolutePathExists(sfu.joinFilename(wd, fname)), expect);
}


static void testAbsolutePathExists()
{
  expectRelExists("test-sm-file-util.cc", true);
  expectRelExists("something-else-random.cc", false);

  // Just print these since the result depends on platform.
  SMFileUtil sfu;
  PVAL(sfu.absolutePathExists("c:/"));
  PVAL(sfu.absolutePathExists("c:/Windows"));
  PVAL(sfu.absolutePathExists("c:/something-nonexistent"));
  PVAL(sfu.absolutePathExists("/"));
  PVAL(sfu.absolutePathExists("/home"));
  PVAL(sfu.absolutePathExists("/something-nonexistent"));
}


static void testTestSMFileUtil()
{
  TestSMFileUtil sfu;

  xassert(!sfu.windowsPathSemantics());

  sfu.m_existingPaths.add("/c");
  xassert(sfu.absolutePathExists("/c"));
  xassert(!sfu.absolutePathExists("/d"));
}


static void entry()
{
  printSomeStuff();
  testJoinFilename();
  testAbsolutePathExists();
  testTestSMFileUtil();

  cout << "test-sm-file-util ok" << endl;
}

USUAL_MAIN

// EOF
