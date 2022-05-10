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
  };

public:      // types
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
    void appendWords(std::vector<std::string> &dest);
  };

public:      // data
  // Sequence of parsed options, in the order they appeared in the
  // input.  The number of elements here is usually less than the size
  // of 'args' passed to the constructor because multiple words can be
  // represented with one Option.
  std::vector<Option> m_options;

private:     // methods
  // Add an element to 'm_options'.
  void addOption(std::string const &name,
                 Separator separator,
                 std::string const &argument,
                 SyntaxError syntaxError = SYN_NONE);

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
  // Parse 'args' as GCC options.  The name of the compiler itself is
  // *not* among these elements.
  //
  // This does not throw any exceptions.  Instead, issues with
  // apparently malformed input are conveyed using the SyntaxError
  // codes in the returned Option objects.
  GCCOptions(std::vector<std::string> const &args);

  ~GCCOptions();

  // Compiler-generated copy constructor and assignment are fine.
};


inline std::ostream& operator<< (std::ostream &os, GCCOptions::Option const &obj)
{
  return obj.insert(os);
}


// Returns "SEP_NONE", etc.
char const *toString(GCCOptions::Separator separator);


// Returns "SYN_NONE", etc.
char const *toString(GCCOptions::SyntaxError syntaxError);


#endif // GCC_OPTIONS_H
