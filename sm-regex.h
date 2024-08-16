// sm-regex.h
// Regular expression operations, as a wrapper around `std::regex`.

/*

The wrapper improves on `std::regex` in these ways:

  * It throws exceptions that derive from `smbase::XBase`, so can have
    context information, including the regex itself for the case of a
    regex syntax error.

  * It has lightweight #include dependencies, in contrast to `<regex>`,
    which (for GCC 9.3) pulls in 82k LOC!  (This header still pulls in
    `<string>`, which is 22k, but that is likely to already be a
    dependency of any module using regular expressions.)

  * It provides an interface that I prefer, using methods instead of
    global functions for searching, etc.

  * It provides an opportunity to do my own syntax checking before
    passing the regex to the standard library.  The latter unfortunately
    has nonportable variations in allowed syntax.  However, I do not
    currently do this extra checking.

Note that `std::regex` does not have good performance; its primary
virtue is portability.  Thus, this wrapper also does not have good
performance.

*/

#ifndef SMBASE_SM_REGEX_H
#define SMBASE_SM_REGEX_H

#include "exc.h"                       // smbase::XBase
#include "sm-macros.h"                 // OPEN_NAMESPACE, NO_OBJECT_COPIES

#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// Exception thrown for a regex syntax error.
class XRegexSyntaxError : public smbase::XBase {
public:      // data
  // The original, uncompiled regular expression.
  std::string m_regex;

  // The implementation-specific error message describing the problem.
  std::string m_errorMessage;

public:
  virtual ~XRegexSyntaxError();

  explicit XRegexSyntaxError(std::string const &regex,
                             std::string const &errorMessage);

  // XBase methods.
  virtual std::string getConflict() const override;
};


// Compiled regex pattern.
class Regex {
  // For simplicity, no copies.
  NO_OBJECT_COPIES(Regex);

private:     // data
  // The original, uncompiled regular expression.
  std::string m_orig_regex;

  // Pointer to compiled `std::regex` allocated with `new`.
  void *m_compiled_regex;

public:      // funcs
  ~Regex();

  // Create a regex from a "Modified ECMAScript regular expression
  // grammar":
  //
  //   https://en.cppreference.com/w/cpp/regex/ecmascript
  //
  // If `regex` has a syntax error, throws `XRegexSyntaxError`.
  //
  explicit Regex(std::string const &regex);

  // Get the original, uncompiled regex.
  std::string const &getOrigRegex() const
    { return m_orig_regex; }

  // True if the regex matches a substring of `str`.
  bool search(std::string const &str) const;

  // Within `str`, replace occurrences that match this Regex with
  // `replacement`, and return the substituted result.
  std::string replaceAll(std::string const &str,
                         std::string const &replacement) const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_REGEX_H
