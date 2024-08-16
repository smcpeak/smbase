// sm-regex.cc
// code for sm-regex.h

#include "sm-regex.h"                  // this module

#include "exc.h"                       // THROW
#include "sm-macros.h"                 // OPEN_NAMESPACE, [M]{C,D}MEMB
#include "string-util.h"               // doubleQuote
#include "stringb.h"                   // stringb

#include <regex>                       // std::{regex, regex_search, regex_replace, smatch}
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
// Properly-typed 'm_compiled_regex'.
#define M_STD_REGEX (reinterpret_cast<std::regex * &>(m_compiled_regex))
#define M_STD_REGEX_C (reinterpret_cast<std::regex * const &>(m_compiled_regex))


Regex::~Regex()
{
  delete M_STD_REGEX;
}


Regex::Regex(std::string const &re)
  : m_orig_regex(re),
    m_compiled_regex(nullptr)
{
  try {
    M_STD_REGEX = new std::regex(m_orig_regex);
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
    return std::regex_search(str, *M_STD_REGEX_C);
  }
  catch (...) {
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
  MatchResults ret;

  std::smatch results;
  try {
    std::regex_search(str, results, *M_STD_REGEX_C);
  }
  catch (...) {
    // Treat a resource exception as failure to match.
    return ret;
  }

  if (!results.empty()) {
    // Copy the results into `ret`.
    ret.m_matches.reserve(results.size());
    for (std::string s : results) {
      ret.m_matches.push_back(std::move(s));
    }
  }

  return ret;
}


std::string Regex::replaceAll(
  std::string const &str,
  std::string const &replacement) const
{
  try {
    return std::regex_replace(str, *M_STD_REGEX_C, replacement);
  }
  catch (...) {
    // Similar to above, regard a resource usage exception as meaning
    // that the regex did not match, meaning there is nothing to
    // replace.
    return str;
  }
}


// --------------------------- MatchResults ----------------------------
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


CLOSE_NAMESPACE(smbase)


// EOF
