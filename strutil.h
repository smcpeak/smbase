// strutil.h            see license.txt for copyright and terms of use
// A set of generic string utilities, including replace(), translate(),
// trimWhitespace(), encodeWithEscapes(), etc.
//
// TODO: This module should be combined with string-utils.h.

#ifndef SMBASE_STRUTIL_H
#define SMBASE_STRUTIL_H

#include "str.h"      // string
#include "array.h"    // ArrayStack

#include <stdio.h>    // FILE


// direct string replacement, replacing instances of oldstr with newstr
// (newstr may be "")
string replace(rostring src, rostring oldstr, rostring newstr);

// Expand a string that may contain 'tr'-like ranges.
string expandRanges(char const *chars);

// works like unix "tr": the source string is translated character-by-character,
// with occurrences of 'srcchars' replaced by corresponding characters from
// 'destchars'; further, either set may use the "X-Y" notation to denote a
// range of characters from X to Y
string translate(rostring src, rostring srcchars, rostring destchars);

// a simple example of using translate; it was originally inline, but a bug
// in egcs made me move it out of line
string stringToupper(rostring src);
//  { return translate(src, "a-z", "A-Z"); }

string stringTolower(rostring src);


// remove any whitespace at the beginning or end of the string
string trimWhitespace(rostring str);
// dsw: get the first alphanum token in the string
string firstAlphanumToken(rostring str);


// encode a block of bytes as a string with C backslash escape
// sequences (but without the opening or closing quotes)
//
// 'src' is *not* rostring, since it is not NUL terminated
string encodeWithEscapes(char const *src, int len);

// Overloads for the other variants of 'char'.
inline string encodeWithEscapes(unsigned char const *src, int len)
  { return encodeWithEscapes((char const *)src, len); }
inline string encodeWithEscapes(signed char const *src, int len)
  { return encodeWithEscapes((char const *)src, len); }

// safe when the text has no NUL characters
string encodeWithEscapes(rostring src);

// adds the quotes too
string quoted(rostring src);


// decode an escaped string; throw XFormat if there is a problem
// with the escape syntax; if 'delim' is specified, it will also
// make sure there are no unescaped instances of that
void decodeEscapes(ArrayStack<char> &dest, rostring src,
                   char delim = 0, bool allowNewlines=false);

// given a string with quotes and escapes, yield just the string;
// works if there are no escaped NULs
string parseQuotedString(rostring text);


// For printable ASCII other than single quote or backslash, return 'c'.
// Otherwise, return '\'', '\\', '\xNN', '\uNNNN', or '\UNNNNNNNN'.
string quoteCharacter(int c);


// Return a string that, in the POSIX shell syntax, denotes 's'.  If no
// quoting is needed, returns 's'.  This uses double-quotes when 's'
// needs quoting, which is when 's' contains a shell metacharacter or
// any character outside the printable ASCII range.
string shellDoubleQuote(string const &s);


// 2018-06-30: I moved 'localTimeString' into datetime.h


// given a directory name like "a/b/c", return "c"
// renamed from 'basename' because of conflict with something in string.h
string sm_basename(rostring src);

// given a directory name like "a/b/c", return "a/b"; if 'src' contains
// no slashes at all, return "."
string dirname(rostring src);


// return 'prefix', pluralized if n!=1; for example
//   plural(1, "egg") yields "egg", but
//   plural(2, "egg") yields "eggs";
// it knows about a few irregular pluralizations (see the source),
// and the expectation is I'll add more irregularities as I need them
string plural(int n, rostring prefix);

// same as 'plural', but with the stringized version of the number:
//   pluraln(1, "egg") yields "1 egg", and
//   pluraln(2, "egg") yields "2 eggs"
string pluraln(int n, rostring prefix);

// prepend with an indefinite article:
//   a_or_an("foo") yields "a foo", and
//   a_or_an("ogg") yields "an ogg"
string a_or_an(rostring noun);


// Sometimes it's useful to store a string value in a static buffer;
// most often this is so 'gdb' can see the result.  This function just
// copies its input into a static buffer (of unspecified length, but
// it checks bounds internally), and returns a pointer to that buffer.
char *copyToStaticBuffer(char const *src);


// true if the first part of 'str' matches 'prefix'
bool prefixEquals(rostring str, rostring prefix);

// and similar for last part
bool suffixEquals(rostring str, rostring suffix);


// True if 'needle' occurs as a substring within 'haystack'.  If
// 'needle' is empty, this always returns true.
bool hasSubstring(string const &haystack, string const &needle);

// If 'needle' occurs within 'haystack', return the byte offset of the
// first byte of the first occurrence.  Otherwise, return -1.
int indexOfSubstring(string const &haystack, string const &needle);


// read/write strings <-> files
void writeStringToFile(rostring str, rostring fname);
string readStringFromFile(rostring fname);

// Append to 'dest' all of the lines in 'fname'.  If 'chomp' is true,
// each line has no newline terminator; otherwise, all but the last have
// newlines, and the last may or may not depending on how the file ends.
// Throws an exception on error, including xSysError for file-not-found.
void readLinesFromFile(ArrayStack<string> /*INOUT*/ &dest,
                       rostring fname,
                       bool chomp = true);


// read the next line from a FILE* (e.g. an AutoFILE); the
// newline is returned if it is present (you can use 'chomp'
// to remove it); returns false (and "") on EOF
bool readLine(string &dest, FILE *fp);


// like perl 'chomp': remove a final newline if there is one
string chomp(rostring src);


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
int compareStringPtrs(string const *a, string const *b);


#endif // SMBASE_STRUTIL_H
