// sm-stristr.cc
// Code for sm-stristr.h.

#include "sm-stristr.h"                // this module

#include "codepoint.h"                 // convertUSASCIIToUpper
#include "xassert.h"                   // xassert


bool equalChars_insens_ascii(char a, char b)
{
  // Treat both incoming 'char' values as unsigned.
  int aCode = static_cast<unsigned char>(a);
  int bCode = static_cast<unsigned char>(b);

  return convertUSASCIIToUpper(aCode) == convertUSASCIIToUpper(bCode);
}


bool prefixEquals_insens_ascii(char const *str, char const *prefix)
{
  while (*prefix) {
    if (!equalChars_insens_ascii(*str, *prefix)) {
      return false;
    }

    ++str;
    ++prefix;
  }

  return true;
}


// Loosely based on the answer by user "chux" at
// https://stackoverflow.com/questions/27303062/strstr-function-like-that-ignores-upper-or-lower-case
//
// Beware: This code has worst-case quadratic running time.
char const *findSubstring_insens_ascii(char const *haystack,
                                       char const *needle)
{
  xassert(haystack);
  xassert(needle);

  do {
    // Does the needle match here?
    if (prefixEquals_insens_ascii(haystack, needle)) {
      return haystack;
    }

    ++haystack;
  } while (*haystack);

  return nullptr;
}


// EOF
