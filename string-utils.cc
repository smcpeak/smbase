// string-utils.cc
// Code for string-utils.h.

#include "string-utils.h"              // this module

#include "strcmp-compare.h"            // StrcmpCompare
#include "vector-utils.h"              // accumulateWith
#include "xassert.h"                   // xassertdb

#include <algorithm>                   // std::binary_search
#include <cstring>                     // std::strrchr
#include <sstream>                     // std::ostringstream


// Based on one of the answers at
// https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
// although that answer is buggy (the final test is wrong) so I fixed
// the bug.
std::vector<std::string> splitNonEmpty(std::string const &text, char sep)
{
  // Result words.
  std::vector<std::string> tokens;

  // Place to start looking for the next token.
  std::string::size_type start = 0;

  // Location of the next 'sep' character after (or at) 'start', or
  // 'npos' if none is found.
  std::string::size_type end = 0;

  // Look for the next occurrence of 'sep'.
  while ((end = text.find(sep, start)) != std::string::npos) {
    // Take the token unless 'sep' occurred immediately.
    if (end != start) {
      tokens.push_back(text.substr(start, end - start));
    }

    // Skip past the separator we just found.
    start = end + 1;
  }

  // In the code I started with, the test was 'end != start', but that
  // is always true because we know that 'end==npos'.  Instead we want
  // to check that 'start' is not at the end of the string, which is
  // *not* where 'end' is.
  if (start < text.size()) {
    tokens.push_back(text.substr(start));
  }

  return tokens;
}


// Given that this is a one-liner, one might ask: why have it at all,
// rather than letting callers use 'accumulateWith'?  The problem with
// the latter is that I can't just pass a quoted string literal because
// template argument deduction is unhappy if I do that.  By defining
// this non-template, that problem goes away.
std::string join(std::vector<std::string> const &vec,
                 std::string const &sep)
{
  return accumulateWith(vec, sep);
}


void insertDoubleQuoted(std::ostream &os, std::string const &str)
{
  os << '"' << std::oct;

  for (char c : str) {
    unsigned char uc = (unsigned char)c;
    switch (c) {
      case '"':
      case '\\':
        os << '\\' << c;
        break;

      case '\a':
        os << "\\a";
        break;

      case '\b':
        os << "\\b";
        break;

      case '\f':
        os << "\\f";
        break;

      case '\n':
        os << "\\n";
        break;

      case '\r':
        os << "\\r";
        break;

      case '\t':
        os << "\\t";
        break;

      case '\v':
        os << "\\v";
        break;

      default:
        if (32 <= uc && uc <= 126) {
          os << c;
        }
        else if (uc <= 7) {
          // 'uc' would print as a single digit, so explicitly print
          // leading zeroes.  Octal output is configured above.
          //
          // I choose to print in octal rather than hex because a hex
          // sequence does not have any length limit, meaning if the
          // hex sequence is followed by a printable character that is
          // also a hex digit, that will be misinterpreted (unless I
          // use a hex escape for it too).
          os << "\\00" << (int)uc;
        }
        else if (uc <= 077) {
          // Would be two digits.
          os << "\\0" << (int)uc;
        }
        else {
          // Three digits.
          os << '\\' << (int)uc;
        }
        break;
    }
  }

  os << std::dec << '"';
}


std::string doubleQuote(std::string const &s)
{
  std::ostringstream oss;
  insertDoubleQuoted(oss, s);
  return oss.str();
}


std::ostream & operator<< (std::ostream &os,
                           std::vector<std::string> const &vec)
{
  os << '[';

  auto it = vec.begin();
  if (it != vec.end()) {
    goto skipComma;

    for (; it != vec.end(); ++it) {
      os << ", ";

    skipComma:
      insertDoubleQuoted(os, *it);
    }
  }

  os << ']';
  return os;
}


std::string toString(std::vector<std::string> const &vec)
{
  std::ostringstream oss;
  oss << vec;
  return oss.str();
}


std::string stripExtension(std::string const &fname)
{
  char const *start = fname.c_str();
  char const *dot = std::strrchr(start, '.');
  if (dot) {
    return std::string(start, dot-start);
  }
  else {
    return fname;
  }
}


bool isStrictlySortedStringArray(char const * const *arr, size_t arrLength)
{
  for (size_t i = 1; i < arrLength; i++) {
    if (strcmpCompare(arr[i-1], arr[i])) {
      // These two are in the right order.
    }
    else {
      // Not in right order.
      return false;
    }
  }
  return true;
}


bool stringInSortedArray(char const *str, char const * const *arr,
                         size_t arrLength)
{
  xassertdb(isStrictlySortedStringArray(arr, arrLength));
  return std::binary_search(arr, arr+arrLength, str, StrcmpCompare());
}


bool beginsWith(std::string const &str, std::string const &prefix)
{
  // This does a backward search from the *beginning*, meaning the
  // prefix must occur right at the start to get a zero return.
  //
  // This bit of cleverness is taken from:
  // https://stackoverflow.com/questions/1878001/how-do-i-check-if-a-c-stdstring-starts-with-a-certain-string-and-convert-a
  return str.rfind(prefix, 0) == 0;
}


// EOF
