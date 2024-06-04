// gcc-options.cc
// Code for gcc-options.h.

#include "gcc-options.h"               // this module

#include "binary-lookup.h"             // binary_lookup
#include "container-util.h"            // insertUnique
#include "sm-file-util.h"              // SMFileUtil
#include "sm-macros.h"                 // EMEMB, DEFINE_ENUMERATION_TO_STRING
#include "strcmp-compare.h"            // StrcmpCompare, etc.
#include "strictly-sorted.h"           // is_strictly_sorted
#include "string-util.h"               // stripExtension, stringInSortedArray, join, beginsWith
#include "xassert.h"                   // xassert

#include <sstream>                     // std::ostringstream

#include <string.h>                    // strrchr


// ----------------------- Option categories ---------------------------
// Description of an option or set of options.
struct OptionsTableEntry {
  // Name of the option as it will be seen by clients of GCCOptions.
  // This could be the option as documented in the GCC manual, or it
  // could be a common prefix of a set of options that all have the same
  // syntactic characteristics.
  char const *m_name;

  // The syntactic characteristics of the option(s).  They are
  // determined in part from the GCC manual, but also from
  // experimentation with GCC-9.3.0.
  //
  // I declare this as 'int' to avoid complicating the table below.
  int m_syntax;
};

// Shorter names to make the table more compact.
#define BARE   GCCOptions::OS_BARE
#define SPACE  GCCOptions::OS_SPACE
#define EMPTY  GCCOptions::OS_EMPTY
#define EQUALS GCCOptions::OS_EQUALS
#define EXACT  GCCOptions::OS_EXACT

// Main options table.  The entries are intended to be sorted in
// "LANG=C sort" order, although the only thing that really matters is
// that a prefix comes before the string it is a prefix of.
static OptionsTableEntry const optionsTable[] = {
  // Columns: \{ \S+ \S+ \}
  { "-###",                     BARE                   },
  { "--coverage",               BARE                   },
  { "--entry",                  SPACE | EQUALS         }, // Docs say '=', gcc accepts ' ' too.
  { "--help",                   BARE | EQUALS          },
  { "--param",                  SPACE | EQUALS         }, // Docs say ' ', gcc accepts '=' too.
  { "--sysroot",                EQUALS                 }, // Docs say '=', ' ' is untested by me.
  { "--target-help",            BARE                   },
  { "--version",                BARE                   },
  { "-A",                       SPACE | EMPTY          },
  { "-B",                       SPACE | EMPTY          }, // '='?
  { "-C",                       BARE                   },
  { "-CC",                      BARE                   },
  { "-D",                       SPACE | EMPTY | EQUALS }, // See note ESO.
  { "-E",                       BARE                   },
  { "-H",                       BARE                   },
  { "-I",                       SPACE | EMPTY          }, // GCC does *not* accept '=' for this one (it is treated as part of the directory name).
  { "-L",                       SPACE | EMPTY          }, // Not sure about '=' here.
  { "-M",                       BARE                   },
  { "-MD",                      BARE                   },
  { "-MF",                      SPACE | EMPTY          },
  { "-MG",                      BARE                   },
  { "-MM",                      BARE                   },
  { "-MMD",                     BARE                   },
  { "-MP",                      BARE                   },
  { "-MQ",                      SPACE | EMPTY          },
  { "-MT",                      SPACE | EMPTY          },
  { "-Mno-modules",             BARE                   }, // My GCC-9.3.0 says this is unrecognized...
  { "-O",                       BARE | EMPTY           },
  { "-P",                       BARE                   },
  { "-Q",                       BARE                   },
  { "-S",                       BARE                   },
  { "-T",                       SPACE | EMPTY          },
  { "-U",                       SPACE | EMPTY          }, // GCC rejects '=' on this one (treats it as part of the symbol, then chokes).
  { "-W",                       EMPTY                  },
  { "-Xassembler",              SPACE                  },
  { "-Xlinker",                 SPACE                  },
  { "-Xpreprocessor",           SPACE                  },
  { "-ansi",                    BARE                   },
  { "-aux-info",                SPACE | EQUALS         }, // Docs say ' ', gcc accepts '=' too.
  { "-c",                       BARE                   },
  { "-d",                       EMPTY                  },
  { "-dumpbase",                SPACE | EXACT          }, // Note ambiguity with "-d".
  { "-dumpbase-ext",            SPACE | EXACT          },
  { "-dumpdir",                 SPACE | EXACT          },
  { "-dumpfullversion",         BARE | EXACT           },
  { "-dumpmachine",             BARE | EXACT           },
  { "-dumpspecs",               BARE | EXACT           },
  { "-dumpversion",             BARE | EXACT           },
  { "-e",                       SPACE | EMPTY          },
  { "-f",                       EMPTY                  }, // Covers hundreds of individual options.
  { "-g",                       BARE | EMPTY           },
  { "-gen-decls",               BARE                   }, // Note ambiguity with "-g".
  { "-idirafter",               SPACE | EMPTY          },
  { "-imacros",                 SPACE | EMPTY          },
  { "-imulitilib",              SPACE | EMPTY          },
  { "-include",                 SPACE | EMPTY          }, // Yes, "-includesome_file" works.
  { "-iplugindir",              EQUALS                 }, // '=' required, ' ' rejected.
  { "-iprefix",                 SPACE | EMPTY          },
  { "-iquote",                  SPACE | EMPTY          },
  { "-isysroot",                SPACE | EMPTY          },
  { "-isystem",                 SPACE | EMPTY          },
  { "-iwithprefix",             SPACE | EMPTY          },
  { "-iwithprefixbefore",       SPACE | EMPTY          },
  { "-l",                       SPACE | EMPTY | EQUALS }, // See note ESO.
  { "-m",                       EMPTY                  },
  { "-no",                      EMPTY                  },
  { "-o",                       SPACE | EMPTY          },
  { "-p",                       BARE                   },
  { "-pass-exit-codes",         BARE                   },
  { "-pedantic",                BARE                   },
  { "-pedantic-errors",         BARE                   },
  { "-pg",                      BARE                   },
  { "-pie",                     BARE                   },
  { "-pipe",                    BARE                   },
  { "-print-",                  EMPTY                  },
  { "-print-objc-runtime-info", BARE                   },
  { "-pthread",                 BARE                   },
  { "-r",                       BARE                   },
  { "-rdynamic",                BARE                   },
  { "-remap",                   BARE                   },
  { "-s",                       BARE                   },
  { "-shared",                  BARE | EMPTY           },
  { "-specs",                   SPACE | EQUALS         }, // Docs say ' ', gcc accepts '=' too.
  { "-static",                  BARE | EMPTY           },
  { "-std",                     EQUALS                 }, // '=' required, ' ' rejected.
  { "-stdlib",                  EQUALS                 }, // Docs say '=', ' ' is untested by me.
  { "-symbolic",                BARE                   },
  { "-traditional",             BARE                   },
  { "-traditional-cpp",         BARE                   },
  { "-trigraphs",               BARE                   },
  { "-u",                       SPACE | EMPTY          }, // I'm not sure how GCC interprets '=', but my guess is it's treated as part of the symbol.
  { "-undef",                   BARE                   }, // Note ambiguity with "-u".
  { "-v",                       BARE                   },
  { "-w",                       BARE                   },
  { "-wrapper",                 SPACE                  },
  { "-x",                       SPACE | EMPTY          },
  { "-z",                       SPACE | EMPTY          },
};

// Note ESO: Nothing is documented as accepting all three of OS_EMPTY,
// OS_SPACE, and OS_EQUALS, but I determined through experimentation
// that some options do, and do *not* fold the '=' into the argument
// (meaning OS_EQUALS takes precedence over OS_EMPTY).

#undef BARE
#undef SPACE
#undef EMPTY
#undef EQUALS
#undef EXACT


// -------------------------- WordIterator -----------------------------
GCCOptions::WordIterator::WordIterator(std::vector<std::string> const &args)
  : m_args(args),
    m_index(0)
{}


std::string GCCOptions::WordIterator::nextAdv()
{
  xassert(hasMore());
  return m_args.at(m_index++);
}


// --------------------------- Separator -------------------------------
DEFINE_ENUMERATION_TO_STRING(
  GCCOptions::Separator,
  GCCOptions::NUM_SEPARATORS,
  (
    "SEP_NONE",
    "SEP_EMPTY",
    "SEP_SPACE",
    "SEP_EQUALS",
  )
)


// -------------------------- SyntaxError ------------------------------
DEFINE_ENUMERATION_TO_STRING(
  GCCOptions::SyntaxError,
  GCCOptions::NUM_SYNTAX_ERRORS,
  (
    "SYN_NONE",
    "SYN_ABRUPT_END",
    "SYN_UNRECOGNIZED",
    "SYN_TRAILING_JUNK",
    "SYN_MISSING_SEPARATOR",
    "SYN_MISSING_EQUALS",
    "SYN_MISSING_ARGUMENT",
    "SYN_INVALID_EQUALS",
  )
)


// --------------------------- OutputMode ------------------------------
DEFINE_ENUMERATION_TO_STRING(
  GCCOptions::OutputMode,
  GCCOptions::NUM_OUTPUT_MODES,
  (
    "OM_GCC_INFO",
    "OM_DEPENDENCIES",
    "OM_PREPROCESSED",
    "OM_ASSEMBLY",
    "OM_OBJECT_CODE",
    "OM_EXECUTABLE",
  )
)


char const *extensionForGCCOutputMode(GCCOptions::OutputMode outputMode)
{
  static char const * const exts[] = {
    "",            // Output goes to stdout.
    ".d",
    ".i",
    ".s",
    ".o",
    ".out",
  };
  ASSERT_TABLESIZE(exts, GCCOptions::NUM_OUTPUT_MODES);
  xassert((unsigned)outputMode < GCCOptions::NUM_OUTPUT_MODES);
  return exts[outputMode];
}


// ----------------------------- Option --------------------------------
GCCOptions::Option::Option(std::string const &name, Separator separator,
                           std::string const &argument, SyntaxError syntaxError)
  : m_name(name),
    m_separator(separator),
    m_argument(argument),
    m_syntaxError(syntaxError)
{}


GCCOptions::Option::~Option()
{}


bool GCCOptions::Option::operator== (Option const &obj) const
{
  return EMEMB(m_name) &&
         EMEMB(m_separator) &&
         EMEMB(m_argument) &&
         EMEMB(m_syntaxError);
}


void GCCOptions::Option::appendWords(std::vector<std::string> &dest) const
{
  switch (m_separator) {
    default:                 // gcov-ignore
      xfailure("unrecognized separator");
      break;

    case SEP_NONE:
      // Either the name or the argument will be empty.
      if (!m_name.empty()) {
        dest.push_back(m_name);
      }
      else {
        dest.push_back(m_argument);
      }
      break;

    case SEP_EMPTY:
      dest.push_back(m_name + m_argument);
      break;

    case SEP_SPACE:
      dest.push_back(m_name);
      dest.push_back(m_argument);
      break;

    case SEP_EQUALS:
      dest.push_back(m_name + "=" + m_argument);
      break;
  }
}


std::ostream& GCCOptions::Option::insert(std::ostream &os) const
{
  os << "{ name=" << quoted(m_name.c_str())
     << ", sep=" << ::toString(m_separator)
     << ", arg=" << quoted(m_argument.c_str())
     << ", syn=" << ::toString(m_syntaxError)
     << " }";
  return os;
}


std::string GCCOptions::Option::toString() const
{
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}


// ------------------------------ Iter ---------------------------------
GCCOptions::Iter::Iter(GCCOptions const &options)
  : m_options(options),
    m_index(0),
    m_xLang()
{
  if (hasMore()) {
    updateState();
  }
}


GCCOptions::Iter::~Iter()
{}


void GCCOptions::Iter::updateState()
{
  Option const &o = opt();
  if (o.m_name == "-x") {
    if (o.m_argument == "none") {
      m_xLang.clear();
    }
    else {
      m_xLang = o.m_argument;
    }
  }
}


GCCOptions::Option const &GCCOptions::Iter::opt() const
{
  xassert(hasMore());
  return m_options.at(m_index);
}


bool GCCOptions::Iter::optIsSourceFile() const
{
  if (opt().isInputFile()) {
    // If we deduce (or there was specified) a non-empty language
    // string, then this is regarded as source code.
    return !gccLanguageForFile(opt().m_argument, xLang()).empty();
  }
  else {
    return false;
  }
}


bool GCCOptions::Iter::hasMore() const
{
  return index() < m_options.size();
}


void GCCOptions::Iter::adv()
{
  xassert(hasMore());
  m_index++;
  if (hasMore()) {
    updateState();
  }
}


// --------------------------- GCCOptions ------------------------------
char const * const GCCOptions::s_defaultPlatformObjectFileSuffix = ".o";


GCCOptions::GCCOptions()
  : m_options(),
    m_platformObjectFileSuffix(s_defaultPlatformObjectFileSuffix)
{}


GCCOptions::~GCCOptions()
{}


GCCOptions::GCCOptions(std::vector<std::string> const &words)
  : m_options(),
    m_platformObjectFileSuffix(s_defaultPlatformObjectFileSuffix)
{
  parse(words);
}


GCCOptions::Option const &GCCOptions::at(size_t index) const
{
  return m_options.at(index);
}


// Based on experimentation with GCC-9.3.0, -M has highest precedence,
// then -E, then -S, then finally -c, regardless of the order in which
// they appear.  This is reflected in the order of the OutputMode
// enumerators.
GCCOptions::OutputMode GCCOptions::outputMode() const
{
  // Mode we think this is, based on what has been seen so far.
  OutputMode curMode = OM_EXECUTABLE;

  for (Iter iter(*this); iter.hasMore(); iter.adv()) {
    Option const &o = iter.opt();

    OutputMode m = curMode;  // Initialized defensively; value not used.
    if (specifiesGCCOutputMode(o.m_name, m /*OUT*/)) {
      if (m < curMode) {
        // 'm' takes precedence.
        curMode = m;
      }
    }
  }

  return curMode;
}


bool GCCOptions::hasOption(std::string const &name) const
{
  std::string dummy;
  return getArgumentForOption(name, dummy);
}


// This function exists because it is fairly common to want to check
// for the presence of either of two options, and it would be possible
// to optimize this by making just one pass over the options, although
// for now I have not done so.
bool GCCOptions::hasEitherOption(std::string const &n1,
                                 std::string const &n2) const
{
  return hasOption(n1) || hasOption(n2);
}


bool GCCOptions::getArgumentForOption(std::string const &name,
                                      std::string &argument) const
{
  for (Iter iter(*this); iter.hasMore(); iter.adv()) {
    Option const &o = iter.opt();
    if (o.m_name == name) {
      argument = o.m_argument;
      return true;
    }
  }
  return false;
}


bool GCCOptions::getExplicitOutputFile(std::string &fname) const
{
  if (outputMode() == OM_DEPENDENCIES) {
    // With -M or -MM, the -MF option takes precedence.  (If -o is also
    // used, the named file gets created but is left empty.)
    if (getArgumentForOption("-MF", fname)) {
      return true;
    }
  }

  return getArgumentForOption("-o", fname);
}


bool GCCOptions::getFirstSourceFileName(std::string &fname) const
{
  for (Iter iter(*this); iter.hasMore(); iter.adv()) {
    Option const &opt = iter.opt();
    if (opt.isInputFile()) {
      std::string lang =
        gccLanguageForFile(opt.m_argument, iter.xLang());
      if (!lang.empty()) {
        // This is a source (not object) file.
        fname = opt.m_argument;
        return true;
      }
    }
  }

  return false;
}


bool GCCOptions::getOutputFile(std::string &fname) const
{
  if (getExplicitOutputFile(fname)) {
    return true;
  }

  OutputMode mode = outputMode();

  if (mode == OM_PREPROCESSED || mode == OM_DEPENDENCIES) {
    return false;
  }

  if (mode == OM_EXECUTABLE) {
    fname = "a.out";
    return true;
  }

  // Scan for a source file name.
  std::string srcFileName;
  if (getFirstSourceFileName(srcFileName)) {
    // Remove any directory and extension from the file name.  The GCC
    // manual does not clearly say to remove the directory, but GCC in
    // fact does that.
    SMFileUtil sfu;
    std::string srcNoExt = stripExtension(sfu.splitPathBase(srcFileName));

    // Default output name.
    fname = srcNoExt + extensionForGCCOutputMode(mode);
    return true;
  }
  else {
    // We didn't see a source file name, so can't compute the output
    // file name.
    return false;
  }
}


bool GCCOptions::createsDependencyFile(std::string &fname) const
{
  if (hasEitherOption("-MD", "-MMD")) {
    // The output file is the first of:
    //   * name given to -MF, or
    //   * name given to -o with suffix replaced with ".d", or
    //   * name of source file, without directory, and suffix replaced
    //     with ".d".

    if (getArgumentForOption("-MF", fname)) {
      return true;
    }

    std::string oname;
    if (getArgumentForOption("-o", oname)) {
      fname = stripExtension(oname) + ".d";
      return true;
    }

    std::string srcname;
    if (getFirstSourceFileName(srcname)) {
      SMFileUtil sfu;
      fname = stripExtension(sfu.splitPathBase(srcname)) + ".d";
      return true;
    }

    // We can't figure out what the name is supposed to be, so pretend
    // no dependency file will be generated.  In a case like this, GCC
    // should give an error.
    return false;
  }
  else {
    return false;
  }
}


bool GCCOptions::getDefaultDependencyTarget(std::string &target) const
{
  // We assume there is no -MT or -MQ.

  if (hasEitherOption("-MD", "-MMD")) {
    std::string ofile;
    if (getArgumentForOption("-o", ofile)) {
      // If explicitly specified, the output file is the target.
      target = ofile;
      return true;
    }
  }

  std::string fname;
  if (getFirstSourceFileName(fname)) {
    // TODO: This should be a class data member so clients can
    // influence the choice of directory separator.  There are a couple
    // other instances in this file that need the same treatment.
    SMFileUtil sfu;

    // From my spec in inst/doc/index.html: "for each source file, its
    // name, without any directory, suffix removed (if it had one), and
    // the platform object file suffix added."
    target = stripExtension(sfu.splitPathBase(fname)) +
               m_platformObjectFileSuffix;
    return true;
  }

  // No source files.
  return false;
}


int GCCOptions::numSourceFiles() const
{
  int ret = 0;
  for (Iter iter(*this); iter.hasMore(); iter.adv()) {
    if (iter.optIsSourceFile()) {
      ret++;
    }
  }
  return ret;
}


void GCCOptions::getCommandWords(
  std::vector<std::string> &commandWords) const
{
  for (Iter iter(*this); iter.hasMore(); iter.adv()) {
    iter.opt().appendWords(commandWords);
  }
}


std::string GCCOptions::toCommandLineString() const
{
  std::vector<std::string> commandWords;
  getCommandWords(commandWords);
  return join(commandWords, " ");
}


void GCCOptions::parse(std::vector<std::string> const &args)
{
  WordIterator iter(args);

  while (iter.hasMore()) {
    std::string optWord = iter.nextAdv();

    // True if 'parseOption' says it found a match.
    bool recognized = false;

    // We work through the entries in reverse order to ensure that we
    // process a longer string before its prefix.
    //
    // This is not very efficient, but my purpose, the speed of this
    // lookup is unimportant.  (And doing better is nontrivial since it
    // would need a prefix tree or similar.)
    for (int i = TABLESIZE(optionsTable) - 1; i >= 0; i--) {
      char const *name = optionsTable[i].m_name;
      GCCOptions::OptionSyntax syntax =
        (GCCOptions::OptionSyntax)(optionsTable[i].m_syntax);

      recognized = parseOption(optWord, name, syntax, iter);
      if (recognized) {
        break;
      }
    }

    if (!recognized) {
      if (optWord[0] == '-') {
        // Unrecognized option switch.  Treat it as a single-word
        // option.
        addOption(optWord, SEP_NONE, "", SYN_UNRECOGNIZED);
      }

      // TODO: An option can be prefixed with '@' to name a "response
      // file" that contains additional options.  I should open that
      // file and read its contents.

      else {
        // Non-option argument.
        addOption("", SEP_NONE, optWord);
      }
    }
  }
}


void GCCOptions::addOption(std::string const &name,
                           Separator separator,
                           std::string const &argument,
                           SyntaxError syntaxError)
{
  addOption(Option(name, separator, argument, syntaxError));
}


void GCCOptions::addOption(Option const &opt)
{
  m_options.push_back(opt);
}


void GCCOptions::addInputFile(std::string const &argument)
{
  addOption("", SEP_NONE, argument, SYN_NONE);
}


void GCCOptions::addBareOption(std::string const &name)
{
  addOption(name, SEP_NONE, "", SYN_NONE);
}


void GCCOptions::addSpaceOption(std::string const &name,
                                std::string const &argument)
{
  addOption(name, SEP_SPACE, argument, SYN_NONE);
}


void GCCOptions::addEmptyOption(std::string const &name,
                                std::string const &argument)
{
  addOption(name, SEP_EMPTY, argument, SYN_NONE);
}


bool GCCOptions::parseOption(
  std::string const &optWordString,
  char const *name,
  OptionSyntax syntax,
  WordIterator &iter)
{
  xassert(syntax != 0);
  xassert(!( (syntax & OS_BARE) && (syntax & OS_SPACE) ));

  // I'll treat it as a C string so I can use pointer arithmetic to
  // easily march through the string's contents.
  char const *optWord = optWordString.c_str();

  if (beginsWith(optWord, name)) {
    char const *after = optWord + strlen(name);

    if ((syntax & OS_EXACT) && *after != 0) {
      // OS_EXACT means we reject if 'name' is a proper prefix of
      // 'optWordString'.  Instead, we'll look for another switch that
      // does allow prefix matching.
      return false;
    }

    if (*after == '=') {
      // Should we treat the '=' as a separator?
      if (syntax & OS_EQUALS) {
        // Argument is part of the same word, after the '='.
        addOption(name, SEP_EQUALS, after+1);
        return true;
      }
      else if ((syntax & OS_SPACE) && !(syntax & OS_EMPTY)) {
        // Complain about using '=' where a separate word is
        // required.
        addOption(name, SEP_EQUALS, after+1, SYN_INVALID_EQUALS);
        return true;
      }
    }

    if (*after == 0) {
      if (syntax & OS_SPACE) {
        // Argument is a separate word.
        if (iter.hasMore()) {
          addOption(name, SEP_SPACE, iter.nextAdv());
        }
        else {
          // But there is not a following word.
          addOption(name, SEP_NONE, "", SYN_ABRUPT_END);
        }
      }
      else if (syntax & OS_BARE) {
        // Argument is one word.
        addOption(name, SEP_NONE, "");
      }
      else if (syntax & OS_EQUALS) {
        // An '=' is required.
        addOption(name, SEP_NONE, "", SYN_MISSING_EQUALS);
      }
      else if (syntax & OS_EMPTY) {
        // An argument is required.
        addOption(name, SEP_NONE, "", SYN_MISSING_ARGUMENT);
      }
      else {
        xfailure("no syntax bits?");
      }
    }

    else /*empty-string separator*/ {
      if (syntax & OS_EMPTY) {
        // Argument is part of the same word.
        addOption(name, SEP_EMPTY, after);
      }
      else if (syntax & OS_EQUALS) {
        // An '=' is required.
        addOption(name, SEP_EMPTY, after, SYN_MISSING_EQUALS);
      }
      else if (syntax & OS_SPACE) {
        // An argument is allowed, but there is no separator.  Treat
        // 'after' as the argument but flag it as invalid.
        addOption(name, SEP_EMPTY, after, SYN_MISSING_SEPARATOR);
      }
      else /*OS_BARE*/ {
        // What comes after the option name is junk.
        addOption(name, SEP_EMPTY, after, SYN_TRAILING_JUNK);
      }
    }

    return true;
  }

  return false;
}


// ------------------------ Global functions ---------------------------
// Set of legal arguments to the "-x" option.
static char const * const xLanguageValues[] = {
  // Sorted: LANG=C sort
  "ada",
  "assembler",
  "assembler-with-cpp",
  "c",
  "c++",
  "c++-cpp-output",
  "c++-header",
  "c++-system-header",
  "c++-user-header",
  "c-header",
  "cpp-output",
  "d",
  "f77",
  "f77-cpp-input",
  "f95",
  "f95-cpp-input",
  "go",
  "objective-c",
  "objective-c++",
  "objective-c++-cpp-output",
  "objective-c++-header",
  "objective-c-cpp-output",
  "objective-c-header",
};


static bool isValidGCCLanguage(char const *lang)
{
  return stringInSortedArray(lang, xLanguageValues,
                             TABLESIZE(xLanguageValues));
}


// Extensions that map to a different language code.
static struct ExtensionMapEntry {
  // Extension that a file might have.
  char const *m_ext;

  // Language code to use for that extension.
  char const *m_lang;
}
const extensionMap[] = {
  // Sorted: LANG=C sort
  { "C",      "c++" },
  { "CPP",    "c++" },
  { "F",      "f77-cpp-input" },
  { "F03",    "f95-cpp-input" },
  { "F08",    "f95-cpp-input" },
  { "F90",    "f95-cpp-input" },
  { "F95",    "f95-cpp-input" },
  { "FOR",    "f77-cpp-input" },
  { "FPP",    "f77-cpp-input" },
  { "FTN",    "f77-cpp-input" },
  { "H",      "c++-header" },
  { "HPP",    "c++-header" },
  { "M",      "objective-c++" },
  { "S",      "assembler-with-cpp" },
  { "adb",    "ada" },
  { "ads",    "ada" },
  { "c",      "c" },
  { "c++",    "c++" },
  { "cc",     "c++" },
  { "cp",     "c++" },
  { "cpp",    "c++" },
  { "cxx",    "c++" },
  { "d",      "d" },
  { "dd",     "d" },
  { "di",     "d" },
  { "f",      "f77" },
  { "f03",    "f95" },
  { "f08",    "f95" },
  { "f90",    "f95" },
  { "f95",    "f95" },
  { "for",    "f77" },
  { "fpp",    "f77-cpp-input" },
  { "ftn",    "f77" },
  { "go",     "go" },
  { "h",      "c-header" },
  { "h++",    "c++-header" },
  { "hh",     "c++-header" },
  { "hp",     "c++-header" },
  { "hpp",    "c++-header" },
  { "hxx",    "c++-header" },
  { "i",      "cpp-output" },
  { "ii",     "c++-cpp-output" },
  { "m",      "objective-c" },
  { "mi",     "objective-c-cpp-output" },
  { "mii",    "objective-c++-cpp-output" },
  { "mm",     "objective-c++" },
  { "s",      "assembler" },
  { "sx",     "assembler-with-cpp" },
  { "tcc",    "c++-header" },
};


static void validateExtensionLanguages()
{
  for (auto entry : extensionMap) {
    xassert(isValidGCCLanguage(entry.m_lang));
  }
}


std::string gccLanguageForFile(std::string const &fname,
                               std::string const &xLang)
{
  if (!xLang.empty()) {
    return xLang;
  }

  // Get the extension.
  char const *dot = strrchr(fname.c_str(), '.');
  if (!dot) {
    return "";
  }
  char const *ext = dot+1;

  // Look up the extension.
  ExtensionMapEntry key = { ext, "" };
  auto compare =
    [](ExtensionMapEntry const &a, ExtensionMapEntry const &b)
    {
      return strcmpCompare(a.m_ext, b.m_ext);
    };
  ExtensionMapEntry const *end = ARRAY_ENDPTR(extensionMap);

  xassert_once(is_strictly_sorted(extensionMap, end, compare));

  ExtensionMapEntry const *entry =
    binary_lookup(extensionMap, end, key, compare);
  if (entry != end) {
    return entry->m_lang;
  }

  return "";
}


#define OM(name) GCCOptions::OM_##name

bool specifiesGCCOutputMode(std::string const &name,
                            GCCOptions::OutputMode &mode)
{
  struct Entry {
    char const *m_name;
    GCCOptions::OutputMode m_mode;
  };
  static Entry const table[] = {
    // Sorted columns: \{ \S+ @30:\S+ \}
    { "-E",                  OM(PREPROCESSED) },
    { "-M",                  OM(DEPENDENCIES) },
    { "-MM",                 OM(DEPENDENCIES) },
    { "-S",                  OM(ASSEMBLY) },
    { "-c",                  OM(OBJECT_CODE) },
    { "-dumpfullversion",    OM(GCC_INFO) },
    { "-dumpmachine",        OM(GCC_INFO) },
    { "-dumpspecs",          OM(GCC_INFO) },
    { "-dumpversion",        OM(GCC_INFO) },
  };

  Entry key = { name.c_str(), OM(GCC_INFO)/*irrelevant*/ };
  auto compare =
    [](Entry const &a, Entry const &b)
    {
      return strcmpCompare(a.m_name, b.m_name);
    };
  Entry const *end = ARRAY_ENDPTR(table);

  xassert_once(is_strictly_sorted(table, end, compare));

  Entry const *found =
    binary_lookup(table, end, key, compare);
  if (found != end) {
    mode = found->m_mode;
    return true;
  }
  else {
    return false;
  }
}


void gcc_options_check_tables()
{
  validateExtensionLanguages();
  xassert(isStrictlySortedStringArray(xLanguageValues,
                                      TABLESIZE(xLanguageValues)));
}


// EOF
