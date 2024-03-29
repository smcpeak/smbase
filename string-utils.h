// string-utils.h
// Utilities related to std::string.

// This is intended to eventually replace strutil.h, which uses the
// old smbase 'string' class.

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <iosfwd>                      // std::ostream
#include <string>                      // std::string
#include <vector>                      // std::vector

// Split 'text' into non-empty words separated by 'sep', which never
// appears in any of the result words.
std::vector<std::string> splitNonEmpty(std::string const &text, char sep);

// Return elements of 'vec' separated by 'sep'.
std::string join(std::vector<std::string> const &vec,
                 std::string const &sep);

// Return 'vec' except with each element prefixed by 'prefix'.
std::vector<std::string> prefixAll(std::vector<std::string> const &vec,
                                   std::string const &prefix);

// Return 'vec' except with each element suffixed by 'suffix'.
std::vector<std::string> suffixAll(std::vector<std::string> const &vec,
                                   std::string const &suffix);

// Insert 'str' into 'os', surrounded by double quotes, and using C-like
// escape sequences for double-quotes, backslashes, and all
// non-printable characters.
void insertDoubleQuoted(std::ostream &os, std::string const &str);

// Return 's' in the 'insertDoubleQuoted' form.
std::string doubleQuote(std::string const &s);

// Write 'vec' to 'os' like: ["first", "second", "third"].  The elements
// are quoted using the 'insertDoubleQuoted' function.
std::ostream& operator<< (std::ostream &os,
                          std::vector<std::string> const &vec);

// Convert 'vec' to a string using 'operator<<'.
std::string toString(std::vector<std::string> const &vec);

// If 'fname' contains any '.' characters, remove the last one and all
// following characters, and return that.  Otherwise return 'fname'.
std::string stripExtension(std::string const &fname);

// True if 'arr' is strictly sorted in ascending order per 'strcmp',
// which is equivalent to "LANG=C sort" order.
bool isStrictlySortedStringArray(char const * const *arr,
                                 size_t arrLength);

// Return true if 'str' compares equal to any of the strings in 'arr',
// whose length is 'arrLength'.  The array must satisfy
// 'isStrictlySortedStringArray'.
bool stringInSortedArray(char const *str, char const * const *arr,
                         size_t arrLength);

// True if 'str' begins with 'prefix'.
bool beginsWith(std::string const &str, std::string const &prefix);

#endif // STRING_UTILS_H
