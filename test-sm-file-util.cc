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


// Run some checks on the 'fn' object directly.
static void checkFNObject(SMFileName const &fn, SMFileName::Syntax syntax)
{
  // Round trip through string should produce an equal object.
  {
    string path = fn.toString(syntax);
    SMFileName fn2(path, syntax);
    xassert(fn == fn2);
  }

  // Make variants with different components to test operator==.
  xassert(fn.withFileSystem(stringb(fn.getFileSystem() << 'x')) != fn);
  xassert(fn.withIsAbsolute(!fn.isAbsolute()) != fn);
  ArrayStack<string> comps2;
  fn.getPathComponents(comps2);
  comps2.push("x");
  xassert(fn.withPathComponents(comps2) != fn);
  xassert(fn.withTrailingSlash(!fn.hasTrailingSlash()) != fn);
}


// Test file name parsing with S_POSIX.
static void expectFNp(
  string const &input,
  bool expectIsAbsolute,
  string const &expectPathComponents,
  bool expectTrailingSlash)
{
  SMFileName fn(input, SMFileName::S_POSIX);
  EXPECT_EQ(fn.getFileSystem(), "");
  EXPECT_EQ(fn.isAbsolute(), expectIsAbsolute);
  EXPECT_EQ(fn.getPathComponentsString(), expectPathComponents);
  EXPECT_EQ(fn.hasTrailingSlash(), expectTrailingSlash);

  checkFNObject(fn, SMFileName::S_POSIX);
}


// Test file name parsing with S_WINDOWS.
static void expectFNw(
  string const &input,
  string const &expectFileSystem,
  bool expectIsAbsolute,
  string const &expectPathComponents,
  bool expectTrailingSlash)
{
  SMFileName fn(input, SMFileName::S_WINDOWS);
  EXPECT_EQ(fn.getFileSystem(), expectFileSystem);
  EXPECT_EQ(fn.isAbsolute(), expectIsAbsolute);
  EXPECT_EQ(fn.getPathComponentsString(), expectPathComponents);
  EXPECT_EQ(fn.hasTrailingSlash(), expectTrailingSlash);

  checkFNObject(fn, SMFileName::S_WINDOWS);
}


// Test file name parsing with S_NATIVE.
static void expectFNn(
  string const &input,
  string const &expectFileSystem,
  bool expectIsAbsolute,
  string const &expectPathComponents,
  bool expectTrailingSlash)
{
  SMFileName fn(input);     // means S_NATIVE
  EXPECT_EQ(fn.getFileSystem(), expectFileSystem);
  EXPECT_EQ(fn.isAbsolute(), expectIsAbsolute);
  EXPECT_EQ(fn.getPathComponentsString(), expectPathComponents);
  EXPECT_EQ(fn.hasTrailingSlash(), expectTrailingSlash);

  checkFNObject(fn, SMFileName::S_NATIVE);
}


// Test with both, expecting the same result.
static void expectFNsame(
  string const &input,
  bool expectIsAbsolute,
  string const &expectPathComponents,
  bool expectTrailingSlash)
{
  expectFNp(input, expectIsAbsolute, expectPathComponents, expectTrailingSlash);
  expectFNw(input, "", expectIsAbsolute, expectPathComponents, expectTrailingSlash);
}


// Test with both, expecting different results.
static void expectFNpw(
  string const &input,
  bool expectPosixIsAbsolute,
  string const &expectPosixPathComponents,
  bool expectPosixTrailingSlash,
  string const &expectWindowsFileSystem,
  bool expectWindowsIsAbsolute,
  string const &expectWindowsPathComponents,
  bool expectWindowsTrailingSlash)
{
  expectFNp(input, expectPosixIsAbsolute, expectPosixPathComponents,
    expectPosixTrailingSlash);
  expectFNw(input, expectWindowsFileSystem, expectWindowsIsAbsolute,
    expectWindowsPathComponents, expectWindowsTrailingSlash);
}


static void testFileName()
{
  expectFNsame("", false, "", false);
  expectFNsame("/", true, "", false);
  expectFNpw("\\",
    false, "\\", false,
    "", true, "", false);
  expectFNsame(".", false, ".", false);
  expectFNpw("//",
    true, "", false,
    "/", true, "", false);
  expectFNsame("a", false, "a", false);
  expectFNsame("a/b", false, "a/b", false);
  expectFNpw("a\\b",
    false, "a\\b", false,
    "", false, "a/b", false);
  expectFNsame("a//b", false, "a/b", false);
  expectFNpw("a/\\b",
    false, "a/\\b", false,
    "", false, "a/b", false);
  expectFNpw("a\\/b",
    false, "a\\/b", false,
    "", false, "a/b", false);
  expectFNsame("a/", false, "a", true);
  expectFNpw("a\\",
    false, "a\\", false,
    "", false, "a", true);
  expectFNsame("/./", true, ".", true);
  expectFNsame("ab/cd", false, "ab/cd", false);
  expectFNsame("x///", false, "x", true);
  expectFNsame("..", false, "..", false);
  expectFNpw("c:",
    false, "c:", false,
    "c:", false, "", false);
  expectFNsame("cc:", false, "cc:", false);
  expectFNpw("c:a",
    false, "c:a", false,
    "c:", false, "a", false);
  expectFNpw("c:.",
    false, "c:.", false,
    "c:", false, ".", false);
  expectFNpw("c:a/b",
    false, "c:a/b", false,
    "c:", false, "a/b", false);
  expectFNpw("C:/",
    false, "C:", true,
    "C:", true, "", false);
  expectFNpw("C://",
    false, "C:", true,
    "C:", true, "", false);
  expectFNpw("C:/windows",
    false, "C:/windows", false,
    "C:", true, "windows", false);
  expectFNpw("C:/windows/system",
    false, "C:/windows/system", false,
    "C:", true, "windows/system", false);
  expectFNpw("C:/program files",
    false, "C:/program files", false,
    "C:", true, "program files", false);
  expectFNpw("//server/share",
    true, "server/share", false,
    "/", true, "server/share", false);
  expectFNpw("\\\\server\\share",
    false, "\\\\server\\share", false,
    "/", true, "server/share", false);
  expectFNpw("//server",
    true, "server", false,
    "/", true, "server", false);
  expectFNpw("///server/share",
    true, "server/share", false,
    "/", true, "server/share", false);

  xassert(SMFileName::isPathSeparator('/', SMFileName::S_POSIX));
  xassert(SMFileName::isPathSeparator('/', SMFileName::S_WINDOWS));
  xassert(SMFileName::isPathSeparator('/', SMFileName::S_NATIVE));

  xassert(!SMFileName::isPathSeparator('\\', SMFileName::S_POSIX));
  xassert(SMFileName::isPathSeparator('\\', SMFileName::S_WINDOWS));

  xassert(!SMFileName::isPathSeparator('x', SMFileName::S_POSIX));
  xassert(!SMFileName::isPathSeparator('x', SMFileName::S_WINDOWS));
  xassert(!SMFileName::isPathSeparator('x', SMFileName::S_NATIVE));

  if (SMFileName::isWindowsSyntax(SMFileName::S_NATIVE)) {
    expectFNn("\\", "", true, "", false);
    xassert(SMFileName::isPathSeparator('\\', SMFileName::S_NATIVE));
  }
  else {
    expectFNn("\\", "", false, "\\", false);
    xassert(!SMFileName::isPathSeparator('\\', SMFileName::S_NATIVE));
  }
}


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
  EXPECT_EQ(sfu.isReadOnly("test.dir/read-only.txt"), true);
  EXPECT_EQ(sfu.isReadOnly("nonexistent-file"), false);
}


static void expectCD(SMFileUtil &sfu, string const &input, string const &expect)
{
  string actual = sfu.collapseDots(input);
  EXPECT_EQ(actual, expect);
}

static void testCollapseDots()
{
  // normalize path separators

  SMFileUtil sfu;
  expectCD(sfu, "", "");
  expectCD(sfu, "a", "a");
  expectCD(sfu, "/", "/");
  expectCD(sfu, "c:/", "c:/");
  expectCD(sfu, ".", ".");
  expectCD(sfu, "./", "./");
  expectCD(sfu, "\\", "/");
  expectCD(sfu, "a/.", "a");
  expectCD(sfu, "a/..", ".");
  expectCD(sfu, "a/../", "./");
  expectCD(sfu, "a/b/..", "a");
  expectCD(sfu, "a/./b", "a/b");
  expectCD(sfu, "a/../b", "b");
  expectCD(sfu, "a/b/../c", "a/c");
  expectCD(sfu, "a/./b/../c", "a/c");
  expectCD(sfu, "a/b/../..", ".");
  expectCD(sfu, "a/b/c/../..", "a");
  expectCD(sfu, "a/b/c/../../d", "a/d");
  expectCD(sfu, "a/b/c/../d/../e", "a/b/e");
  expectCD(sfu, "././././", "./");
  expectCD(sfu, "..", "..");
  expectCD(sfu, "../..", "../..");
  expectCD(sfu, "../../a", "../../a");
  expectCD(sfu, ".././../a/b/../c", "../../a/c");
  expectCD(sfu, ".././../a/b/../../c", "../../c");
  expectCD(sfu, "./../..", "../..");
  expectCD(sfu, ".././..", "../..");
  expectCD(sfu, "../../.", "../..");
}


static void expectGFK(SMFileUtil &sfu,
                      string fname, SMFileUtil::FileKind expect)
{
  cout << "expectGFK: " << fname << endl;
  SMFileUtil::FileKind actual = sfu.getFileKind(fname);
  EXPECT_EQ(actual, expect);
}

static void testGetFileKind()
{
  SMFileUtil sfu;

  // Ordinary.
  expectGFK(sfu, "sm-file-util.cc", SMFileUtil::FK_REGULAR);
  EXPECT_EQ(sfu.pathExists("sm-file-util.cc"), true);

  // Directory.
  expectGFK(sfu, "test", SMFileUtil::FK_DIRECTORY);
  expectGFK(sfu, "test/", SMFileUtil::FK_DIRECTORY);

  // Non-existent.
  expectGFK(sfu, "nonexist", SMFileUtil::FK_NONE);
  expectGFK(sfu, "nonexist/", SMFileUtil::FK_NONE);
  EXPECT_EQ(sfu.pathExists("nonexist"), false);

  // Specfically test with a path composed of an existing file name with
  // a slash appended, since that seems to provoke ENOTDIR from 'stat'.
  expectGFK(sfu, "sm-file-util.cc/", SMFileUtil::FK_NONE);
}


static void testAtomicallyRenameFile()
{
  string content("test content\n");
  string srcFname("tarf.src.tmp");
  string destFname("tarf.dest.tmp");

  writeStringToFile(content, srcFname);
  writeStringToFile("other content\n", destFname);

  // Overwrite 'destFname'.
  SMFileUtil sfu;
  sfu.atomicallyRenameFile(srcFname, destFname);

  // Check that the new content arrived.
  string actual = readStringFromFile(destFname);
  EXPECT_EQ(actual, content);

  // Clean up 'destFname'.
  sfu.removeFile(destFname);

  // Check that both files are gone.
  expectGFK(sfu, srcFname, SMFileUtil::FK_NONE);
  expectGFK(sfu, destFname, SMFileUtil::FK_NONE);


  // Verify that the function refuses to operate on directories.
  try {
    sfu.atomicallyRenameFile("fonts", "fonts");
    xfailure("that should have failed!");
  }
  catch (XFatal &x) {
    cout << "atomicallyRenameFile refused to move directory, as expected:\n"
         << x.why() << endl;
  }
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

  testFileName();
  printSomeStuff();
  testJoinFilename();
  testAbsolutePathExists();
  testTestSMFileUtil();
  testSplitPath();
  testEnsureEndsWith();
  testStripTrailing();
  testDirectoryExists();
  testIsReadOnly();
  testCollapseDots();
  testGetFileKind();
  testAtomicallyRenameFile();

  cout << "test-sm-file-util ok" << endl;
}

ARGS_TEST_MAIN

// EOF
