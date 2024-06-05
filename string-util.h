// string-util.h
// Utilities related to `std::string`.

// This was intended to eventually replace strutil.h, which was based on
// the old smbase 'string' class.  However, I have now changed str.h to
// declare 'string' as an alias for 'std::string', so both modules now
// use 'std::string', making them somewhat redundant.
//
// My current plan is to move what I want to keep from `strutil` into
// this module, leaving `strutil` deprecated.

#ifndef SMBASE_STRING_UTIL_H
#define SMBASE_STRING_UTIL_H

#include "codepoint.h"                 // CodePoint
#include "sm-macros.h"                 // DEPRECATED

#include <cstdint>                     // std::{int64_t, uint64_t}
#include <iosfwd>                      // std::ostream
#include <string>                      // std::string
#include <vector>                      // std::vector


// ------------------------------ Parsing ------------------------------
// Split 'text' into non-empty words separated by 'sep', which never
// appears in any of the result words.
std::vector<std::string> splitNonEmpty(std::string const &text, char sep);


// Remove any whitespace (as determined by `std::isspace`) at the
// beginning or end of the string.
std::string trimWhitespace(std::string const &str);


// ------------------------- Tests on strings --------------------------
// True if 'str' begins with 'prefix'.
bool beginsWith(std::string const &str, std::string const &prefix);

// True if 'str' ends with 'suffix'.
bool endsWith(std::string const &str, std::string const &suffix);

// True if 'str' contains 'c'.
bool contains(std::string const &str, char c);


// True if 'needle' occurs as a substring within 'haystack'.  If
// 'needle' is empty, this always returns true.
bool hasSubstring(std::string const &haystack, std::string const &needle);

// If 'needle' occurs within 'haystack', return the byte offset of the
// first byte of the first occurrence.  Otherwise, return -1.
int indexOfSubstring(std::string const &haystack, std::string const &needle);


// ------------------ Manipulating vectors of strings ------------------
// Return elements of 'vec' separated by 'sep'.
std::string join(std::vector<std::string> const &vec,
                 std::string const &sep);

// Return 'vec' except with each element prefixed by 'prefix'.
std::vector<std::string> prefixAll(std::vector<std::string> const &vec,
                                   std::string const &prefix);

// Return 'vec' except with each element suffixed by 'suffix'.
std::vector<std::string> suffixAll(std::vector<std::string> const &vec,
                                   std::string const &suffix);

// Write 'vec' to 'os' like: ["first", "second", "third"].  The elements
// are quoted using the 'insertDoubleQuoted' function.
std::ostream& operator<< (std::ostream &os,
                          std::vector<std::string> const &vec);

// Convert 'vec' to a string using 'operator<<'.
std::string toString(std::vector<std::string> const &vec);


// --------------------- Searching array of char* ----------------------
// True if 'arr' is strictly sorted in ascending order per 'strcmp',
// which is equivalent to "LANG=C sort" order.
bool isStrictlySortedStringArray(char const * const *arr,
                                 std::size_t arrLength);

// Return true if 'str' compares equal to any of the strings in 'arr',
// whose length is 'arrLength'.  The array must satisfy
// 'isStrictlySortedStringArray'.
bool stringInSortedArray(char const *str, char const * const *arr,
                         std::size_t arrLength);


// ----------------------------- Escaping ------------------------------
/*
  Write unicode code point `c` to `os`, substituting a C escape sequence
  if it is not printable US-ASCII or is a metacharacter.  Specifically:

  - non-printing characters <= 255 escaped using octal,

  - characters >= 256 escaped using "\u{N+}" with capital hex digits,

  - whitespace and metacharacters (backslash, single-, and double-quote)
    escaped using backslash mnemonics, and

  - all other characters represent themselves.

  If `delim` is not 0, then if it is double-quote, do not escape single
  quotes, and vice-versa.

  Requires: `0 <= c && c <= 0x10FFFF`.
*/
void insertPossiblyEscapedChar(std::ostream &os, int c, int delim=0);


// Return `src` with all of its characters escaped using
// `insertPossiblyEscapedChar`, but without surrounding the result with
// quotation marks.
std::string encodeWithEscapes(std::string const &src);

// Same, but with the source specified by a pointer and byte length.
std::string encodeWithEscapes(char const *src, int len);

// Overloads for the other variants of 'char'.
inline std::string encodeWithEscapes(unsigned char const *src, int len)
  { return encodeWithEscapes((char const *)src, len); }
inline std::string encodeWithEscapes(signed char const *src, int len)
  { return encodeWithEscapes((char const *)src, len); }


// Insert 'str' into 'os', surrounded by double quotes, and using C-like
// escape sequences for double-quotes, backslashes, and all
// non-printable characters.
void insertDoubleQuoted(std::ostream &os, std::string const &str);

// Return 's' in the 'insertDoubleQuoted' form.
std::string doubleQuote(std::string const &s);

// Return `c` enclosed in single quotes if it is printable and not a
// metacharacter, or as an escape sequence if not.
//
// Requires: `c.has_value()`.
std::string singleQuoteChar(CodePoint c);


// ---------------------------- File names -----------------------------
// If 'fname' contains any '.' characters, remove the last one and all
// following characters, and return that.  Otherwise return 'fname'.
std::string stripExtension(std::string const &fname);


// ----------------------- Manipulating strings ------------------------
// If 'str' is longer than 'maxLen', truncate it to 'maxLen', with the
// last three characters replaced with "...".  If 'maxLen' is less than
// or equal to 3, and truncation is necessary, the result is 'maxLen'
// dots.
std::string possiblyTruncatedWithEllipsis(
  std::string const &str, std::size_t maxLen);


// Within `src`, replace occurrences of `oldstr` with `newstr` (which
// may be empty).  After each replacement, searching for the next
// occurrence begins right after the inserted text, so that text is not
// subject to further replacement.
//
// Requires: `!oldstr.empty()`.
std::string replaceAll(
  std::string const &src,
  std::string const &oldstr,
  std::string const &newstr);

// Old name that I think is a little too short.
std::string replace(
  std::string const &src,
  std::string const &oldstr,
  std::string const &newstr)
  DEPRECATED("Use `replaceAll` instead.");


// If 'full' ends with 'suffix', return a string like 'full' but with
// that suffix removed.  Otherwise just return 'full'.
std::string removeSuffix(std::string const &full,
                         std::string const &suffix);


// Expand a string that may contain 'tr'-like ranges.  For example,
// "A-F" becomes "ABCDEF".
std::string expandRanges(char const *chars);

/* This is sort of like unix "tr" in that it returns `str` after
   translating each of its characters.

   For each character, if it is in `srcChars`, then it is translated to
   the character at the same position in `destChars`, or to itself if
   there is no corresponding character in `destChars` because it is too
   short.

   Both `srcChars` and `destChars` are first processed with
   `expandRange` so that, for example "A-Z" becomes "ABC...XYZ".
*/
std::string translate(
  std::string const &str,
  std::string const &srcChars,
  std::string const &destChars);

// Convert all of the US-ASCII letters in `src` to uppercase.
std::string stringToupper(std::string const &src);

// Convert all of the US-ASCII letters in `src` to lowercase.
std::string stringTolower(std::string const &src);


// ----------------------- Regular expressions -------------------------
// True if 'str' matches 'regex'.
//
// The regex match is performed by compiling it with the defaults for
// 'std::regex' and then calling 'std::regex_search'.
//
// This is not very efficient because it compiles the regex from scratch
// every time.
//
bool matchesRegex(std::string const &str, std::string const &regex);

// Turn `s` into a regex that will match `s` only, escaping any regex
// metacharacters it contains.
std::string escapeForRegex(std::string const &s);


// ---------------------- Stringifying numbers -------------------------
// Return `value` as a string of digits in `radix`, itself in [2,36].
std::string uint64ToRadixDigits(
  std::uint64_t magnitude, int radix);

// Same, but also with a radix indicator of "0b", "0o", or "0x".
// `radix` must be in {2, 8, 10, 16}.
std::string uint64ToRadixPrefixedDigits(
  std::uint64_t magnitude, int radix);

/* Return `value` as a string of digits in `radix`, possibly with a
   radix indicator of "0b", "0o", or "0x".  If it is negative, then a
   hyphen is the first character.  When letters are used (`radix>10`),
   they are uppercase.

   `radix` must be in [2,36].

   If `radixIndicator`, `radix` must be in {2, 8, 10, 16}.
*/
std::string int64ToRadixDigits(
  std::int64_t value, int radix, bool radixIndicator);


#endif // SMBASE_STRING_UTIL_H
