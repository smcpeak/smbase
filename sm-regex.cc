// sm-regex.cc
// code for sm-regex.h

#include "sm-regex.h"                  // this module

#include "exc.h"                       // THROW
#include "sm-macros.h"                 // OPEN_NAMESPACE, [M]{C,D}MEMB
#include "string-util.h"               // doubleQuote
#include "stringb.h"                   // stringb

#include <regex>                       // std::{regex, regex_search, ...}
#include <string>                      // std::string
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// ------------------------- XRegexSyntaxError -------------------------
XRegexSyntaxError::~XRegexSyntaxError()
{}


XRegexSyntaxError::XRegexSyntaxError(
  std::string const &regex,
  std::string const &errorMessage)
  : m_regex(regex),
    m_errorMessage(errorMessage)
{}


std::string XRegexSyntaxError::getConflict() const
{
  return stringb(
    "Regex " << doubleQuote(m_regex) <<
    " syntax error: " << m_errorMessage);
}


// ------------------------------- Regex -------------------------------
// Properly-typed 'm_compiled_regex_ptr'.
#define M_COMPILED_REGEX_PTR(obj) \
  (reinterpret_cast<std::regex *       &>((obj).m_compiled_regex_ptr))
#define M_COMPILED_REGEX_PTR_C(obj) \
  (reinterpret_cast<std::regex * const &>((obj).m_compiled_regex_ptr))


Regex::~Regex()
{
  delete M_COMPILED_REGEX_PTR(*this);
}


Regex::Regex(std::string const &re)
  : m_orig_regex(re),
    m_compiled_regex_ptr(nullptr)
{
  try {
    M_COMPILED_REGEX_PTR(*this) = new std::regex(m_orig_regex);
  }
  catch (std::regex_error &x) {
    // Note: The memory that `new` allocated before the exception was
    // thrown has already been deallocated automatically.

    THROW(XRegexSyntaxError(m_orig_regex, x.what()));
  }
}


bool Regex::searchB(std::string const &str) const
{
  try {
    // This can throw exceptions related to resource usage.
    return std::regex_search(str, *M_COMPILED_REGEX_PTR_C(*this));
  }
  catch (std::regex_error &) {
    // Excessive resource usage that is detected and reported as an
    // exception is similar to excessive resource usage that simply
    // causes the program to run slowly.  Since it's a significant
    // hassle to try to meaningfully handle a resource exception, I'm
    // simply going to say that the regex did not match the given input.
    // If this occurs frequently enough to be an issue, this is probably
    // best found and dealt with using performance analysis tools.
    return false;
  }
}


MatchResults Regex::searchMR(std::string const &str) const
{
  std::smatch results;
  try {
    std::regex_search(str, results, *M_COMPILED_REGEX_PTR_C(*this));
  }
  catch (std::regex_error &) {
    // Treat a resource exception as failure to match.
    return MatchResults();
  }

  return MatchResults(&results, MatchResults::SMATCH_TAG);
}


std::string Regex::replaceAll(
  std::string const &str,
  std::string const &replacement) const
{
  try {
    return std::regex_replace(str, *M_COMPILED_REGEX_PTR_C(*this), replacement);
  }
  catch (std::regex_error &) {
    // Similar to above, regard a resource usage exception as meaning
    // that the regex did not match, meaning there is nothing to
    // replace.
    return str;
  }
}


// --------------------------- MatchResults ----------------------------
MatchResults::MatchResults(void const *match_results, SMatchTag)
  : m_matches()
{
  std::smatch const &results =
    *reinterpret_cast<std::smatch const *>(match_results);

  if (!results.empty()) {
    // Copy the results into `m_matches`.
    m_matches.reserve(results.size());
    for (std::string s : results) {
      m_matches.push_back(std::move(s));
    }
  }
}


MatchResults::~MatchResults()
{}


MatchResults::MatchResults()
  : m_matches()
{}


MatchResults::MatchResults(MatchResults const &obj)
  : DMEMB(m_matches)
{}


MatchResults::MatchResults(MatchResults &&obj)
  : MDMEMB(m_matches)
{}


MatchResults &MatchResults::operator=(MatchResults const &obj)
{
  CMEMB(m_matches);
  return *this;
}


MatchResults &MatchResults::operator=(MatchResults &&obj)
{
  MCMEMB(m_matches);
  return *this;
}


// ----------------------- MatchResultsIterator ------------------------
// Properly-typed 'm_iter_ptr'.
#define M_ITER_PTR(obj) \
  (reinterpret_cast<std::sregex_iterator *       &>((obj).m_iter_ptr))
#define M_ITER_PTR_C(obj) \
  (reinterpret_cast<std::sregex_iterator * const &>((obj).m_iter_ptr))


// Replace the existing underlying iterator of `this`.
#define REPLACE_ITER_PTR(newPtr)            \
  delete M_ITER_PTR(*this);                 \
  M_ITER_PTR(*this) = (newPtr) /* user ; */


void MatchResultsIterator::setAsEnd()
{
  m_targetString.clear();

  REPLACE_ITER_PTR(new std::sregex_iterator());
}


MatchResultsIterator::~MatchResultsIterator()
{
  delete M_ITER_PTR(*this);
}


MatchResultsIterator::MatchResultsIterator()
  : m_targetString(),
    m_iter_ptr(nullptr)
{
  setAsEnd();
}


MatchResultsIterator::MatchResultsIterator(
  std::string const &str,
  Regex const &regex)
  : m_targetString(str),
    m_iter_ptr(nullptr)
{
  try {
    // Iterate over the copy in case `str` is destroyed.
    M_ITER_PTR(*this) = new std::sregex_iterator(
      m_targetString.begin(),
      m_targetString.end(),
      *M_COMPILED_REGEX_PTR_C(regex));
  }
  catch (std::regex_error &) {
    // Treat as no matches.
    setAsEnd();
  }
}


MatchResultsIterator::MatchResultsIterator(MatchResultsIterator const &obj)
  : DMEMB(m_targetString),
    m_iter_ptr(nullptr)
{
  M_ITER_PTR(*this) = new std::sregex_iterator(*M_ITER_PTR_C(obj));
}


MatchResultsIterator::MatchResultsIterator(MatchResultsIterator &&obj)
  : MDMEMB(m_targetString),
    m_iter_ptr(nullptr)
{
  M_ITER_PTR(*this) =
    new std::sregex_iterator(std::move(*M_ITER_PTR(obj)));
}


MatchResultsIterator &MatchResultsIterator::operator=(MatchResultsIterator const &obj)
{
  if (this != &obj) {
    CMEMB(m_targetString);

    REPLACE_ITER_PTR(new std::sregex_iterator(*M_ITER_PTR_C(obj)));
  }

  return *this;
}


MatchResultsIterator &MatchResultsIterator::operator=(MatchResultsIterator &&obj)
{
  if (this != &obj) {
    MCMEMB(m_targetString);

    REPLACE_ITER_PTR(
      new std::sregex_iterator(std::move(*M_ITER_PTR(obj))));
  }

  return *this;
}


bool MatchResultsIterator::operator==(MatchResultsIterator const &obj) const
{
  return *M_ITER_PTR_C(*this) == *M_ITER_PTR_C(obj);
}


MatchResults MatchResultsIterator::operator*() const
{
  // I don't think this can throw `regex_error`.
  std::smatch const sm = * *M_ITER_PTR_C(*this);
  return MatchResults(&sm, MatchResults::SMATCH_TAG);
}


MatchResultsIterator &MatchResultsIterator::operator++()
{
  try {
    ++ *M_ITER_PTR(*this);
  }
  catch (std::regex_error &) {
    // Treat as no more matches.
    setAsEnd();
  }

  return *this;
}


// ----------------------- MatchResultsIterable ------------------------
MatchResultsIterable::~MatchResultsIterable()
{}


MatchResultsIterable::MatchResultsIterable(
  std::string const &str,
  Regex const &regex)
  : m_begin(str, regex),
    m_end()
{}


CLOSE_NAMESPACE(smbase)


// EOF
