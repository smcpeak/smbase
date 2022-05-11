// gcc-options-test.cc
// Tests for gcc-options module.

#include "gcc-options.h"               // module under test

#include "sm-test.h"                   // EXPECT_EQ
#include "vector-utils.h"              // accumulateWith[Map]
#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout


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
    std::cout << "expect: " << accumulateWithMap(expect, ts, sep) << '\n';
    std::cout << "actual: " << accumulateWithMap(actual, ts, sep) << '\n';
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

  if (reconstructed != args) {
    std::string sep(" ");
    std::cout << "args         : " << accumulateWith(args, sep) << '\n';
    std::cout << "reconstructed: " << accumulateWith(reconstructed, sep) << '\n';
    xfailure("reconstructed is different from args");
  }
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
    {
      { "-dumpbase", "fname" },
      { SPACE("-dumpbase", "fname") },
    },
    {
      { "-dumpbase=fname" },
      { EQUALS("-dumpbase", "fname") },
    },
    {
      { "-dumpbasefname" },
      { { "-dumpbase", SEP(EMPTY), "fname", SYN(MISSING_EQUALS) } },
    },
    {
      { "-dumpbsefname" },         // "-d" takes over
      { EMPTY("-d", "umpbsefname") },
    },
    {
      { "-dumpbse", "fname" },     // also "-d", so fname not consumed
      { EMPTY("-d", "umpbse"), ARG("fname") },
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
  #define OM(name) GCCOptions::OM_##name

  struct OMTest {
    std::vector<std::string> m_words;
    GCCOptions::OutputMode m_expect;
  }
  const tests[] = {
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
  };

  for (OMTest const &t : tests) {
    GCCOptions opts(t.m_words);
    GCCOptions::OutputMode actual = opts.outputMode();
    xassert(actual == t.m_expect);
  }

  #undef OM
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
  static struct {
    char const *m_name;
    bool m_expect;
  } const tests[] = {
    { "-c",        true },
    { "-E",        true },
    { "-S",        true },
    { "-f",        false },
    { "",          false },
    { "-f-c",      false },
  };

  for (auto t : tests) {
    bool actual = specifiesGCCOutputMode(t.m_name);
    xassert(actual == t.m_expect);
  }
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

  std::vector<GCCOptions::Option> expect = {
    { "n1", SEP(EQUALS), "a1",    SYN(INVALID_EQUALS) },
    { "n2", SEP(NONE),   "a2",    SYN(NONE)           },
    { "",   SEP(NONE),   "file1", SYN(NONE)           },
    { "-c", SEP(NONE),   "",      SYN(NONE)           },
    { "-o", SEP(NONE),   "file2", SYN(NONE)           },
  };
  checkEqual(opts.getOptions(), expect);
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
}


// EOF
