// test-sm-file-util.cc
// Tests for 'sm-file-util' module.

// Currently these "tests" are quite bad, mostly just printing things
// and relying on me to manually validate them, although I'm slowly
// adding tests with greater diagnostic value.  The main difficulty is
// that some of the behavior is inherently platform-dependent.

#include "sm-file-util.h"              // module to test

// smbase
#include "nonport.h"                   // GetMillisecondsAccumulator
#include "strutil.h"                   // compareStringPtrs
#include "test.h"                      // ARGS_TEST_MAIN, PVAL


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

  ArrayStack<SMFileUtil::DirEntryInfo> entries;
  string wd = sfu.currentDirectory();
  sfu.getSortedDirectoryEntries(entries, wd);
  cout << wd << " has " << entries.length() << " entries:" << endl;
  for (int i=0; i < entries.length(); i++) {
    cout << "  " << entries[i].m_name << ": " << entries[i].m_kind << endl;
  }

  // Repeat with a directory separator appended, expect same results.
  int numEntries = entries.length();
  entries.clear();

  // Add some initial chaff to make sure 'entries' gets cleared.
  entries.push(SMFileUtil::DirEntryInfo("---", SMFileUtil::FK_NONE));

  sfu.getSortedDirectoryEntries(entries, stringb(wd << '/'));
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


static void expectSTDS(SMFileUtil &sfu, char const *dir, char const *expect)
{
  string actual = sfu.stripTrailingDirectorySeparator(dir);
  EXPECT_EQ(actual, string(expect));
}

static void testStripTrailing()
{
  TestSMFileUtil sfu;

  // Unix semantics for things that differ from Windows.
  expectSTDS(sfu, "a\\", "a\\");

  // Things to test both ways.
  for (int i=0; i < 2; i++) {
    expectSTDS(sfu, "", "");
    expectSTDS(sfu, "/", "/");
    expectSTDS(sfu, "a", "a");
    expectSTDS(sfu, "a/", "a");
    expectSTDS(sfu, "aa", "aa");
    expectSTDS(sfu, "/a", "/a");
    expectSTDS(sfu, "/a/", "/a");

    sfu.m_windowsPathSemantics = true;
  }

  // Windows semantics tests.
  expectSTDS(sfu, "a\\", "a");
  expectSTDS(sfu, "c:\\", "c:\\");
  expectSTDS(sfu, "c:", "c:");
  expectSTDS(sfu, "c:\\a", "c:\\a");
  expectSTDS(sfu, "c:\\a\\", "c:\\a");
}


static void expectDE(SMFileUtil &sfu, string const &path, bool expect)
{
  PVAL(path);
  bool actual = sfu.directoryExists(path);
  EXPECT_EQ(actual, expect);
}

static void testDirectoryExists()
{
  SMFileUtil sfu;

  expectDE(sfu, "", false);
  expectDE(sfu, ".", true);
  expectDE(sfu, "..", true);
  expectDE(sfu, "/", true);
  if (sfu.windowsPathSemantics()) {
    expectDE(sfu, "c:/", true);
    expectDE(sfu, "c:/nonexistent-directory", false);
    PVAL(sfu.directoryExists("c:/Windows"));
  }
  else {
    expectDE(sfu, "/tmp", true);
    expectDE(sfu, "/nonexistent-directory", false);
  }
  expectDE(sfu, "fonts", true);
  expectDE(sfu, "fonts/", true);
  expectDE(sfu, "sm-file-util.h", false);
  expectDE(sfu, "nonexist", false);
}


static void testIsReadOnly()
{
  SMFileUtil sfu;
  EXPECT_EQ(sfu.isReadOnly("test-sm-file-util.cc"), false);
  EXPECT_EQ(sfu.isReadOnly("Makefile"), true);
  EXPECT_EQ(sfu.isReadOnly("nonexistent-file"), false);
}


// Defined in sm-file-util.cc.
void getDirectoryEntries_scanThenStat(SMFileUtil &sfu,
  ArrayStack<SMFileUtil::DirEntryInfo> /*OUT*/ &entries, string const &directory);


static void entry(int argc, char **argv)
{
  bool useScanAndProbe = false;
  if (argc >= 3 &&
      (0==strcmp(argv[1], "-probe") ||
       ((useScanAndProbe=true), (0==strcmp(argv[1], "-scan"))))) {
    string directory(argv[2]);

    SMFileUtil sfu;
    ArrayStack<SMFileUtil::DirEntryInfo> entries;

    long elapsed = 0;
    {
      GetMillisecondsAccumulator timer(elapsed);

      // Loop for performance measurement.  Original implementation took
      // 700ms to do 100 iterations probing smbase.  Most time is spent
      // in 'directoryExists'.
      for (int i=0; i<100; i++) {
        if (useScanAndProbe) {
          getDirectoryEntries_scanThenStat(sfu, entries, directory);
        }
        else {
          sfu.getDirectoryEntries(entries, directory);
        }
      }
    }

    for (int i=0; i < entries.length(); i++) {
      cout << entries[i].m_name << ": " << entries[i].m_kind << endl;
    }
    PVAL(elapsed);
    return;
  }

  printSomeStuff();
  testJoinFilename();
  testAbsolutePathExists();
  testTestSMFileUtil();
  testSplitPath();
  testEnsureEndsWith();
  testStripTrailing();
  testDirectoryExists();
  testIsReadOnly();

  cout << "test-sm-file-util ok" << endl;
}

ARGS_TEST_MAIN

// EOF
