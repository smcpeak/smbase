// gcc-options.cc
// Code for gcc-options.h.

#include "gcc-options.h"               // this module

#include "binary-lookup.h"             // binary_lookup
#include "container-utils.h"           // insertUnique
#include "sm-file-util.h"              // SMFileUtil
#include "sm-macros.h"                 // EMEMB, DEFINE_ENUMERATION_TO_STRING
#include "strcmp-compare.h"            // StrcmpCompare, etc.
#include "strictly-sorted.h"           // is_strictly_sorted
#include "string-utils.h"              // stripExtension, stringInSortedArray, join
#include "strutil.h"                   // prefixEquals
#include "xassert.h"                   // xassert

#include <algorithm>                   // std::sort
#include <map>                         // std::map
#include <sstream>                     // std::ostringstream

#include <string.h>                    // strrchr


// ----------------------- Option categories ---------------------------
// These categories are determined in part from the GCC manual, but also
// from experimentation with GCC-9.3.0.  In theory, based on my reading
// of the manual, this is exhaustive(!).
//
// The names are meant to be kept in "LANG=C sort" order within their
// group.
//
// NOTE: The first four groups' syntax means they apply to any option
// that begins with that name, whereas the rest require some sort of
// separator if they accept an argument.

// TODO: I should make these into one table, but I need to write a
// script first to help with the formatting.


// OS_EMPTY
static char const * const emptyArgOptions[] = {
  "-W",
  "-d",
  "-f",
  "-m",
  "-no",
  "-print-",
};


// OS_EMPTY | OS_SPACE
static char const * const emptyOrSpaceArgOptions[] = {
  "-A",
  "-B",            // '='?
  "-I",            // GCC does *not* accept '=' for this one (it is treated as part of the directory name).
  "-L",            // Not sure about '=' here.
  "-MF",
  "-MQ",
  "-MT",
  "-T",
  "-U",            // GCC rejects '=' on this one (treats it as part of the symbol, then chokes).
  "-e",
  "-idirafter",
  "-imacros",
  "-imulitilib",
  "-include",      // Yes, "-includesome_file" works.
  "-iprefix",
  "-iquote",
  "-isysroot",
  "-isystem",
  "-iwithprefix",
  "-iwithprefixbefore",
  "-o",
  "-u",            // I'm not sure how GCC interprets '=', but my guess is it's treated as part of the symbol.
  "-x",
  "-z",
};


// OS_EMPTY | OS_BARE
static char const * const optionalEmptyArgSwitches[] = {
  "-O",
  "-g",
  "-shared",
  "-static",
};


// OS_EMPTY | OS_SPACE | OS_EQUALS
//
// Nothing is documented as accepting all three of these, but I
// determined through experimentation that some options do, and do *not*
// fold the '=' into the argument (meaning OS_EQUALS takes precedence
// over OS_EMPTY).
//
static char const * const emptyOrSpaceOrEqualsArgOptions[] = {
  "-D",
  "-l",
};


// OS_EQUALS | OR_SPACE
static char const * const equalsOrSpaceArgOptions[] = {
  // For this group, the manual only says ' ', but gcc accepts '='.
  "--param",
  "-aux-info",

  // For these, the manual only says '=', but gcc accepts ' '.
  "--entry",
  "-specs",
};


// OS_SPACE | OS_EXACT
static char const * const exactSpaceArgOptions[] = {
  "-dumpbase",     // Note ambiguity with "-d".
  "-dumpbase-ext",
  "-dumpdir",
};


// OS_SPACE
static char const * const spaceArgOptions[] = {
  "-Xassembler",
  "-Xlinker",
  "-Xpreprocessor",
  "-wrapper",
};


// OS_EQUALS
static char const * const equalsArgOptions[] = {
  // For these, I confirmed that '=' is required and ' ' is rejected.
  "-iplugindir",
  "-std",

  // These are documented as accepting '=' and I have not tested if they
  // also accept ' '.
  "--sysroot",
  "-stdlib",
};


// OS_BARE
static char const * const noArgSwitches[] = {
  "-###",
  "--coverage",
  "--target-help",
  "--version",
  "-C",
  "-CC",
  "-E",
  "-H",
  "-M",
  "-MD",
  "-MG",
  "-MM",
  "-MMD",
  "-MP",
  "-Mno-modules",  // My GCC-9.3.0 says this is unrecognized...
  "-P",
  "-Q",
  "-S",
  "-ansi",
  "-c",
  "-gen-decls",    // Note: "-g" is a prefix, creating an ambiguity.
  "-p",
  "-pass-exit-codes",
  "-pedantic",
  "-pedantic-errors",
  "-pg",
  "-pie",
  "-pipe",
  "-print-objc-runtime-info",
  "-pthread",
  "-r",
  "-rdynamic",
  "-remap",
  "-s",
  "-symbolic",
  "-traditional",
  "-traditional-cpp",
  "-trigraphs",
  "-undef",        // Note ambiguity with "-u".
  "-v",
  "-w",
};


// OS_BARE | OS_EXACT
static char const * const exactNoArgOptions[] = {
  "-dumpfullversion",
  "-dumpmachine",
  "-dumpspecs",
  "-dumpversion",
};


// OS_BARE | OS_EQUALS
static char const * const optionalEqualsArgSwitches[] = {
  "--help",
};


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
    "OM_MAKE_RULE",
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
GCCOptions::GCCOptions()
  : m_options()
{}


GCCOptions::~GCCOptions()
{}


GCCOptions::GCCOptions(std::vector<std::string> const &words)
  : m_options()
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
  if (outputMode() == OM_MAKE_RULE) {
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

  if (mode == OM_PREPROCESSED || mode == OM_MAKE_RULE) {
    return false;
  }

  if (mode == OM_EXECUTABLE) {
    fname = "a.out";
    return true;
  }

  // Scan for a source file name.
  std::string srcFileName;
  if (getFirstSourceFileName(srcFileName)) {
    // Remove any extension from the file name.
    std::string srcNoExt = stripExtension(srcFileName);

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
  if (hasOption("-MD") || hasOption("-MMD")) {
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
  // Map from option name (pointer) to its syntax style.
  //
  // This gets built every time we parse an argument list.  I could
  // save the map, but in most cases we only parse one list per
  // process anyway.
  //
  // The entries are sorted in reverse option order name so that when
  // one option name is a prefix of another, we try the prefix second.
  std::map<char const *, OptionSyntax, StrcmpRevCompare> optionToSyntax;


  // Build the map.
  #define PROCESS_OPTION_GROUP(array, syntax)          \
    for (char const *name : array) {                   \
      insertUnique(optionToSyntax,                     \
        std::make_pair(name, (OptionSyntax)(syntax))); \
    }

  // The first four have OS_EMPTY, which is the possibility that gives
  // rise to ambiguities, so must be treated with care.
  PROCESS_OPTION_GROUP(emptyArgOptions,                OS_EMPTY);
  PROCESS_OPTION_GROUP(emptyOrSpaceArgOptions,         OS_EMPTY | OS_SPACE);
  PROCESS_OPTION_GROUP(optionalEmptyArgSwitches,       OS_EMPTY | OS_BARE);
  PROCESS_OPTION_GROUP(emptyOrSpaceOrEqualsArgOptions, OS_EMPTY | OS_SPACE | OS_EQUALS);

  PROCESS_OPTION_GROUP(equalsOrSpaceArgOptions,        OS_EQUALS | OS_SPACE);
  PROCESS_OPTION_GROUP(exactSpaceArgOptions,           OS_SPACE | OS_EXACT);
  PROCESS_OPTION_GROUP(spaceArgOptions,                OS_SPACE);
  PROCESS_OPTION_GROUP(equalsArgOptions,               OS_EQUALS);
  PROCESS_OPTION_GROUP(noArgSwitches,                  OS_BARE);
  PROCESS_OPTION_GROUP(exactNoArgOptions,              OS_BARE | OS_EXACT);
  PROCESS_OPTION_GROUP(optionalEqualsArgSwitches,      OS_BARE | OS_EQUALS);

  #undef PROCESS_OPTION_GROUP


  WordIterator iter(args);

  while (iter.hasMore()) {
    std::string optWord = iter.nextAdv();

    // True if 'parseOption' says it found a match.
    bool recognized = false;

    // TODO: I could use 'lower_bound' to find the relevant entries
    // faster than iterating through all of them.
    for (auto pr : optionToSyntax) {
      char const *name = pr.first;
      OptionSyntax syntax = pr.second;

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

  if (prefixEquals(optWord, name)) {
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


// TODO: I don't think I need this function.  Remove it.
void GCCOptions::ensureExplicitOutputFile()
{
  std::string fn;
  if (getExplicitOutputFile(fn)) {
    return;
  }

  // Compute the default.
  if (getOutputFile(fn)) {
    // Specify it as an option.
    addSpaceOption("-o", fn);
  }
  else {
    // Didn't come up with a name.  Oh well.
  }
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
    { "-M",                  OM(MAKE_RULE) },
    { "-MM",                 OM(MAKE_RULE) },
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
