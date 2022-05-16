// gcc-options.h
// GCCOptions class.

#ifndef GCC_OPTIONS_H
#define GCC_OPTIONS_H

#include <iosfwd>                      // std::ostream
#include <string>                      // std::string
#include <vector>                      // std::vector


// Parse a sequence of command line options according to the GCC
// command line syntax.
//
// My primary goal here is to distinguish option names, option
// arguments, and input file names, such that one could reliably scan
// the result for certain options of interest, perhaps make changes,
// then use that as a command line.
//
// Therefore, I try to only make distinctions where necessary for
// parsing the overall structure.  For example, all of the "-f" options
// are grouped together under a single option, and a client would have
// to inspect its 'm_argument' to make further distinctions.  But
// options that take arguments (like "-o") are distinguished from those
// that do not (like "-c").
//
// Terminology: In this class, "word" means an elements of 'args'.
//
class GCCOptions {
private:     // types
  // Encapsulate the sequence of words and an iterator to them.
  //
  // Perhaps this should be generalized to an iterator pair (begin and
  // end) that works with any container?
  //
  class WordIterator {
  private:     // data
    // Sequence of words.
    std::vector<std::string> const &m_args;

    // Index of next element in 'm_argv' to process.
    size_t m_index;

  public:
    WordIterator(std::vector<std::string> const &args);

    // True if there are more words to process.
    bool hasMore() const
      { return m_index < m_args.size(); }

    // Retrieve one word and advance to the next.
    std::string nextAdv();
  };

public:      // types
  // Various kinds of option syntax that a given option can accept.
  // Every option has some subset of these as possibilities.
  enum OptionSyntax {
    // The option can be passed as its name alone with no argument (it
    // does not consume the next word).  This is incompatible with
    // OS_SPACE.  Example: "-c".
    OS_BARE                  = 0x0001,

    // The option can be passed as its name as one word, then an
    // argument as the next word (presumably using a space on a command
    // line string to separate them).  Example: "-I incdir".
    OS_SPACE                 = 0x0002,

    // The option can be passed as its name, followed by more text, all
    // as part of a single word.  Example: "-Iincdir".  The name "empty"
    // is due to using the empty string as a separator.
    OS_EMPTY                 = 0x0004,

    // The option can be passed as its name, then '=', then an argument,
    // all as one word.  Example: "-std=c99".  When this happens, the
    // '=' is include in the option name nor the argument.
    OS_EQUALS                = 0x0008,

    // The option is only recognized when the name is an exact match,
    // not merely a prefix of what appeared on the command line.  This
    // a syntactic restriction rather than a possibility, so must be
    // combined with at least one other bit.
    OS_EXACT                 = 0x0010,
  };

  // Syntax of the separator between the option name and its argument.
  enum Separator {
    // No separator because only one word was present.
    SEP_NONE,

    // Empty string separator, like "-DFOO".
    SEP_EMPTY,

    // Whitespace separator, meaning separate words, like "-o filename".
    SEP_SPACE,

    // Equals character, like "-include=some_dir".
    SEP_EQUALS,

    NUM_SEPARATORS
  };

  // Optional syntax error in an option.
  enum SyntaxError {
    // No error.
    SYN_NONE,

    // There is no separator because we saw an option that accepts an
    // argument, but it was the last word.
    SYN_ABRUPT_END,

    // The option name begins with a hyphen, but we do not recognize
    // what follows, so the entire text has been put into the option
    // name, with no argument.
    SYN_UNRECOGNIZED,

    // We recognize the option name, but it does not accept any argument
    // (for example, "-c") and there was text after the option name.
    // The extra text has been put into the argument string.
    SYN_TRAILING_JUNK,

    // The separator was empty but that is invalid.  For example,
    // "-dumpbase=FOO" and "-dumpbase FOO" are valid but "-dumpbaseFOO"
    // is not.
    SYN_MISSING_SEPARATOR,

    // The option requires an argument after '=', but the '=' was
    // missing.  If the separator is SEP_NONE, then the option just
    // ended without specifying any argument, whereas if it is
    // SEP_EMPTY, then there was text after the option name (but no '='
    // where it should be).
    SYN_MISSING_EQUALS,

    // An argument is required directly after the option name, as part
    // of the same word, but none was present.
    SYN_MISSING_ARGUMENT,

    // An '=' was used to separate option from argument but GCC requires
    // the argument to be its own word.  Example: "-wrapper".
    SYN_INVALID_EQUALS,

    NUM_SYNTAX_ERRORS
  };

  // Possible types of output a GCC command line can specify.  These
  // are in order of decreasing precedence.
  enum OutputMode {
    OM_GCC_INFO,             // -dumpversion, etc.  GCC stops after printing.
    OM_DEPENDENCIES,         // -M and -MM
    OM_PREPROCESSED,         // -E
    OM_ASSEMBLY,             // -S
    OM_OBJECT_CODE,          // -c
    OM_EXECUTABLE,           // No switch.

    NUM_OUTPUT_MODES
  };

  // Represent a single parsed conceptual option, which might have been
  // presented as multiple words.
  class Option {
  public:      // data
    // The option name, beginning with a hyphen (like "-I"), or the
    // empty string if this option is a stand-alone argument like the
    // name of an input file.
    std::string m_name;

    // How the argument was separated from the name.  If this is
    // SEP_NONE, then exactly one of 'm_name' or 'm_argument' is empty.
    Separator m_separator;

    // Argument to the option, or empty string if there was no argument.
    std::string m_argument;

    // Possible syntax error.
    SyntaxError m_syntaxError;

  public:      // methods
    Option(std::string const &name, Separator separator,
           std::string const &argument, SyntaxError syntaxError);
    ~Option();

    // Compiler-generated copy constructor and assignment are fine.

    bool operator== (Option const &obj) const;
    bool operator!= (Option const &obj) const
      { return !operator==(obj); }

    // Print for debug purposes.
    std::ostream& insert(std::ostream &os) const;
    std::string toString() const;

    // Append to 'dest' command line words that reproduce the syntax
    // originally used to specify this option.  In general, it should be
    // the case that appending all parsed options yields exactly the
    // original sequence of words.  (However, once response files are
    // implemented, that won't be true anymore.)
    void appendWords(std::vector<std::string> &dest) const;

    // True if this option represents the name of an input file.
    bool isInputFile() const
      { return m_separator==SEP_NONE && !m_argument.empty(); }
  };

  // Class to iterate over the Options in a GCCOptions object.
  //
  // In addition to tracking position within the sequence, it tracks
  // stateful settings, particularly "-x", which once set affect the
  // interpretation of later options, and can be specified multiple
  // times.
  //
  class Iter {
  private:     // data
    // Option sequence we are iterating over.
    GCCOptions const &m_options;

    // Current option index.  Equals 'm_options.size()' when
    // '!hasMore()'.
    size_t m_index;

    // Current "-x" setting, or the empty string if none is set, which
    // is the case initially and after "-xnone".
    std::string m_xLang;

  private:     // methods
    // Having just advanced, update internal state based on the current
    // option.
    void updateState();

  public:      // methods
    Iter(GCCOptions const &options);
    ~Iter();

    // Current Option.  Requires 'hasMore()'.
    Option const &opt() const;

    size_t index() const
      { return m_index; }

    std::string xLang() const
      { return m_xLang; }

    // True if there are more options to iterate over, including the
    // current one.
    bool hasMore() const;

    // Advance to the next option.  Requires 'hasMore()'.
    void adv();
  };

public:      // class data
  // Always ".o".
  static char const * const s_defaultPlatformObjectFileSuffix;

private:     // instance data
  // Sequence of parsed options, in the order they appeared in the
  // input.  The number of elements here is usually less than the size
  // of 'args' passed to the constructor because multiple words can be
  // represented with one Option.
  std::vector<Option> m_options;

public:      // data
  // Certain operations, such as computing the default dependency rule
  // target name, are specified by the GCC manual to use the "platform
  // object file suffix".  In those situations, this field is used as
  // that value.  It is initially 's_defaultPlatformObjectFileSuffix',
  // and this class does not change its value.  The client can change it
  // at will.
  std::string m_platformObjectFileSuffix;

private:     // methods
  // Try to parse 'optWord' as an instance of option 'name', which uses
  // 'syntax'.  If it needs another word, get it from 'iter'.  Return
  // true if the option is recognized as an instance of 'name' and
  // processed accordingly.
  bool parseOption(
    std::string const &optWord,
    char const *name,
    OptionSyntax syntax,
    WordIterator &iter);

public:      // methods
  GCCOptions();
  ~GCCOptions();

  // Compiler-generated copy constructor and assignment are fine.

  // Parse 'words' using 'parse' method.
  explicit GCCOptions(std::vector<std::string> const &words);

  bool operator== (GCCOptions const &obj) const
    { return m_options == obj.m_options; }
  bool operator!= (GCCOptions const &obj) const
    { return !operator==(obj); }

  // Number of parsed options.
  size_t size() const
    { return m_options.size(); }

  bool empty() const
    { return m_options.empty(); }

  // Get one option.
  Option const &at(size_t index) const;
  Option const &operator[](size_t index) const
    { return at(index); }

  // Get entire vector.
  std::vector<Option> const &getOptions() const
    { return m_options; }

  // Get the output mode specified on this command line.
  OutputMode outputMode() const;

  // Return true if an option with 'name' is present.
  bool hasOption(std::string const &name) const;

  // True if 'hasOption(n1) || hasOption(n2)'.
  bool hasEitherOption(std::string const &n1,
                       std::string const &n2) const;

  // If there is an option with 'name', yield its argument in 'argument'
  // and return true.  Otherwise return false.
  bool getArgumentForOption(std::string const &name,
                            std::string &argument) const;

  // If there is an output file explicitly specified with "-o", or with
  // "-MF" in OM_DEPENDENCIES, put its argument into 'fname'.  Otherwise
  // return false.
  bool getExplicitOutputFile(std::string &fname) const;

  // If there is at least one source file listed on the command line,
  // return true and set 'fname' to the first such.
  bool getFirstSourceFileName(std::string &fname) const;

  // If an output file is explicitly specified, put that into 'fname'
  // and return true.  Otherwise, compute the default output file,
  // taking into account the current output mode, store it in 'fname'
  // and return true.  If no output file is specified and the mode is
  // OM_PREPROCESSED or OM_DEPENDENCIES, return false, signifying
  // standard output.  In assembly or object code mode, if the command
  // line does not have any source file name, then the compiler will not
  // do anything (aside from print a warning), and we return false.
  //
  // TODO: This interface is flawed, since there can be multiple source
  // files on the command line even with -c or -S, and consequently
  // multiple primary output files.
  bool getOutputFile(std::string &fname) const;

  // If the command contains -MD or -MMD, return true, and set 'fname'
  // to the name of the dependency file it creates (which could be
  // specified explicitly or -MF or implicitly in a couple ways).
  //
  // If we can't figure out what the file would be named (because the
  // command line seems invalid), return false even if -MD/-MMD are
  // present.
  bool createsDependencyFile(std::string &fname) const;

  // Compute the default name for a dependency rule target.  This
  // assumes there is no -MT or -MQ option.  Return false if we cannot
  // compute it; one reason for that would be because there are no
  // source files on the command line.
  //
  // TODO: This interface assumes there are not multiple source files.
  bool getDefaultDependencyTarget(std::string &target) const;

  // Get the sequence of command words.
  void getCommandWords(std::vector<std::string> &commandWords) const;

  // Return a space-separated string with all the command words.  This
  // is meant for error messages or the like, as the words are not
  // quoted, so there can be ambiguity.
  std::string toCommandLineString() const;

  // Parse 'args' as GCC options and append them to the options
  // sequence.  The name of the compiler itself is *not* among these
  // elements.
  //
  // This does not throw any exceptions.  Instead, issues with
  // apparently malformed input are conveyed using the SyntaxError
  // codes in the returned Option objects.
  void parse(std::vector<std::string> const &args);

  // Add an element to 'm_options'.
  void addOption(std::string const &name,
                 Separator separator,
                 std::string const &argument,
                 SyntaxError syntaxError = SYN_NONE);
  void addOption(Option const &opt);

  // Add an option that is just 'argument'.
  void addInputFile(std::string const &argument);

  // Add an option that is just 'name'.
  void addBareOption(std::string const &name);

  // Add 'name' 'argument' as if they were two consecutive words.
  void addSpaceOption(std::string const &name,
                      std::string const &argument);
};


inline std::ostream& operator<< (std::ostream &os, GCCOptions::Option const &obj)
{
  return obj.insert(os);
}


// Returns "SEP_NONE", etc.
char const *toString(GCCOptions::Separator separator);


// Returns "SYN_NONE", etc.
char const *toString(GCCOptions::SyntaxError syntaxError);


// Returns "OM_PREPROCESSED", etc.
char const *toString(GCCOptions::OutputMode outputMode);


// Returns ".o" for OM_OBJECT_CODE, etc.
char const *extensionForGCCOutputMode(GCCOptions::OutputMode outputMode);


// If 'xLang' is the empty string, apply GCC's file name heuristics to
// 'fname' to deduce its language, yielding a string that could be the
// argument to the "-x" switch.  Otherwise, return 'xLang'.  If the
// return value is empty, it means that 'xLang' was empty and the
// extension (if any) of 'fname' was not recognized, which GCC
// interprets as being something for the linker (an object file or
// library archive).
std::string gccLanguageForFile(std::string const &fname,
                               std::string const &xLang);



// True if 'name' is among those that specify the gcc output mode,
// namely, "-c", "-E", "-S", "-M", "-MM", or a flag like "-dumpversion".
// If so, set 'mode' to the mode it specifies.
bool specifiesGCCOutputMode(std::string const &name,
                            GCCOptions::OutputMode /*OUT*/ &mode);


// For use in the unit tests, check consistency of the internal tables.
void gcc_options_check_tables();


#endif // GCC_OPTIONS_H
