// sm-file-util-test.cc
// Tests for 'sm-file-util' module.

// Currently these "tests" are quite bad, mostly just printing things
// and relying on me to manually validate them, although I'm slowly
// adding tests with greater diagnostic value.  The main difficulty is
// that some of the behavior is inherently platform-dependent.

#include "sm-file-util.h"              // module to test

// smbase
#include "nonport.h"                   // GetMillisecondsAccumulator, getFileModificationTime
#include "run-process.h"               // RunProcess
#include "sm-test.h"                   // PVAL
#include "strutil.h"                   // compareStringPtrs
#include "syserr.h"                    // XSysError


static bool verbose = false;

#define DIAG(stuff)        \
  if (verbose) {           \
    cout << stuff << endl; \
  }

#define VPVAL(stuff)                                        \
  if (verbose) {                                            \
    PVAL(stuff);                                            \
  }                                                         \
  else {                                                    \
    /* Evaluate it to ensure no crash, but do not print. */ \
    (void)stuff;                                            \
  }



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

  if (fn.hasPathComponents()) {
    // We can only freely toggle the trailing slash if there are path
    // components.
    xassert(fn.withTrailingSlash(!fn.hasTrailingSlash()) != fn);
  }
}


// Check that 'sfu' reports the same properties as 'fn' on 'input'.
static void checkAgainstSFU(
  SMFileUtil &sfu,
  string const &input,
  SMFileName const &fn)
{
  EXPECT_EQ(sfu.isAbsolutePath(input), fn.isAbsolute());
  EXPECT_EQ(sfu.endsWithDirectorySeparator(input),
            fn.endsWithPathSeparator());
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

  // Test against SMFileUtil too.
  TestSMFileUtil sfu;
  sfu.m_windowsPathSemantics = false;
  checkAgainstSFU(sfu, input, fn);
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

  // Test against SMFileUtil too.
  TestSMFileUtil sfu;
  sfu.m_windowsPathSemantics = true;
  checkAgainstSFU(sfu, input, fn);
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

  // Test against SMFileUtil too.
  SMFileUtil sfu;
  checkAgainstSFU(sfu, input, fn);
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

  VPVAL(sfu.windowsPathSemantics());

  VPVAL(sfu.normalizePathSeparators("a/b\\c"));
  VPVAL(sfu.normalizePathSeparators("a/b/c/d/e/f/g/h"));
  VPVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));
  VPVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));
  VPVAL(sfu.normalizePathSeparators(sfu.getAbsolutePath("a/b/c/d/e/f/g/h")));

  VPVAL(sfu.currentDirectory());

  VPVAL(sfu.isDirectorySeparator('x'));
  VPVAL(sfu.isDirectorySeparator('/'));
  VPVAL(sfu.isDirectorySeparator('\\'));

  VPVAL(sfu.isAbsolutePath("/a/b"));
  VPVAL(sfu.isAbsolutePath("/"));
  VPVAL(sfu.isAbsolutePath("d:/a/b"));
  VPVAL(sfu.isAbsolutePath("//server/share/a/b"));
  VPVAL(sfu.isAbsolutePath("\\a\\b"));
  VPVAL(sfu.isAbsolutePath("a/b"));
  VPVAL(sfu.isAbsolutePath("b"));
  VPVAL(sfu.isAbsolutePath("."));
  VPVAL(sfu.isAbsolutePath("./a"));

  VPVAL(sfu.getAbsolutePath("a"));
  VPVAL(sfu.getAbsolutePath("/a"));
  VPVAL(sfu.getAbsolutePath("d:/a/b"));

  VPVAL(sfu.absolutePathExists("d:/wrk/editor"));
  VPVAL(sfu.absoluteFileExists("d:/wrk/editor"));
  VPVAL(sfu.absolutePathExists("d:/wrk/editor/main.h"));
  VPVAL(sfu.absoluteFileExists("d:/wrk/editor/main.h"));
}


static void testGetSortedDirectoryEntries()
{
  SMFileUtil sfu;

  ArrayStack<SMFileUtil::DirEntryInfo> entries1;
  string wd = sfu.currentDirectory();
  sfu.getSortedDirectoryEntries(entries1, wd);
  DIAG(wd << " has " << entries1.length() << " entries");

  // Disable printing the entries to cut down on the noise.
  if (false) {
    for (int i=0; i < entries1.length(); i++) {
      DIAG("  " << entries1[i].asString());
    }
  }

  // Repeat with a directory separator appended, expect same results.
  {
    ArrayStack<SMFileUtil::DirEntryInfo> entries2;

    // Add some initial junk to check that 'entries2' gets cleared
    // by 'getSortedDirectoryEntries'.
    entries2.push(SMFileUtil::DirEntryInfo("---", SMFileUtil::FK_NONE));

    sfu.getSortedDirectoryEntries(entries2, stringb(wd << '/'));

    // This failed once, seemingly randomly.  I couldn't reproduce it.
    // So I've added more diagnostics in case it happens again.
    if (entries1.length() != entries2.length()) {
      DIAG("Listing results changed based on adding '/'!");
      VPVAL(entries1.length());
      VPVAL(entries2.length());

      int i1 = 0;
      int i2 = 0;
      while (i1 < entries1.length() && i2 < entries2.length()) {
        SMFileUtil::DirEntryInfo const &e1 = entries1[i1];
        SMFileUtil::DirEntryInfo const &e2 = entries2[i2];

        int cmp = e1.compareTo(e2);
        if (cmp < 0) {
          DIAG(  "only in entries1: " << e1.asString());
          ++i1;
        }
        else if (cmp > 0) {
          DIAG(  "only in entries2: " << e2.asString());
          ++i2;
        }
        else {
          ++i1;
          ++i2;
        }
      }
      while (i1 < entries1.length()) {
        SMFileUtil::DirEntryInfo const &e1 = entries1[i1];
        DIAG(  "only in entries1: " << e1.asString());
        ++i1;
      }
      while (i2 < entries2.length()) {
        SMFileUtil::DirEntryInfo const &e2 = entries2[i2];
        DIAG(  "only in entries2: " << e2.asString());
        ++i2;
      }

      xfailure("directory lists are not equal");
    }
  }
}


static void testGetDirectoryEntries()
{
  SMFileUtil sfu;
  ArrayStack<SMFileUtil::DirEntryInfo> entries;

  try {
    DIAG("Should throw:");
    sfu.getDirectoryEntries(entries, "nonexist-dir");
    xfailure("nonexist-dir exists?!");
  }
  catch (XBase &x) {
    DIAG("Attempting to read nonexist-dir: " << x.why());
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


static void expectJoinIRF(char const *a, char const *b, char const *expect)
{
  SMFileUtil sfu;
  EXPECT_EQ(sfu.joinIfRelativeFilename(a, b), string(expect));
}


static void testJoinIfRelativeFilename()
{
  expectJoinIRF("", "", "");
  expectJoinIRF("a", "", "a");
  expectJoinIRF("", "b", "b");
  expectJoinIRF("a", "b", "a/b");
  expectJoinIRF("a/", "b", "a/b");
  expectJoinIRF("a", "/b", "/b");      // keep absolute suffix
  expectJoinIRF("a/", "/b", "/b");     // keep absolute suffix
  expectJoinIRF("a", "b/", "a/b/");

  SMFileUtil sfu;
  if (sfu.isDirectorySeparator('\\')) {
    expectJoinIRF("a", "\\b", "\\b");
  }
  else {
    expectJoinIRF("a", "\\b", "a/\\b");
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
  expectRelExists("sm-file-util-test.cc", true);
  expectRelExists("something-else-random.cc", false);

  // Just print these since the result depends on platform.
  SMFileUtil sfu;
  VPVAL(sfu.absolutePathExists("c:/"));
  VPVAL(sfu.absolutePathExists("c:/Windows"));
  VPVAL(sfu.absolutePathExists("c:/something-nonexistent"));
  VPVAL(sfu.absolutePathExists("/"));
  VPVAL(sfu.absolutePathExists("/home"));
  VPVAL(sfu.absolutePathExists("/something-nonexistent"));
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
  VPVAL(inputPath);

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
  VPVAL(path);
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
    VPVAL(sfu.directoryExists("c:/Windows"));
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
  DIAG("expectGFK: " << fname);
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
    DIAG("atomicallyRenameFile refused to move directory, as expected:\n"
         << x.why());
  }
}


// Run 'rm -rf path'.
static void rm_rf(char const *path)
{
  RunProcess::check_run(std::vector<string>{"rm", "-rf", path});
}


static void testCreateDirectoryAndParents()
{
  SMFileUtil sfu;

  // Start by clearing the test directory.
  rm_rf("tmpdir");

  // Make directories.
  sfu.createDirectoryAndParents("tmpdir/a/b/c/");
  xassert(sfu.directoryExists("tmpdir/a/b/c"));

  // Remove some of them.
  rm_rf("tmpdir/a/b");
  xassert(sfu.directoryExists("tmpdir/a"));
  xassert(!sfu.directoryExists("tmpdir/a/b"));

  // Re-make some.
  sfu.createDirectoryAndParents("tmpdir/a/b/c");
  xassert(sfu.directoryExists("tmpdir/a/b/c"));

  // Remove some again.
  rm_rf("tmpdir/a/b");

  // Make 'b' as a regular file.
  RunProcess::check_run(std::vector<string>{"touch", "tmpdir/a/b"});

  // Now try to create.
  try {
    sfu.createDirectoryAndParents("tmpdir/a/b/c");
    xfailure("that should have failed");
  }
  catch (XSysError &x) {
    xassert(x.reason == XSysError::R_ALREADY_EXISTS);
  }

  // Clean up.
  rm_rf("tmpdir");
}


static void testReadAndWriteFile()
{
  // All bytes.
  std::vector<unsigned char> bytes;
  for (int i=0; i < 256; i++) {
    bytes.push_back((unsigned char)i);
  }

  SMFileUtil sfu;
  string fname = "test.dir/allbytes.bin";
  sfu.writeFile(fname, bytes);

  std::vector<unsigned char> bytes2(sfu.readFile(fname));

  xassert(bytes2 == bytes);
}


static void testTouchFile()
{
  SMFileUtil sfu;

  string fname = "test.dir/tmp";

  // Make sure the file is initially absent.
  if (sfu.pathExists(fname)) {
    sfu.removeFile(fname);
  }
  xassert(!sfu.pathExists(fname));

  // Touch it to create it as empty.
  sfu.touchFile(fname);
  xassert(sfu.pathExists(fname));

  int64_t ts1;
  xassert(getFileModificationTime(fname.c_str(), ts1 /*OUT*/));

  // Touch the empty file.
  DIAG("testTouchFile: sleep 1 ...");
  portableSleep(1);
  sfu.touchFile(fname);
  int64_t ts2;
  xassert(getFileModificationTime(fname.c_str(), ts2 /*OUT*/));
  xassert(ts2 > ts1);

  // Write it with a byte.
  sfu.writeFile(fname, std::vector<unsigned char>{'x'});
  int64_t ts3;
  xassert(getFileModificationTime(fname.c_str(), ts3 /*OUT*/));

  // Touch that.
  DIAG("testTouchFile: sleep 1 ...");
  portableSleep(1);
  sfu.touchFile(fname);
  int64_t ts4;
  xassert(getFileModificationTime(fname.c_str(), ts4 /*OUT*/));
  xassert(ts4 > ts3);

  // Clean up.
  sfu.removeFile(fname);
}


// Defined in sm-file-util.cc.
void getDirectoryEntries_scanThenStat(SMFileUtil &sfu,
  ArrayStack<SMFileUtil::DirEntryInfo> /*OUT*/ &entries, string const &directory);


// Check that we can manipulate arrays of DirEntryInfo properly.  This
// previously caused a crash due to a bug in ArrayStack::sort.
static void testArrayOfDirEntry()
{
  for (int j=0; j < 10; ++j) {
    ArrayStack<SMFileUtil::DirEntryInfo> entries;
    for (int i=0; i < 1000; ++i) {
      entries.push(SMFileUtil::DirEntryInfo(stringb(i), SMFileUtil::FK_REGULAR));
    }

    entries.sort(&SMFileUtil::DirEntryInfo::compare);
  }
}


// Called from unit-tests.cc.
void test_sm_file_util()
{
  bool useProbe = !!std::getenv("SM_FILE_UTIL_TEST_PROBE");

  if (char const *scanDir = std::getenv("SM_FILE_UTIL_TEST_SCAN")) {
    VPVAL(scanDir);
    VPVAL(useProbe);

    string directory(scanDir);

    SMFileUtil sfu;
    ArrayStack<SMFileUtil::DirEntryInfo> entries;

    long elapsed = 0;
    {
      GetMillisecondsAccumulator timer(elapsed);

      // Loop for performance measurement.  Original implementation took
      // 700ms to do 100 iterations probing smbase.  Most time is spent
      // in 'directoryExists'.
      for (int i=0; i<100; i++) {
        if (useProbe) {
          getDirectoryEntries_scanThenStat(sfu, entries, directory);
        }
        else {
          sfu.getDirectoryEntries(entries, directory);
        }
      }
    }

    for (int i=0; i < entries.length(); i++) {
      DIAG(entries[i].m_name << ": " << entries[i].m_kind);
    }
    VPVAL(elapsed);
    return;
  }

  testFileName();
  printSomeStuff();
  testGetSortedDirectoryEntries();
  testGetDirectoryEntries();
  testJoinFilename();
  testJoinIfRelativeFilename();
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
  testCreateDirectoryAndParents();
  testReadAndWriteFile();
  testArrayOfDirEntry();

  // This test is annoyingly slow, so it is disabled by default.
  if (getenv("SM_FILE_UTIL_TEST_TOUCH")) {
    testTouchFile();
  }
}


// EOF
