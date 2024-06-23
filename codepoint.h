// codepoint.h
// Routines related to Unicode code points: tests, conversions, etc.

#ifndef SMBASE_CODEPOINT_H
#define SMBASE_CODEPOINT_H

#include "xassert.h"                   // xassert

#include <iosfwd>                      // std::ostream


/* A "Code Point" is (quoting https://www.unicode.org/glossary/#C):

     Any value in the Unicode codespace; that is, the range of integers
     from 0 to 10FFFF_16.

   This class represents a code point distinctly from other integer
   types to ensure control over conversions.  In particular, it allows
   plain `char` to be treated as a code point by converting it to
   `unsigned char` first.

   Additionally, for convenience of functions that need an "absent" code
   point value to represent, e.g., end of file, this class supports an
   absent value.
*/
class CodePoint {
private:     // data
  // The code point value, or -1 for absent.
  //
  // Invariant: `-1 <= m_value && m_value <= 0x10FFFF`.
  int m_value;

public:
  // Absent value.
  CodePoint()
    : m_value(-1)
  {}

  /* Represent `value`, or an absent value if `value` is -1.

     It is debatable to allow -1 to be passed here, but I'm inserting
     this class into a body of code that has been using `int` with the
     "-1 means EOF" convention in many places, so this is the least
     disruptive incremental approach.
  */
  /*implicit*/ CodePoint(int value)
    : m_value(value)
  {
    xassertPrecondition(-1 <= value && value <= 0x10FFFF);
  }

  /* This constructor is the primary reason this class was created, to
     map `char` to [0,255] when I want to examine its value as a
     character code point.  This means I can, for example, iterate over
     the elements of a `std::string` and invoke various functions that
     take a `CodePoint` without having large values misinterpreted as
     negative.
  */
  /*implicit*/ CodePoint(char c)
    : m_value(static_cast<unsigned char>(c))
  {
    xassert(m_value >= 0);
  }

  // May as well do this too, even though `signed char` per se is rare.
  /*implicit*/ CodePoint(signed char c)
    : m_value(static_cast<unsigned char>(c))
  {
    xassert(m_value >= 0);
  }

  CodePoint(CodePoint const &obj) noexcept = default;
  CodePoint &operator=(CodePoint const &obj) noexcept = default;

  // Comparison of CodePoints has the "absent" value less than all
  // others.
  #define MAKE_CODEPOINT_RELOP(op)                        \
    bool operator op(CodePoint const &obj) const noexcept \
      { return m_value op obj.m_value; }

  MAKE_CODEPOINT_RELOP(==)
  MAKE_CODEPOINT_RELOP(!=)
  MAKE_CODEPOINT_RELOP(< )
  MAKE_CODEPOINT_RELOP(<=)
  MAKE_CODEPOINT_RELOP(> )
  MAKE_CODEPOINT_RELOP(>=)

  #undef MAKE_CODEPOINT_RELOP

  // True if the object holds a valid code point value.
  bool has_value() const
  {
    return m_value != -1;
  }

  // Return the value.
  //
  // Requires: `has_value()`.
  int value() const
  {
    xassertPrecondition(has_value());
    return m_value;
  }

  // Return the value, or -1 if there is none.
  int valueOrN1() const
  {
    return m_value;
  }

  // Write the integer value or -1 if there is none.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, CodePoint const &obj)
    { obj.write(os); return os; }
};


// -------------------- Unicode general category --------------------
// The intent of this module is to eventually conform to the Unicode
// standard, but the implementation does not do so at this time for code
// points outside [0,127].

// The functions in the "category" section accept an absent `CodePoint`
// and return false.

// Unicode general category "Letter, uppercase".
bool isUppercaseLetter(CodePoint c);

// Unicode general category "Letter, lowercase".
bool isLowercaseLetter(CodePoint c);

// Unicode general category "Letter".
bool isLetter(CodePoint c);

// Unicode general cetegory "Number, decimal digit".
bool isDecimalDigit(CodePoint c);

// Unicode property White_Space=yes.
bool isWhitespace(CodePoint c);

// True if `c` is in [0xD800,0xDC00).
bool isHighSurrogate(CodePoint c);

// True if `c` is in [0xDC00,0xE000).
bool isLowSurrogate(CodePoint c);


// ------------------------ My own categories -----------------------
// This section has code point categories unrelated to those in
// Unicode.  They also return false for an absent `CodePoint`.

// True if 'c' is a character than can appear in an identifier in the
// C programming language.
bool isCIdentifierCharacter(CodePoint c);

// True if `c` can appear at the start of a C identifier.
bool isCIdentifierStartCharacter(CodePoint c);

// True if 'c' is considered whitespace in C.
bool isCWhitespace(CodePoint c);

// True if 'c' is one of the printable ASCII characters.  Note that, as
// best I can tell, the printable range is the same for all national
// ASCII variants as it is for US-ASCII, namely [32,126].
bool isASCIIPrintable(CodePoint c);

// True if 'c' is in ['0','9'].
bool isASCIIDigit(CodePoint c);

// True if `c` is in ['A','Z'].
bool isASCIIUppercaseLetter(CodePoint c);

// True if `c` is in ['a','z'].
bool isASCIILowercaseLetter(CodePoint c);

// True if `c` is an uppercase or lowercase US-ASCII letter.
bool isASCIILetter(CodePoint c);

// True if 'isASCIIDigit' or 'c' is in ['A','F'] or ['a','f'].
bool isASCIIHexDigit(CodePoint c);

// True if 'c' is in ['0','7'].
bool isASCIIOctDigit(CodePoint c);

// True if 'c' is a POSIX or Bash shell metacharacter, including space,
// under the assumption that IFS has its usual value.
bool isShellMetacharacter(CodePoint c);


// ---------------------------- Conversions ----------------------------
// If 'c' encodes a lowercase letter in US-ASCII, return the
// corresponding uppercase letter code according to American English.
// Otherwise, return 'c'.
int convertUSASCIIToUpper(int c);


// ------------------------------ Decoders -----------------------------
// In contrast to the categorization functions, the decoders require
// that the given `CodePoint` have a value.

// Map a hex digit to [0,15].
//
// Requires: `c.has_value()`.
int decodeASCIIHexDigit(CodePoint c);

// Given `highSurrogate` in [0xD800,0xDC00) and `lowSurrogate` in
// [0xDC00,0xE000), decode them as a single code point in
// [0x10000,0x10FFFF].
CodePoint decodeSurrogatePair(
  CodePoint highSurrogate, CodePoint lowSurrogate);

/* Implement the following map:

     input          output
     ----------     ------
     'b' or 'B'          2
     'o' or 'O'          8
     'x' or 'X'         16
     else                0

   This is meant for use as part of an integer decoder.

   Requires: `c.has_value()`.
*/
int decodeRadixIndicatorLetter(CodePoint c);

// If `c` denotes a digit value in `radix`, return that value.
// Otherwise return -1.  When `radix > 10`, letters case-insensitively
// denote digits starting with 'A' denoting 10.
//
// Requires: `c.has_value()`.
// Requires: `radix` in [2,36].
int decodeASCIIRadixDigit(CodePoint c, int radix);

// True if `decodeASCIIRadixDigit` would return non-negative.
//
// Requires: `c.has_value()`.
bool isASCIIRadixDigit(CodePoint c, int radix);

// If `radix` is one of those for which there is a special radix
// prefix code letter, return that letter.  Otherwise return 0.
char encodeRadixIndicatorLetter(int radix);


#endif // SMBASE_CODEPOINT_H
