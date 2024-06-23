// strutil.h            see license.txt for copyright and terms of use
// A set of generic string utilities, including replace(), translate(),
// trimWhitespace(), encodeWithEscapes(), etc.
//
// TODO: Move the parts of this module I want to keep into
// string-util.h.

#ifndef SMBASE_STRUTIL_H
#define SMBASE_STRUTIL_H

#include "array.h"                     // ArrayStack
#include "sm-macros.h"                 // DEPRECATED

// My basic plan is to move the functionality that I think is worth
// keeping over into `string-util`, leaving the `strutil` module with
// only legacy compatibility aliases.  As I do that, modules that are
// still including `strutil` need access to the moved functions, so I
// pull them in here.
#include "string-util.h"               // moved functions

#include <string>                      // std::string

#include <stdio.h>                     // FILE


// dsw: get the first alphanum token in the string
//
// Silently deprecated: I think this is not a good way to do parsing, so
// I am not moving this to `string-util`.
//
std::string firstAlphanumToken(std::string const &str);


std::string quoted(std::string const &src)
  DEPRECATED("Use `doubleQuote` in `string-util` instead.");


// decode an escaped string; throw XFormat if there is a problem
// with the escape syntax; if 'delim' is specified, it will also
// make sure there are no unescaped instances of that
void decodeEscapes(ArrayStack<char> &dest, std::string const &src,
                   char delim = 0, bool allowNewlines=false)
  DEPRECATED("Use `decodeCStringEscapesToStream` or "
             "`decodeCStringEscapesToString` in `c-string-reader`.");

// given a string with quotes and escapes, yield just the string;
// works if there are no escaped NULs
std::string parseQuotedString(std::string const &text)
  DEPRECATED("Use `parseQuotedCString` in `c-string-reader`.");


// For printable ASCII other than single quote or backslash, return 'c'.
// Otherwise, return '\'', '\\', '\xNN', '\uNNNN', or '\UNNNNNNNN'.
std::string quoteCharacter(int c)
  DEPRECATED("Use `singleQuoteChar` in `string-util`.");


// Return a string that, in the POSIX shell syntax, denotes 's'.  If no
// quoting is needed, returns 's'.  This uses double-quotes when 's'
// needs quoting, which is when 's' contains a shell metacharacter or
// any character outside the printable ASCII range.
std::string shellDoubleQuote(string const &s);


// 2018-06-30: I moved 'localTimeString' into datetime.h


// given a directory name like "a/b/c", return "c"
// renamed from 'basename' because of conflict with something in string.h
std::string sm_basename(std::string const &src);

// given a directory name like "a/b/c", return "a/b"; if 'src' contains
// no slashes at all, return "."
std::string dirname(std::string const &src);


// return 'prefix', pluralized if n!=1; for example
//   plural(1, "egg") yields "egg", but
//   plural(2, "egg") yields "eggs";
// it knows about a few irregular pluralizations (see the source),
// and the expectation is I'll add more irregularities as I need them
std::string plural(int n, std::string const &prefix);

// same as 'plural', but with the stringized version of the number:
//   pluraln(1, "egg") yields "1 egg", and
//   pluraln(2, "egg") yields "2 eggs"
std::string pluraln(int n, std::string const &prefix);

// prepend with an indefinite article:
//   a_or_an("foo") yields "a foo", and
//   a_or_an("ogg") yields "an ogg"
std::string a_or_an(std::string const &noun);


// Sometimes it's useful to store a string value in a static buffer;
// most often this is so 'gdb' can see the result.  This function just
// copies its input into a static buffer (of unspecified length, but
// it checks bounds internally), and returns a pointer to that buffer.
char *copyToStaticBuffer(char const *src);


// true if the first part of 'str' matches 'prefix'
bool prefixEquals(std::string const &str, std::string const &prefix)
  DEPRECATED("Use `beginsWith` in `string-util` instead.");

// and similar for last part
bool suffixEquals(std::string const &str, std::string const &suffix)
  DEPRECATED("Use `endsWith` in `string-util` instead.");


// 2024-06-04: Moved `hasSubstring` and `indexOfSubstring` to
// `string-util`.


// Variants of the above where we treat the characters as US-ASCII and
// ignore letter case.
bool hasSubstring_insens_ascii(string const &haystack,
                               string const &needle);
int indexOfSubstring_insens_ascii(string const &haystack,
                                  string const &needle);


// read/write strings <-> files
void writeStringToFile(std::string const &str, std::string const &fname);
std::string readStringFromFile(std::string const &fname);

// Append to 'dest' all of the lines in 'fname'.  If 'chomp' is true,
// each line has no newline terminator; otherwise, all but the last have
// newlines, and the last may or may not depending on how the file ends.
// Throws an exception on error, including XSysError for file-not-found.
void readLinesFromFile(ArrayStack<std::string> /*INOUT*/ &dest,
                       std::string const &fname,
                       bool chomp = true);


// read the next line from a FILE* (e.g. an AutoFILE); the
// newline is returned if it is present (you can use 'chomp'
// to remove it); returns false (and "") on EOF
bool readLine(std::string &dest, FILE *fp);


// like perl 'chomp': remove a final newline if there is one
std::string chomp(std::string const &src);


// dsw: build a string with delimiters between each appended string
struct DelimStr {
  char delimiter;               // this could be more general but I don't need it
  stringBuilder sb;

  explicit DelimStr(char delimiter0);
  DelimStr& operator << (char const *text);
};


// compare function for strings; for use with qsort()
int compareStrings(const void *a, const void *b);
void qsortStringArray(char const **strings, int size);

// Variant for use with ArrayStack::sort.
int compareStringPtrs(std::string const *a, std::string const *b);


#endif // SMBASE_STRUTIL_H
