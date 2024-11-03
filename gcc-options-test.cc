// gcc-options-test.cc
// Tests for gcc-options module.

#include "gcc-options.h"               // module under test

#include "exc.h"                       // smbase::XBase
#include "sm-test.h"                   // EXPECT_EQ
#include "string-util.h"               // toString(std::vector)
#include "vector-util.h"               // vecAccumulateWith[Map]
#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout

using namespace smbase;


#define OM(name) GCCOptions::OM_##name


static void testEmpty()
{
  GCCOptions gccOptions;
  xassert(gccOptions.empty());
}


static void checkEqual(
  std::vector<GCCOptions::Option> const &actual,
  std::vector<GCCOptions::Option> const &expect)
{
  std::string sep(" ");

  if (actual != expect) {
    auto ts = [](GCCOptions::Option const &opt) { return opt.toString(); };
    std::cout << "expect: " << vecAccumulateWithMap(expect, ts, sep) << '\n';
    std::cout << "actual: " << vecAccumulateWithMap(actual, ts, sep) << '\n';
    xfailure("actual is different from expect");
  }
}


static void checkEqual(
  std::vector<std::string> const &actual,
  std::vector<std::string> const &expect)
{
  std::string sep(" ");

  if (actual != expect) {
    std::cout << "expect: " << vecAccumulateWith(expect, sep) << '\n';
    std::cout << "actual: " << vecAccumulateWith(actual, sep) << '\n';
    xfailure("actual is different from expect");
  }
}


static void testOne(
  std::vector<std::string> const &args,
  std::vector<GCCOptions::Option> const &expect)
{
  GCCOptions gccOptions(args);
  std::vector<GCCOptions::Option> const &actual = gccOptions.getOptions();

  checkEqual(actual, expect);

  // Rebuild the word sequence and check it is the same.
  std::vector<std::string> reconstructed;
  gccOptions.getCommandWords(reconstructed);

  checkEqual(reconstructed, args);
}


// Make it a little easier to name the enumerators.
#define SEP(name) GCCOptions::SEP_##name
#define SYN(name) GCCOptions::SYN_##name


static void testParse()
{
  // Bare option name.
  #define BARE(name) { name, SEP(NONE), "", SYN(NONE) }

  // Space-separated option and arg.
  #define SPACE(name, arg) { name, SEP(SPACE), arg, SYN(NONE) }

  // Equals-separated option and arg.
  #define EQUALS(name, arg) { name, SEP(EQUALS), arg, SYN(NONE) }

  // Juxtaposed option and arg.
  #define EMPTY(name, arg) { name, SEP(EMPTY), arg, SYN(NONE) }

  // Argument alone.
  #define ARG(arg) { "", SEP(NONE), arg, SYN(NONE) }

  struct InputAndResult {
    std::vector<std::string> m_input;
    std::vector<GCCOptions::Option> m_expect;
  };
  InputAndResult const iars[] = {
    // A few preliminaries.
    {
      { "-c" },
      { BARE("-c") },
    },
    {
      { "-o", "fname" },
      { SPACE("-o", "fname") },
    },
    {
      { "-ofname" },
      { EMPTY("-o", "fname") },
    },
    {
      { "-ofname", "-c", "foo.c" },
      { EMPTY("-o", "fname"), BARE("-c"), ARG("foo.c") },
    },

    // OS_EMPTY
    {
      { "-Werror", "-Wl,-blah" },
      { EMPTY("-W", "error"), EMPTY("-W", "l,-blah") },
    },
    {
      { "-print-x" },
      { EMPTY("-print-", "x") },
    },
    {
      { "-W" },
      { { "-W", SEP(NONE), "", SYN(MISSING_ARGUMENT) } },
    },

    // OS_EMPTY | OS_SPACE
    {
      { "-Ifoo" },
      { EMPTY("-I", "foo") },
    },
    {
      { "-I", "foo" },
      { SPACE("-I", "foo") },
    },
    {
      { "-I=foo" },
      { EMPTY("-I", "=foo") },
    },
    {
      { "-I" },
      { { "-I", SEP(NONE), "", SYN(ABRUPT_END) } },
    },
    {
      { "-includefoo" },
      { EMPTY("-include", "foo") },
    },

    // OS_EMPTY | OS_BARE
    {
      { "-O" },
      { BARE("-O") },
    },
    {
      { "-O2" },
      { EMPTY("-O", "2") },
    },
    {
      { "-O", "fname" },
      { BARE("-O"), ARG("fname") },
    },
    {
      { "-staticlibgcc" },
      { EMPTY("-static", "libgcc") },
    },

    // OS_EMPTY | OS_SPACE | OS_EQUALS
    {
      { "-Dfoo" },
      { EMPTY("-D", "foo") },
    },
    {
      { "-D", "foo" },
      { SPACE("-D", "foo") },
    },
    {
      { "-D=foo" },
      { EQUALS("-D", "foo") },
    },
    {
      { "-D" },
      { { "-D", SEP(NONE), "", SYN(ABRUPT_END) } },
    },

    // OS_EQUALS | OR_SPACE
    {
      { "--param", "foo=bar" },
      { SPACE("--param", "foo=bar") },
    },
    {
      { "--param=foo=bar" },
      { EQUALS("--param", "foo=bar") },
    },
    {
      { "--paramfoo=bar" },
      { { "--param", SEP(EMPTY), "foo=bar", SYN(MISSING_EQUALS) } },
    },
    {
      { "--param" },
      { { "--param", SEP(NONE), "", SYN(ABRUPT_END) } },
    },

    // OS_SPACE | OS_EXACT
    {
      { "-dumpbase", "fname" },
      { SPACE("-dumpbase", "fname") },
    },
    {
      { "-dumpbase-ext", "fname" },
      { SPACE("-dumpbase-ext", "fname") },
    },
    {
      { "-dumpdir", "fname" },
      { SPACE("-dumpdir", "fname") },
    },
    {     // An '=' *cannot* follow -dumpbase and be recognized as such.
      { "-dumpbase=fname" },
      { EMPTY("-d", "umpbase=fname") },
    },
    {
      { "-dumpbasefname" },
      { EMPTY("-d", "umpbasefname") },
    },
    {
      { "-dumpbsefname" },
      { EMPTY("-d", "umpbsefname") },
    },
    {
      { "-dumpbse", "fname" },     // also "-d", so fname not consumed
      { EMPTY("-d", "umpbse"), ARG("fname") },
    },
    {
      { "-dumpbase-extx", "fname" },
      { EMPTY("-d", "umpbase-extx"), ARG("fname") },
    },

    // OS_SPACE
    {
      { "-Xlinker", "-lc" },
      { SPACE("-Xlinker", "-lc") },
    },
    {
      { "-Xlinker-lc" },
      { { "-Xlinker", SEP(EMPTY), "-lc", SYN(MISSING_SEPARATOR) } },
    },
    {
      { "-Xlinker=-lc" },
      { { "-Xlinker", SEP(EQUALS), "-lc", SYN(INVALID_EQUALS) } },
    },
    {
      { "-Xlinker" },
      { { "-Xlinker", SEP(NONE), "", SYN(ABRUPT_END) } },
    },

    // OS_EQUALS
    {
      { "-std=c11" },
      { EQUALS("-std", "c11") },
    },
    {
      { "-std", "c11" },
      { { "-std", SEP(NONE), "", SYN(MISSING_EQUALS) }, ARG("c11") },
    },
    {
      { "-stdc11" },
      { { "-std", SEP(EMPTY), "c11", SYN(MISSING_EQUALS) } },
    },

    // OS_BARE
    {
      { "-E" },
      { BARE("-E") },
    },
    {
      { "-Enonsense" },
      { { "-E", SEP(EMPTY), "nonsense", SYN(TRAILING_JUNK) } },
    },
    {
      { "-gen-decls" },
      { BARE("-gen-decls") },
    },
    {
      { "-gen-dexls" },            // "-g" takes over
      { EMPTY("-g", "en-dexls") },
    },
    {
      { "-undef" },
      { BARE("-undef") },
    },
    {
      { "-undexf" },               // "-u" takes over
      { EMPTY("-u", "ndexf") },
    },

    // OS_BARE | OS_EQUALS
    {
      { "--help", "foo" },
      { BARE("--help"), ARG("foo") },
    },
    {
      { "--help=foo" },
      { EQUALS("--help", "foo") },
    },

    // Unrecognized.  It's actually a bit tricky to get into this case
    // because you have to avoid using a prefix that *is* recognized.
    {
      { "-an-unrecognized-sw" },
      { { "-an-unrecognized-sw", SEP(NONE), "", SYN(UNRECOGNIZED) } },
    },
  };

  #undef BARE
  #undef SPACE
  #undef EQUALS
  #undef EMPTY
  #undef ARG

  for (InputAndResult const &iar : iars) {
    testOne(iar.m_input, iar.m_expect);
  }
}


static void testOutputMode()
{
  struct OMTest {
    std::vector<std::string> m_words;
    GCCOptions::OutputMode m_expect;
  }
  const tests[] = {
    // Columns: \{ @40:OM \}
    { { },                             OM(EXECUTABLE) },
    { { "hello.c" },                   OM(EXECUTABLE) },
    { { "-c" },                        OM(OBJECT_CODE) },
    { { "-c", "-c" },                  OM(OBJECT_CODE) },
    { { "-E" },                        OM(PREPROCESSED) },
    { { "-S" },                        OM(ASSEMBLY) },
    { { "-c", "-E" },                  OM(PREPROCESSED) },
    { { "-E", "-c" },                  OM(PREPROCESSED) },
    { { "-c", "-S" },                  OM(ASSEMBLY) },
    { { "-S", "-c" },                  OM(ASSEMBLY) },
    { { "-E", "-S" },                  OM(PREPROCESSED) },
    { { "-S", "-E" },                  OM(PREPROCESSED) },
    { { "-E", "-S", "-c" },            OM(PREPROCESSED) },
    { { "-c", "-S", "-E" },            OM(PREPROCESSED) },
    { { "-c", "-S", "-E", "-c" },      OM(PREPROCESSED) },
    { { "-M" },                        OM(DEPENDENCIES) },
    { { "-M", "-E" },                  OM(DEPENDENCIES) },
    { { "-c", "-M" },                  OM(DEPENDENCIES) },
    { { "-M", "-S" },                  OM(DEPENDENCIES) },
    { { "-M", "-E", "-S", "-c" },      OM(DEPENDENCIES) },
    { { "-MM" },                       OM(DEPENDENCIES) },
    { { "-MM", "-E" },                 OM(DEPENDENCIES) },
    { { "-c", "-MM" },                 OM(DEPENDENCIES) },
    { { "-dumpversion", "-MM" },       OM(GCC_INFO) },
    { { "-E", "-dumpmachine" },        OM(GCC_INFO) },
    { { "-c", "-dumpmachin" },         OM(OBJECT_CODE) },
  };

  for (OMTest const &t : tests) {
    GCCOptions opts(t.m_words);
    GCCOptions::OutputMode actual = opts.outputMode();
    xassert(actual == t.m_expect);
  }
}


static void testLanguageForFile()
{
  static struct LangTest {
    char const *m_fname;
    char const *m_expect;
  }
  const tests[] = {
    { "hello.c",             "c" },
    { "hello.cc",            "c++" },
    { "hello.C",             "c++" },
    { "gorf.f",              "f77" },
    { "foo.bar.tcc",         "c++-header" },
    { ".....c++",            "c++" },
    { "",                    "" },
    { "hello.c.",            "" },
    { "hello.o",             "" },
  };

  for (LangTest const &lt : tests) {
    std::string actual = gccLanguageForFile(lt.m_fname, "" /*xLang*/);
    EXPECT_EQ(actual, std::string(lt.m_expect));
  }

  std::string actual = gccLanguageForFile("f.c", "c++");
  xassert(actual == "c++");
}


static void testLangInCommand()
{
  struct LICTest {
    // Sequence of GCC command line words.
    std::vector<std::string> m_words;

    // Sequence of "-x" settings in effect after parsing the
    // corresponding argument word.
    std::vector<char const *> m_expect;
  }
  const tests[] = {
    {
      { "hello.c" },
      { "" }
    },
    {
      { "hello.c", "foo.o" },
      { "", "" }
    },
    {
      { "hello.c", "-xc", "hello.o" },
      { "", "c", "c" }
    },
    {
      { "hello.c", "-xc", "hello.o", "-xc++", "f.c" },
      { "", "c", "c", "c++", "c++" }
    },
    {
      { "hello.c", "-xc", "hello.o", "-xnone", "hello.o" },
      { "", "c", "c", "", "" }
    },
  };

  for (LICTest const &t : tests) {
    GCCOptions opts(t.m_words);
    for (GCCOptions::Iter iter(opts); iter.hasMore(); iter.adv()) {
      std::string actual = iter.xLang();
      char const *expect = t.m_expect.at(iter.index());
      xassert(actual == expect);
    }
  }
}


static void testSpecifiesGCCOutputMode()
{
  #define om_none GCCOptions::NUM_OUTPUT_MODES

  static struct {
    char const *m_name;

    // Expected mode, or NUM_OUTPUT_MODES to mean none.
    GCCOptions::OutputMode m_expect;
  }
  const tests[] = {
    // Columns: \{ \S+ @30:\S+ \}
    { "-c",                  OM(OBJECT_CODE) },
    { "-E",                  OM(PREPROCESSED) },
    { "-S",                  OM(ASSEMBLY) },
    { "-f",                  om_none },
    { "",                    om_none },
    { "-f-c",                om_none },
    { "-M",                  OM(DEPENDENCIES) },
    { "-MM",                 OM(DEPENDENCIES) },

    // These two specify to generate dependency rules as a side effect,
    // but do not change what the primary output (which goes into the
    // file named by -o) is.
    { "-MD",                 om_none },
    { "-MMD",                om_none },

    { "-dumpversio",         om_none },
    { "-dumpversion",        OM(GCC_INFO) },
    { "-dumpversionx",       om_none },
    { "-dumpmachine",        OM(GCC_INFO) },
    { "-dumpfullversion",    OM(GCC_INFO) },
    { "-dumpspecs",          OM(GCC_INFO) },
  };

  for (auto const &t : tests) {
    GCCOptions::OutputMode actualMode = om_none;
    bool actual = specifiesGCCOutputMode(t.m_name, /*OUT*/ actualMode);
    EXPECT_EQ(actual, t.m_expect!=om_none);
    if (actual) {
      EXPECT_EQ(actualMode, t.m_expect);
    }
  }

  #undef om_none
}


static void testToString()
{
  xassert(streq(toString(SEP(SPACE)), "SEP_SPACE"));
  xassert(streq(toString(SYN(UNRECOGNIZED)), "SYN_UNRECOGNIZED"));
  xassert(streq(toString(GCCOptions::OM_ASSEMBLY), "OM_ASSEMBLY"));

  GCCOptions::Option opt("n", SEP(EQUALS),
                         "a", SYN(NONE));
  xassert(opt.toString() ==
    "{ name=\"n\", sep=SEP_EQUALS, arg=\"a\", syn=SYN_NONE }");
}


static void testAddOption()
{
  GCCOptions opts;

  opts.addOption("n1", SEP(EQUALS), "a1", SYN(INVALID_EQUALS));
  opts.addOption(GCCOptions::Option("n2", SEP(NONE), "a2", SYN(NONE)));
  opts.addInputFile("file1");
  opts.addBareOption("-c");
  opts.addSpaceOption("-o", "file2");
  opts.addEmptyOption("-D", "foobar");

  std::vector<GCCOptions::Option> expect = {
    // Columns: \{ \S+ \S+ \S+ \S+ \}
    { "n1", SEP(EQUALS), "a1",     SYN(INVALID_EQUALS) },
    { "n2", SEP(NONE),   "a2",     SYN(NONE)           },
    { "",   SEP(NONE),   "file1",  SYN(NONE)           },
    { "-c", SEP(NONE),   "",       SYN(NONE)           },
    { "-o", SEP(SPACE),  "file2",  SYN(NONE)           },
    { "-D", SEP(EMPTY),  "foobar", SYN(NONE)           },
  };
  checkEqual(opts.getOptions(), expect);
}


static void testGetExplicitOutputFile()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;    // NULL if we expect false return.
  }
  const tests[] = {
    {
      {},
      NULL,
    },
    {
      { "-c" },
      NULL,
    },
    {
      { "-c", "foo.c" },
      NULL,
    },
    {
      { "-o", "foo" },
      "foo",
    },
    {
      { "-obar", "foo" },
      "bar",
    },
    {
      { "-M", "-obar", "foo" },
      "bar",
    },
    {
      { "-M", "-obar", "-MFbaz", "foo" },
      "baz",
    },
    {
      { "-MD", "-obar", "-MFbaz", "foo" },
      "bar",
    },
    {
      { "-MM", "-obar", "-MF", "baz", "foo" },
      "baz",
    },
    {
      { "-M", "-MFbaz" },
      "baz",
    },
    {
      { "-M" "foo" },
      NULL,
    },
  };

  for (auto const &t : tests) {
    GCCOptions opts(t.m_input);
    std::string actual;
    bool found = opts.getExplicitOutputFile(actual);
    EXPECT_EQ(found, t.m_expect!=NULL);
    if (found) {
      EXPECT_EQ(actual, std::string(t.m_expect));
    }
  }
}


static void testGetFirstSourceFileName()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;
  }
  const tests[] = {
    {
      {},
      NULL
    },
    {
      { "foo.c" },
      "foo.c"
    },
    {
      { "foo.c", "bar.c" },
      "foo.c"
    },
    {
      { "foo.o", "bar.c" },
      "bar.c"
    },
    {
      { "-xc", "foo.o", "bar.c" },
      "foo.o"
    },
    {
      { "-xnone", "foo.o", "bar.c" },
      "bar.c"
    },
    {
      { "-xc", "-xnone", "foo.o", "bar.c" },
      "bar.c"
    },
    {
      { "-xc", "-xnone", "foo.o" },
      NULL
    },
  };

  for (auto const &t : tests) {
    GCCOptions opts(t.m_input);
    std::string actual;
    bool found = opts.getFirstSourceFileName(actual);
    EXPECT_EQ(found, t.m_expect!=NULL);
    if (found) {
      EXPECT_EQ(actual, std::string(t.m_expect));
    }
  }
}


static void testGetOutputFile()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;
  }
  const tests[] = {
    {
      {},
      "a.out"
    },
    {
      { "-o", "foo" },
      "foo"
    },
    {
      { "-c", "foo.c" },
      "foo.o"
    },
    {
      { "-c", "src/foo.c" },
      "foo.o"      // Does *not* include 'src'.
    },
    {
      { "-c", "foo.c", "-o", "bar.o" },
      "bar.o"
    },
    {
      { "-S", "foo.c" },
      "foo.s"
    },
    {
      { "-c" },
      NULL
    },

    // See doc/index.html#gcc-dependency-rules .
    {
      { "-M", "bar.c" },
      NULL
    },
    {
      { "-M", "-MFbar.d", "foo.c" },
      "bar.d"
    },
    {
      { "-M", "-obar.d", "foo.c" },
      "bar.d"
    },
    {
      { "-M", "-MFbaz.d", "-obar.d", "foo.c" },
      "baz.d"
    },
  };

  for (auto const &t : tests) {
    GCCOptions opts(t.m_input);
    std::string actual;
    bool found = opts.getOutputFile(actual);
    EXPECT_EQ(found, t.m_expect!=NULL);
    if (found) {
      EXPECT_EQ(actual, std::string(t.m_expect));
    }
  }
}


static void testCreatesDependencyFile()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;    // NULL for false return
  }
  const tests[] = {
    {
      { "-c", "foo.c" },
      NULL
    },
    {
      { "-c", "foo.c", "-MF", "something" },  // GCC would complain
      NULL
    },
    {
      { "-c", "foo.c", "-MD" },
      "foo.d"
    },
    {
      { "-c", "src/foo.c", "-MD" },
      "foo.d"
    },
    {
      { "-c", "-xc", "foo", "-MD" },
      "foo.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-MF", "bar.d" },
      "bar.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-MF", "obj/bar.d" },
      "obj/bar.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-o", "bar.o" },
      "bar.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-o", "bar" },
      "bar.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-o", "bar.bar.o" },
      "bar.bar.d"
    },
    {
      { "-c", "foo.c", "-MMD", "-MF", "bar.d", "-o", "baz.d" },
      "bar.d"                  // -MF takes precedence
    },
  };

  for (auto const &t : tests) {
    GCCOptions opts(t.m_input);
    std::string actual;
    bool found = opts.createsDependencyFile(actual);
    EXPECT_EQ(found, t.m_expect!=NULL);
    if (found) {
      EXPECT_EQ(actual, std::string(t.m_expect));
    }
  }
}


static void testGetDefaultDependencyTarget()
{
  struct Test {
    std::vector<std::string> m_input;
    char const *m_expect;    // NULL for false return
  }
  const tests[] = {
    {
      {},
      NULL,
    },
    {
      { "-c", "foo.c" },
      "foo.o"
    },
    {
      { "-c", "src/foo.c" },
      "foo.o"
    },
    {
      { "-c", "foo.c", "-MD" },
      "foo.o"
    },
    {
      { "-c", "src/foo.c", "-MD" },
      "foo.o"
    },
    {
      { "-c", "-xc", "foo", "-MD" },
      "foo.o"
    },
    {
      { "-c", "foo.c", "-MMD", "-o", "bar.o" },
      "bar.o"
    },
    {
      { "-c", "foo.c", "-MMD", "-o", "bar" },
      "bar"
    },
    {
      { "-c", "foo.c", "-MMD", "-MF", "bar.d", "-o", "obj/baz.o" },
      "obj/baz.o"
    },
  };

  for (auto const &t : tests) {
    try {
      GCCOptions opts(t.m_input);
      std::string actual;
      bool found = opts.getDefaultDependencyTarget(actual);
      EXPECT_EQ(found, t.m_expect!=NULL);
      if (found) {
        EXPECT_EQ(actual, std::string(t.m_expect));
      }
    }
    catch (XBase &x) {
      x.prependContext(stringb(__func__ << ": " << toString(t.m_input)));
      throw;
    }
  }
}


static void testNumSourceFiles()
{
  struct Test {
    std::vector<std::string> m_input;
    int m_expect;
  }
  const tests[] = {
    {
      {},
      0,
    },
    {
      { "-c", "foo.c" },
      1,
    },
    {
      { "-c", "foo.c", "bar.c" },
      2,
    },
    {
      { "-c", "src/foo.c" },
      1,
    },
    {
      { "-c", "foo.c", "-MD" },
      1,
    },
    {
      { "foo.c", "bar.c", "other.o" },
      2,
    },
    {
      { "foo.c", "bar.c", "-xc", "other.o" },
      3,
    },
    {
      { "foo.c", "bar.c", "-xc", "-xnone", "other.o" },
      2,
    },
  };

  for (auto const &t : tests) {
    try {
      GCCOptions opts(t.m_input);
      int actual = opts.numSourceFiles();
      EXPECT_EQ(actual, t.m_expect);
    }
    catch (XBase &x) {
      x.prependContext(stringb(__func__ << ": " << toString(t.m_input)));
      throw;
    }
  }
}


void test_gcc_options()
{
  // Defined in gcc-options.cc.
  gcc_options_check_tables();

  testEmpty();
  testParse();
  testOutputMode();
  testLanguageForFile();
  testLangInCommand();
  testSpecifiesGCCOutputMode();
  testToString();
  testAddOption();
  testGetExplicitOutputFile();
  testGetFirstSourceFileName();
  testGetOutputFile();
  testCreatesDependencyFile();
  testGetDefaultDependencyTarget();
  testNumSourceFiles();
}


// EOF
