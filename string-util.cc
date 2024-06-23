// string-util.cc
// Code for string-util.h.

#include "string-util.h"               // this module

#include "breaker.h"                   // breaker
#include "exc.h"                       // smbase::xmessage
#include "overflow.h"                  // safeToInt
#include "strcmp-compare.h"            // StrcmpCompare
#include "strutil.h"                   // stringf
#include "vector-util.h"               // accumulateWith
#include "xassert.h"                   // xassert, xassertdb, xassertPrecondition

#include <algorithm>                   // std::binary_search
#include <cctype>                      // std::isspace
#include <cstdlib>                     // std::abs
#include <cstring>                     // std::{strchr, strrchr}
#include <limits>                      // std::numeric_limits
#include <sstream>                     // std::ostringstream
#include <string_view>                 // std::string_view
#include <regex>                       // std::regex
#include <vector>                      // std::vector

using namespace smbase;


// ------------------------------ Parsing ------------------------------
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


std::string trimWhitespace(string const &origStr)
{
  std::string_view str(origStr);

  // Indices enclosing the portion of the string that remains a
  // candidate to return.
  std::size_t begin = 0;
  std::size_t end = str.size();

  // Set `begin` to the index of the first non-ws character.
  while (begin < end &&
         std::isspace(static_cast<unsigned char>(str[begin]))) {
    ++begin;
  }

  // Set `end` to the index+1 of the last non-ws character.
  while (begin < end &&
         std::isspace(static_cast<unsigned char>(str[end-1]))) {
    --end;
  }

  xassert(begin <= end);

  return std::string(str.substr(begin, end-begin));
}


// ------------------------- Tests on strings --------------------------
bool beginsWith(std::string const &str, std::string const &prefix)
{
  // This does a backward search from the *beginning*, meaning the
  // prefix must occur right at the start to get a zero return.
  //
  // This bit of cleverness is taken from:
  // https://stackoverflow.com/questions/1878001/how-do-i-check-if-a-c-stdstring-starts-with-a-certain-string-and-convert-a
  return str.rfind(prefix, 0) == 0;
}


bool endsWith(std::string const &str, std::string const &suffix)
{
  if (str.size() >= suffix.size()) {
    return str.find(suffix, str.size() - suffix.size() /*pos*/) !=
           std::string::npos;
  }
  else {
    return false;
  }
}


bool contains(std::string const &str, char c)
{
  return str.find(c) != std::string::npos;
}


bool hasSubstring(
  std::string const &haystack, std::string const &needle)
{
  return indexOfSubstring(haystack, needle) >= 0;
}

int indexOfSubstring(
  std::string const &haystack, std::string const &needle)
{
  std::string::size_type i = haystack.find(needle);
  if (i == std::string::npos) {
    return -1;
  }
  else {
    return safeToInt(i);
  }
}


// ------------------ Manipulating vectors of strings ------------------
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


std::vector<std::string> prefixAll(std::vector<std::string> const &vec,
                                   std::string const &prefix)
{
  std::vector<std::string> ret;
  ret.reserve(vec.size());
  for (auto const &s : vec) {
    ret.push_back(prefix + s);
  }
  return ret;
}


std::vector<std::string> suffixAll(std::vector<std::string> const &vec,
                                   std::string const &suffix)
{
  std::vector<std::string> ret;
  ret.reserve(vec.size());
  for (auto const &s : vec) {
    ret.push_back(s + suffix);
  }
  return ret;
}


std::vector<std::string> stringVectorFromPointerArray(
  int count, char const * const * NULLABLE array)
{
  xassertPrecondition(count==0 || array!=nullptr);

  std::vector<std::string> ret;

  for (int i=0; i < count; ++i) {
    xassertPrecondition(array[i]);
    ret.push_back(std::string(array[i]));
  }

  return ret;
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


// --------------------- Searching array of char* ----------------------
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


// ----------------------------- Escaping ------------------------------
void insertPossiblyEscapedChar(std::ostream &os, int c, int delim)
{
  xassertPrecondition(0 <= c && c <= 0x10FFFF);

  switch (c) {
    case '"':
    case '\'':
      if (!delim || delim == c) {
        // If there is no active delimiter context, or it is the same
        // as the quotation mark `c`, escape `c`.
        os << '\\';
      }
      os << (char)c;
      break;

    case '\\':
      os << '\\' << (char)c;
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
      if (32 <= c && c <= 126) {
        os << (char)c;
      }
      else if (c <= 255) {
        // I choose to print in octal rather than hex because a hex
        // sequence does not have any length limit, meaning if the
        // hex sequence is followed by a printable character that is
        // also a hex digit, that will be misinterpreted (unless I
        // use a hex escape for it too).
        os << stringf("\\%03o", c);
      }
      else {
        os << stringf("\\u{%X}", c);
      }
      break;
  }
}


std::string encodeWithEscapes(std::string const &src)
{
  return encodeWithEscapes(src.data(), src.size());
}


std::string encodeWithEscapes(char const *src, int len)
{
  std::ostringstream oss;
  while (len-- > 0) {
    insertPossiblyEscapedChar(oss, (unsigned char)*src);
    ++src;
  }
  return oss.str();
}


void insertDoubleQuoted(std::ostream &os, std::string const &str)
{
  os << '"';

  for (char c : str) {
    unsigned char uc = (unsigned char)c;
    insertPossiblyEscapedChar(os, (int)uc, '"');
  }

  os << '"';
}


std::string doubleQuote(std::string const &s)
{
  std::ostringstream oss;
  insertDoubleQuoted(oss, s);
  return oss.str();
}


std::string singleQuoteChar(CodePoint c)
{
  std::ostringstream oss;
  oss << '\'';
  insertPossiblyEscapedChar(oss, c.value(), '\'');
  oss << '\'';
  return oss.str();
}


// ---------------------------- File names -----------------------------
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


// ----------------------- Manipulating strings ------------------------
std::string possiblyTruncatedWithEllipsis(
  std::string const &str, std::size_t maxLen)
{
  if (str.length() > maxLen) {
    if (maxLen <= 3) {
      return std::string(maxLen, '.');
    }
    else {
      return str.substr(0, maxLen-3) + "...";
    }
  }
  else {
    return str;
  }
}


std::string replaceAll(
  std::string const &origSrc,
  std::string const &oldstr,
  std::string const &newstr)
{
  // It doesn't make sense to replace an empty substring, and if we
  // tried we would get into an infinite loop.
  xassertPrecondition(!oldstr.empty());

  std::string ret(origSrc);
  std::string::size_type i = ret.find(oldstr);
  while (i != std::string::npos) {
    ret.replace(i, oldstr.size(), newstr);
    i = ret.find(oldstr, i + newstr.size());
  }
  return ret;
}


std::string replace(
  std::string const &origSrc,
  std::string const &oldstr,
  std::string const &newstr)
{
  return replaceAll(origSrc, oldstr, newstr);
}


std::string removeSuffix(std::string const &full,
                         std::string const &suffix)
{
  if (endsWith(full, suffix)) {
    return full.substr(0, full.size() - suffix.size());
  }
  else {
    return full;
  }
}


std::string expandRanges(char const *chars_)
{
  std::ostringstream ret;

  // Fix from Hendrik Tews: use unsigned chars to as not to fall over
  // when ranges have values near 127 on compilers for which 'char' is
  // signed by default (which is probably the common case)
  unsigned char *chars = (unsigned char*)chars_;

  while (*chars) {
    if (chars[1] == '-' && chars[2] != 0) {
      // range specification
      if (chars[0] > chars[2]) {
        xmessage("expandRanges: range specification with wrong collation order");
      }

      // use 'int' so we can handle chars[2] == 255 (otherwise we get
      // infinite loop)
      for (int c = chars[0]; c <= chars[2]; c++) {
        ret << (unsigned char)c;
      }
      chars += 3;
    }
    else {
      // simple character specification
      ret << chars[0];
      chars++;
    }
  }

  return ret.str();
}


std::string translate(
  std::string const &origSrc,
  std::string const &srcchars,
  std::string const &destchars)
{
  // first, expand range notation in the specification sequences
  std::string srcSpec = expandRanges(toCStr(srcchars));
  std::string destSpec = expandRanges(toCStr(destchars));

  // Build a translation map.  Initially it maps every character to
  // itself.
  char map[256];
  std::string::size_type i;
  for (i=0; i<256; i++) {
    map[i] = (char)i;
  }

  // Now set map elements corresponding to entries in `srcSpec` to their
  // counterparts in `destSpec`.  If the specifications are not the same
  // size, ignore the excess from the longer ("SysV" behavior).
  for (i=0; i < srcSpec.length() && i < destSpec.length(); i++) {
    map[(unsigned char)( srcSpec[i] )] = destSpec[i];
  }

  // Array of characters holding the result of translation.
  std::vector<char> ret(origSrc.size());

  // Run through `src`, applying `map`.
  i = 0;
  for (char c : origSrc) {
    ret[i] = (char)map[(unsigned char)c];
    ++i;
  }

  return std::string(ret.data(), origSrc.size());
}


std::string stringToupper(std::string const &src)
{
  return translate(src, "a-z", "A-Z");
}

std::string stringTolower(std::string const &src)
{
  return translate(src, "A-Z", "a-z");
}


// ----------------------- Regular expressions -------------------------
bool matchesRegex(std::string const &str, std::string const &regex)
{
  try {
    std::regex re(regex);
    return std::regex_search(str, re);
  }
  catch (...) {
    // Give myself an easy place to put a breakpoint.
    //
    // TODO: Create a dedicated exception type for this.
    breaker();
    throw;
  }
}


// Based on https://stackoverflow.com/a/39237913/2659307 .
std::string escapeForRegex(std::string const &s)
{
  static const char metacharacters[] = R"(\.^$-+()[]{}|?*)";
  std::string out;
  out.reserve(s.size());
  for (char ch : s) {
    if (std::strchr(metacharacters, ch)) {
      // Put a backslash before each metacharacter.
      out.push_back('\\');
    }
    out.push_back(ch);
  }
  return out;
}


std::string replaceAllRegex(
  std::string const &str,
  std::string const &regexToReplace,
  std::string const &replacement)
{
  std::regex re(regexToReplace);
  return std::regex_replace(str, re, replacement);
}


// ---------------------- Stringifying numbers -------------------------
std::string uint64ToRadixDigits(
  std::uint64_t magnitude, int radix)
{
  xassertPrecondition(2 <= radix && radix <= 36);

  if (magnitude == 0) {
    return "0";
  }

  // Digits in reverse order.
  std::vector<char> digits;

  while (magnitude > 0) {
    uint64_t d = magnitude % (uint64_t)radix;
    magnitude /= (uint64_t)radix;

    if (d < 10) {
      digits.push_back((char)('0' + d));
    }
    else {
      digits.push_back((char)('A' + (d - 10)));
    }
  }

  return std::string(digits.rbegin(), digits.rend());
}


std::string uint64ToRadixPrefixedDigits(
  std::uint64_t magnitude, int radix)
{
  xassertPrecondition(radix==2 || radix==8 || radix==10 || radix==16);

  char const *prefix =
    (radix==2?  "0b" :
     radix==8?  "0o" :
     radix==16? "0x" : "");

  return std::string(prefix) + uint64ToRadixDigits(magnitude, radix);
}


std::string int64ToRadixDigits(
  std::int64_t value, int radix, bool radixIndicator)
{
  uint64_t magnitude;
  if (value == std::numeric_limits<int64_t>::min()) {
    // It is undefined to call `std::abs` on `value`, so directly
    // specify the result.
    magnitude = (std::numeric_limits<uint64_t>::max() >> 1) + 1;
  }
  else {
    magnitude = static_cast<uint64_t>(std::abs(value));
  }

  std::string magString;
  if (!radixIndicator) {
    magString = uint64ToRadixDigits(magnitude, radix);
  }
  else {
    magString = uint64ToRadixPrefixedDigits(magnitude, radix);
  }

  if (value < 0) {
    return std::string("-") + magString;
  }
  else {
    return magString;
  }
}


// EOF
