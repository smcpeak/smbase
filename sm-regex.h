// sm-regex.h
// Regular expression operations, as a wrapper around `std::regex`.

/*

The wrapper improves on `std::regex` in these ways:

  * It throws exceptions that derive from `smbase::XBase`, so can have
    context information, including the regex itself for the case of a
    regex syntax error.

  * It has lighterweight #include dependencies compared to `<regex>`,
    which (for GCC 9.3) pulls in 82k LOC!

  * It provides an interface that I prefer, using methods instead of
    global functions for searching, etc.

  * It provides an opportunity to do my own syntax checking before
    passing the regex to the standard library.  The latter unfortunately
    has nonportable variations in allowed syntax.  However, I do not
    currently do this extra checking.

  * It suppresses resource exhaustion exceptions arising during regex
    searching, treating them as failures to match.  I think it is
    unreasonable to handle such problems at the execution site.  Any
    resource exhaustion exception is adjacent to a performance problem,
    and performance problems are better handled holistically.

Note that `std::regex` does not have good performance; its primary
virtue is portability.  Thus, this wrapper also does not have good
performance.

This wrapper currently only exposes a subset of the `std::regex`
functionality.  My plan is to expand it if and when needed.

*/

#ifndef SMBASE_SM_REGEX_H
#define SMBASE_SM_REGEX_H

#include "sm-regex-fwd.h"              // fwds for this module

#include "smbase/exc.h"                // smbase::XBase
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, NO_OBJECT_COPIES

#include <cstddef>                     // std::size_t
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Exception thrown for a regex syntax error.
class XRegexSyntaxError : public smbase::XBase {
public:      // data
  // The original, uncompiled regular expression.
  std::string m_regex;

  // The implementation-specific error message describing the problem.
  std::string m_errorMessage;

public:      // methods
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

  // Let MatchResultsIterator get the compiled regex.
  friend class MatchResultsIterator;

private:     // data
  // The original, uncompiled regular expression.
  std::string m_orig_regex;

  // Pointer to compiled `std::regex` allocated with `new`.
  void *m_compiled_regex_ptr;

public:      // methods
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
  //
  // The "B" suffix means it returns `bool`.
  bool searchB(std::string const &str) const;

  // Search for a matching substring of `str`, returning a match object
  // that either indicates a failure to match or has the matching
  // substrings.
  //
  // The "MR" suffix means it returns `MatchResults`.
  MatchResults searchMR(std::string const &str) const;

  // Within `str`, replace occurrences that match this Regex with
  // `replacement`, and return the substituted result.
  std::string replaceAll(std::string const &str,
                         std::string const &replacement) const;
};


// Result of a regex match or search operation.
//
// This is similar to `std::smatch`, with the primary difference being
// that it makes a copy of the substrings instead of retaining iterators
// to the original target due to the danger of the iterators dangling if
// the target string is destroyed before the matches are examined.  (I
// consider the solution in LWG DR 2329, namely prohibiting passing an
// rvalue reference, to be inadequate.)
//
class MatchResults {
  // Let Regex and MatchResultsIterator populate this object.
  friend class Regex;
  friend class MatchResultsIterator;

private:     // types
  // Marker type to select the private constructor.
  enum SMatchTag { SMATCH_TAG };

private:     // data
  // Matched substrings, where index 0 is the entire match, and
  // subsequent indices correspond to parenthesized groups in the regex.
  std::vector<std::string> m_matches;

private:     // methods
  // Populate from a pointer to `std::smatch`.  This copies the
  // substrings and does not retain the passed pointer.
  MatchResults(void const *smatch, SMatchTag);

public:      // methods
  ~MatchResults();

  // Construct an empty match, which indicates that match was
  // unsuccessful if it is not subsequently populated.
  MatchResults();

  MatchResults(MatchResults const &obj);
  MatchResults(MatchResults &&obj);

  MatchResults &operator=(MatchResults const &obj);
  MatchResults &operator=(MatchResults &&obj);

  // True if the match failed.
  bool empty() const
    { return m_matches.empty(); }

  // True if the match succeeded.
  bool succeeded() const
    { return !empty(); }
  operator bool () const
    { return succeeded(); }

  // Number of matched substrings, including the one corresponding to
  // the entire match.  Returns 0 if the match failed.
  std::size_t size() const
    { return m_matches.size(); }

  // Get the indicated match.
  std::string const &at(std::size_t index) const
    { return m_matches.at(index); }
  std::string const &operator[](std::size_t index) const
    { return at(index); }

  // Get all substrings as a vector.
  std::vector<std::string> const &asVector() const
    { return m_matches; }
};


// Iterator for successive match results.
//
// Similar to `std::sregex_iterator`.
//
class MatchResultsIterator {
private:     // data
  // The string we are finding matches in.  We make a copy because
  // otherwise it is easy to get dangling iterators.  This is empty for
  // the end iterator.
  std::string m_targetString;

  // Pointer to `std::sregex_iterator` allocated with `new`.
  void *m_iter_ptr;

private:      // methods
  // Turn this into an end iterator.
  void setAsEnd();

public:      // methods
  ~MatchResultsIterator();

  // Return an end iterator.
  MatchResultsIterator();

  // Return a begin iterator over matches of `regex` within `str`.
  //
  // This makes a copy of `str`, but it retains a reference to `regex`,
  // so the latter object must live at least as long as the iterator.
  //
  MatchResultsIterator(std::string const &str, Regex const &regex);

  bool operator==(MatchResultsIterator const &obj) const;
  bool operator!=(MatchResultsIterator const &obj) const
    { return !operator==(obj); }

  // Return the current match results.
  //
  // Requires that this is not the end iterator.
  //
  MatchResults operator*() const;

  // Advance the iterator.
  //
  // Requires that this is not the end iterator.
  //
  MatchResultsIterator &operator++();
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_REGEX_H
