// gcc-options-test.cc
// Tests for gcc-options module.

#include "gcc-options.h"               // module under test

#include "vector-utils.h"              // accumulateWith[Map]
#include "xassert.h"                   // xassert

#include <iostream>                    // std::cout


static void testEmpty()
{
  std::vector<std::string> opts;
  GCCOptions gccOptions(opts);
  xassert(opts.empty());
}


static void testOne(
  std::vector<std::string> const &args,
  std::vector<GCCOptions::Option> const &expect)
{
  GCCOptions gccOptions(args);
  std::vector<GCCOptions::Option> const &actual = gccOptions.m_options;

  std::string sep(" ");

  if (actual != expect) {
    auto ts = [](GCCOptions::Option const &opt) { return opt.toString(); };
    std::cout << "args: " << accumulateWith(args, sep) << '\n';
    std::cout << "expect: " << accumulateWithMap(expect, ts, sep) << '\n';
    std::cout << "actual: " << accumulateWithMap(actual, ts, sep) << '\n';
    xfailure("actual is different from expect");
  }

  // Rebuild the word sequence and check it is the same.
  std::vector<std::string> reconstructed;
  for (auto opt : gccOptions.m_options) {
    opt.appendWords(reconstructed);
  }

  if (reconstructed != args) {
    std::cout << "args         : " << accumulateWith(args, sep) << '\n';
    std::cout << "reconstructed: " << accumulateWith(reconstructed, sep) << '\n';
    xfailure("reconstructed is different from args");
  }
}

static void test1()
{
  // Make it a little easier to name the enumerators.
  #define SEP(name) GCCOptions::SEP_##name
  #define SYN(name) GCCOptions::SYN_##name

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
  InputAndResult iars[] = {
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
  };

  #undef BARE
  #undef SPACE
  #undef EQUALS
  #undef EMPTY
  #undef ARG

  #undef SEP
  #undef SYN

  for (InputAndResult const &iar : iars) {
    testOne(iar.m_input, iar.m_expect);
  }
}

void test_gcc_options()
{
  testEmpty();
  test1();
}

// EOF
