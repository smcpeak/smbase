// sm-stristr.h
// Case-insensitive substring search.

#ifndef SM_STRISTR_H
#define SM_STRISTR_H


// True if 'a' and 'b' are equal aside from letter case when both are
// interpreted as US-ASCII.
bool equalChars_insens_ascii(char a, char b);


// True if 'str' begins with 'prefix', ignoring letter case, and
// treating both as strings of US-ASCII characters.
bool prefixEquals_insens_ascii(char const *str, char const *prefix);


// Treating both 'haystack' and 'needle' as strings of US-ASCII
// characters, if 'needle' appears as a substring within 'haystack',
// treating uppercase and lowercase letters as equivalent, return a
// pointer to the first such occurrence.  Otherwise, return nullptr.
//
// If 'needle' is empty, returns 'haystack', as the empty string is
// regarded as a substring at every location.
//
// Both pointers must not be nullptr.
//
char const *findSubstring_insens_ascii(char const *haystack,
                                       char const *needle);


// Unit tests, implemented in sm-stristr-test.cc.
void test_sm_stristr();


#endif // SM_STRISTR_H
