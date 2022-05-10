// gcc-options.cc
// Code for gcc-options.h.

#include "gcc-options.h"               // this module

#include "container-utils.h"           // insertUnique
#include "sm-macros.h"                 // EMEMB
#include "strutil.h"                   // prefixEquals
#include "xassert.h"                   // xassert

#include <algorithm>                   // std::sort
#include <map>                         // std::map
#include <sstream>                     // std::ostringstream

#include <string.h>                    // strcmp


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


// OS_EMPTY
static char const * const emptyArgOptions[] = {
  "-W",
  "-d",            // Several "-dumpXXX" options are lumped in here.
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
  "-dumpbase",     // Note ambiguity with "-d".
  "-dumpbase-ext",
  "-dumpdir",

  // For these, the manual only says '=', but gcc accepts ' '.
  "--entry",
  "-specs",
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
  "-Mno-modules",
  "-P",
  "-Q",
  "-S",
  "-ansi",
  "-c",
  "-gen-decls",  // Note: "-g" is a prefix, creating an ambiguity.
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
char const *toString(GCCOptions::Separator separator)
{
  char const * const names[] = {
    "SEP_NONE",
    "SEP_EMPTY",
    "SEP_SPACE",
    "SEP_EQUALS",
  };

  STATIC_ASSERT(TABLESIZE(names) == GCCOptions::NUM_SEPARATORS);
  if ((unsigned)separator < GCCOptions::NUM_SEPARATORS) {
    return names[separator];
  }
  else {
    return "(invalid separator code)";
  }
}



// -------------------------- SyntaxError ------------------------------
char const *toString(GCCOptions::SyntaxError syntaxError)
{
  char const * const names[] = {
    "SYN_NONE",
    "SYN_ABRUPT_END",
    "SYN_UNRECOGNIZED",
    "SYN_TRAILING_JUNK",
    "SYN_MISSING_SEPARATOR",
    "SYN_MISSING_EQUALS",
    "SYN_MISSING_ARGUMENT",
    "SYN_INVALID_EQUALS",
  };

  STATIC_ASSERT(TABLESIZE(names) == GCCOptions::NUM_SYNTAX_ERRORS);
  if ((unsigned)syntaxError < GCCOptions::NUM_SYNTAX_ERRORS) {
    return names[syntaxError];
  }
  else {
    return "(invalid syntax error code)";
  }
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


void GCCOptions::Option::appendWords(std::vector<std::string> &dest)
{
  switch (m_separator) {
    default:
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


// --------------------------- GCCOptions ------------------------------
static bool strcmpRevCompare(char const *a, char const *b)
{
  // Reverse of the usual order.
  return strcmp(a,b) > 0;
}

class StrcmpRevCompare {
public:
  bool operator() (char const *a, char const *b) {
    return strcmpRevCompare(a, b);
  }
};


GCCOptions::GCCOptions(std::vector<std::string> const &args)
  : m_options()
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
  PROCESS_OPTION_GROUP(spaceArgOptions,                OS_SPACE);
  PROCESS_OPTION_GROUP(equalsArgOptions,               OS_EQUALS);
  PROCESS_OPTION_GROUP(noArgSwitches,                  OS_BARE);
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


GCCOptions::~GCCOptions()
{}


void GCCOptions::addOption(std::string const &name,
                           Separator separator,
                           std::string const &argument,
                           SyntaxError syntaxError)
{
  m_options.push_back(Option(name, separator, argument, syntaxError));
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


// EOF
