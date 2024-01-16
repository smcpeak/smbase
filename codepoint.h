// codepoint.h
// Routines related to Unicode code points: tests, conversions, etc.

#ifndef CODEPOINT_H
#define CODEPOINT_H

// Within this module, 'int' is used to denote a Unicode code point.
// Negative values and values greater than 0x10FFFF are accepted but
// regarded as not being mapped to anything.

// -------------------- Unicode general category --------------------
// The intent of this module is to eventually conform to the Unicode
// standard, but the implementation does not do so at this time for code
// points outside [0,127].

// Unicode general category "Letter, uppercase".
bool isUppercaseLetter(int c);

// Unicode general category "Letter, lowercase".
bool isLowercaseLetter(int c);

// Unicode general category "Letter".
bool isLetter(int c);

// Unicode general cetegory "Number, decimal digit".
bool isDecimalDigit(int c);

// Unicode property White_Space=yes.
bool isWhitespace(int c);


// ------------------------ My own categories -----------------------
// This section has code point categories unrelated to those in
// Unicode.

// True if 'c' is a character than can appear in an identifier in the
// C programming language.
bool isCIdentifierCharacter(int c);

// True if 'c' is one of the printable ASCII characters.  Note that, as
// best I can tell, the printable range is the same for all national
// ASCII variants as it is for US-ASCII, namely [32,126].
bool isASCIIPrintable(int c);

// True if 'c' is in ['0','9'].
bool isASCIIDigit(int c);

// True if 'isASCIIDigit' or 'c' is in ['A','F'] or ['a','f'].
bool isASCIIHexDigit(int c);

// True if 'c' is in ['0','7'].
bool isASCIIOctDigit(int c);

// True if 'c' is a POSIX or Bash shell metacharacter, including space,
// under the assumption that IFS has its usual value.
bool isShellMetacharacter(int c);


// ---------------------------- Conversions ----------------------------
// If 'c' encodes a lowercase letter in US-ASCII, return the
// corresponding uppercase letter code according to American English.
// Otherwise, return 'c'.
int convertUSASCIIToUpper(int c);


#endif // CODEPOINT_H
