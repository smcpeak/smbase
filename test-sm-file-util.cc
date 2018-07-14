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

  ArrayStack<string> entries;
  string wd = sfu.currentDirectory();
  sfu.getDirectoryEntries(entries, wd);
  cout << wd << " has " << entries.length() << " entries:" << endl;
  for (int i=0; i < entries.length(); i++) {
    cout << "  " << entries[i] << endl;
  }

  // Repeat with a directory separator appended, expect same results.
  int numEntries = entries.length();
  entries.clear();
  entries.push("---");     // Make sure 'entries' gets cleared.
  sfu.getDirectoryEntries(entries, stringb(wd << '/'));
  xassert(numEntries == entries.length());

  try {
    cout << "Should throw:" << endl;
    sfu.getDirectoryEntries(entries, "nonexist-dir");
    cout << "nonexist-dir exists?!" << endl;
  }
  catch (xBase &x) {
    cout << "Attempting to read nonexist-dir: " << x.why() << endl;
  }
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


static void expectSplit(SMFileUtil &sfu,
  char const *expectDir,
  char const *expectBase,
  char const *inputPath)
{
  PVAL(inputPath);

  // Make sure 'splitPath' changes these.
  string actualDir = "---";
  string actualBase = "---";

  sfu.splitPath(actualDir, actualBase, inputPath);

  EXPECT_EQ(actualDir, string(expectDir));
  EXPECT_EQ(actualBase, string(expectBase));

  EXPECT_EQ(sfu.splitPathDir(inputPath), string(expectDir));
  EXPECT_EQ(sfu.splitPathBase(inputPath), string(expectBase));
}


static void testSplitPath()
{
  TestSMFileUtil sfu;

  expectSplit(sfu, "", "", "");
  expectSplit(sfu, "", "a", "a");
  expectSplit(sfu, "/", "a", "/a");
  expectSplit(sfu, "a/", "b", "a/b");
  expectSplit(sfu, "/a/", "b", "/a/b");
  expectSplit(sfu, "a/", "", "a/");
  expectSplit(sfu, "/a/", "", "/a/");
  expectSplit(sfu, "/a/b/", "", "/a/b/");
  expectSplit(sfu, "/", "", "/");
}


static void expectEEWDS(SMFileUtil &sfu, char const *dir, char const *expect)
{
  string actual = sfu.ensureEndsWithDirectorySeparator(dir);
  EXPECT_EQ(actual, string(expect));
}


static void testEnsureEndsWith()
{
  TestSMFileUtil sfu;

  expectEEWDS(sfu, "", "/");
  expectEEWDS(sfu, "/", "/");
  expectEEWDS(sfu, "\\", "\\/");
  expectEEWDS(sfu, "a", "a/");
  expectEEWDS(sfu, "a/", "a/");
  expectEEWDS(sfu, "a\\", "a\\/");

  // At least for now, I do not remove extra separators.
  expectEEWDS(sfu, "a//", "a//");

  sfu.m_windowsPathSemantics = true;

  expectEEWDS(sfu, "", "/");
  expectEEWDS(sfu, "/", "/");
  expectEEWDS(sfu, "\\", "\\");
  expectEEWDS(sfu, "a", "a/");
  expectEEWDS(sfu, "a/", "a/");
  expectEEWDS(sfu, "a\\", "a\\");
  expectEEWDS(sfu, "a//", "a//");
  expectEEWDS(sfu, "a\\\\", "a\\\\");
}


static void entry()
{
  printSomeStuff();
  testJoinFilename();
  testAbsolutePathExists();
  testTestSMFileUtil();
  testSplitPath();
  testEnsureEndsWith();

  cout << "test-sm-file-util ok" << endl;
}

USUAL_MAIN

// EOF
