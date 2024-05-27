// sm-ap-uint.h
// `APUInteger`, an arbitrary-precision unsigned integer class.
//
// This is primarily meant to be used as part of the implementation of
// `APInteger` defined in `sm-ap-int.h`.

// This file is in the public domain.

#ifndef SMBASE_SM_AP_UINT_H
#define SMBASE_SM_AP_UINT_H

#include "codepoint.h"                 // isASCIIHexDigit, decodeASCIIHexDigit
#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "div-up.h"                    // div_up
#include "double-width-type.h"         // smbase::DoubleWidthType
#include "exc.h"                       // xformatsb
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB
#include "string-utils.h"              // singleQuoteChar
#include "vector-utils.h"              // vectorReverseOf
#include "xassert.h"                   // xassert

#include <algorithm>                   // std::max
#include <cstddef>                     // std::ptrdiff_t
#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string_view>                 // std::string_view
#include <type_traits>                 // std::{is_integral, is_signed, is_unsigned}
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


/* Arbitrary-precision unsigned integer represented as a sequence of
   `Word`, which must be an unsigned type.

   The main reasin this is a template with the choice of word type
   abstract is so I can easily test the code using a small word size and
   then use a larger one in production.  I'm thinking I will wrap this
   class with another that hides all the template stuff and fixes the
   choice of word, presumably to `uint32_t`.
*/
template <typename Word>
class APUInteger {
public:      // types
  // An index used to access elements of `m_vec`.  This is a signed
  // quantity so that downward iteration is more convenient since we can
  // stop when it's negative rather than using other contorted tests.
  typedef std::ptrdiff_t Index;

  // I currently assume I have access to a double-word type.
  typedef typename DoubleWidthType<Word>::DWT DWord;

private:     // data
  /* The magnitude of the integer, from least significant to most
     significant (similar to "little endian").  That is, the represented
     magnitude is:

       Sum over i of: m_vec[i] * (N ** i)

     where N is one larger than the largest value of Word.

     This can be empty for the case of zero, but is not required to be.

     The reason I do not insist on a normal form currently is it would
     potentially make the sequence-of-words (`setWord`, etc.) and
     sequence-of bits (`setBit`, etc.) interfaces awkward due to perhaps
     doing needless trimming.  But this is a decision I might revisit.
  */
  std::vector<Word> m_vec;

private:     // methods
  // ---------- Operations on Words ----------
  // Return the number of bits in each word.
  static constexpr Index bitsPerWord()
  {
    return sizeof(Word) * 8;
  }

  // Add `other` into `w`, returning the carry bit.
  static Word addWithCarry(Word &w, Word other)
  {
    w += other;    // May wrap.
    return w < other;
  }

  // Subtract `b` from `a`, returning 1 if that requires borrowing one
  // unit from the next-highest word.
  static Word subtractWithBorrow(Word &a, Word b)
  {
    Word origA = a;
    a -= b;        // May wrap.
    return a > origA;
  }

  // Return `a*b` in two words.
  static void multiplyWords(
    Word &lowProd,
    Word &highProd,
    Word a,
    Word b)
  {
    // For now, assume we have access to a double-word type.
    DWord da = a;
    DWord db = b;
    DWord prod = da*db;
    lowProd = (Word)prod;
    highProd = (Word)(prod >> bitsPerWord());
  }

  // ---------- Arithmetic helpers ----------
  // Add `other` into `this`.
  void add(APUInteger const &other)
  {
    // Carry value from the previous word.
    Word carry = 0;

    // Process all of the words in `other`, then keep going as long as
    // we have a carry to propagate.
    Index i = 0;
    for (; i < other.size() || carry != 0; ++i) {
      // Get current word.
      Word d = this->getWord(i);

      // Add the carry and the other number's word.
      Word carry1 = addWithCarry(d, carry);
      Word carry2 = addWithCarry(d, other.getWord(i));

      // Update the word.
      this->setWord(i, d);

      // Compute the carry to use in the next iteration.
      carry = carry1 + carry2;

      // It is not possible for both additions to yield a carry because
      // if the first does, then the resulting `d` is zero, so the
      // second addition yields `other.getWord(i)` with no carry.
      xassert(carry <= 1);
    }
  }

  // Subtract `other` from `this`.  `this` must be at least as large.
  void subtract(APUInteger const &other)
  {
    // Amount to borrow from the current word in order to supply the
    // previous iteration's word.
    Word borrow = 0;

    for (Index i = 0; i < other.size() ||
                      (i < this->size() && borrow != 0); ++i) {
      Word d = this->getWord(i);

      Word borrow1 = subtractWithBorrow(d, borrow);
      Word borrow2 = subtractWithBorrow(d, other.getWord(i));

      this->setWord(i, d);

      borrow = borrow1 + borrow2;

      // It is not possible for both operations to yield a borrow
      // because if the first does, then it leaves `d` as the maximum
      // value of a Word, so the second subtraction cannot require a
      // borrow.
      xassert(borrow <= 1);
    }

    // Otherwise, `other` was larger.
    xassert(borrow == 0);
  }

  // ---------- Serialization helpers ----------
  // Write `w` to `os` as hexadecimal, possibly with `leadingZeroes`.
  static void writeWordAsHex(std::ostream &os, Word w, bool leadingZeroes)
  {
    // In order to avoid altering the formatting state of `os`, use a
    // temporary stream object to write to the same stream buffer.
    // https://stackoverflow.com/a/43771309/2659307
    std::ostream tmpOS(os.rdbuf());

    // The `setf` interface is quite awkward because you have to both
    // say what bits you want and also specify a mask designating which
    // parts of the existing flags to alter.
    tmpOS.setf(
      std::ios_base::hex       | std::ios_base::uppercase | std::ios_base::right,
      std::ios_base::basefield | std::ios_base::uppercase | std::ios_base::adjustfield);

    if (leadingZeroes) {
      // Two digits per byte.
      tmpOS.width(sizeof(Word)*2);
      tmpOS.fill('0');
    }

    if (sizeof(Word) == 1) {
      // The `char` types need to be treated as integer here to get
      // digits rather than a single character.
      tmpOS << (int)w;
    }
    else {
      tmpOS << w;
    }
  }

  // Interpret the hexadecimal digits in [start,end) as denoting a Word
  // value.  There must not be more digits than could fit in a Word.
  // Throws XFormat if a character is not hexadecimal.
  static Word wordFromHexDigit(std::string_view digits)
  {
    Index digitsPerWord = sizeof(Word)*2;
    Index numDigits = digits.size();
    xassert(numDigits <= digitsPerWord);

    // Value computed so far.
    Word w = 0;

    // Left shift amount to apply to the next digit to place.
    int shiftAmount = 0;

    // Work from least to most significant digit.
    for (Index i = numDigits-1; i >= 0; --i) {
      int c = digits[i];
      if (!isASCIIHexDigit(c)) {
        xformatsb("Expecting hexadecimal digit, instead found " <<
                  singleQuoteChar(c));
      }
      Word v = decodeASCIIHexDigit(c);

      w |= v << shiftAmount;

      shiftAmount += 4;
    }

    return w;
  }

  /* Return `value` as a digit in base `radix`.  For a radix larger than
     10, digit 11 is 'A', digit 12 is 'B', and so on up to digit 36 as
     'Z'.  When the result is a letter, it is always uppercase.

     Preconditions:
       2 <= radix <= 36
       0 <= value < radix
  */
  static char getAsRadixDigit(int value, int radix)
  {
    xassert(2 <= radix && radix <= 36);
    xassert(0 <= value && value < radix);

    if (value < 10) {
      return '0' + value;
    }
    else {
      return 'A' + (value - 10);
    }
  }

  /* Regard `digit` as a digit in base `radix` and return its numeric
     value.  `radix` must be in [2,36].

     If `digit` is not valid for the radix, throw `XFormat`.
  */
  static Word wordFromRadixDigit(int digit, int radix)
  {
    xassert(2 <= radix && radix <= 36);

    // First try to map the digit to a value without regard for radix.
    int dv = -1;
    if ('0' <= digit && digit <= '9') {
      dv = digit - '0';
    }
    else if ('A' <= digit && digit <= 'Z') {
      dv = digit - 'A' + 10;
    }
    else if ('a' <= digit && digit <= 'z') {
      dv = digit - 'a' + 10;
    }

    // If it was not a digit or letter, or was but the denoted value is
    // too large, complain.
    if (dv < 0 || dv >= radix) {
      xformatsb("Expecting a base-" << radix <<
                " digit, instead found " << singleQuoteChar(digit));
    }

    return (Word)dv;
  }

  /* If `c` is one of the letters that can follow a leading '0' to
     indicate the radix, return the denoted radix.  Otherwise return 0.
  */
  static int decodeRadixIndicatorLetter(char c)
  {
    switch (c) {
      case 'b':
      case 'B':
        return 2;

      case 'o':
      case 'O':
        return 8;

      case 'x':
      case 'X':
        return 16;

      default:
        return 0;
    }
  }

  /* If `radix` is one of those for which there is a special radix
     prefix code letter, return that letter.  Otherwise return 0.
  */
  static char encodeRadixIndicatorLetter(int radix)
  {
    switch (radix) {
      case 2:      return 'b';
      case 8:      return 'o';
      case 16:     return 'x';
      default:     return 0;
    }
  }

public:      // methods
  // ---------- Constructors ----------
  // Zero.
  APUInteger()
    : m_vec()
  {
    // May as well check this here.
    static_assert(std::is_integral<Word>::value);
    static_assert(std::is_unsigned<Word>::value);
  }

  APUInteger(APUInteger const &obj)
    : DMEMB(m_vec)
  {}

  APUInteger(APUInteger &&obj)
    : MDMEMB(m_vec)
  {}

  // Construct from `PRIM`, presumed to be a primitive type.  The
  // argument must be non-negative.  As this preserves information, it
  // is allowed to be invoked implicitly.
  template <typename PRIM>
  APUInteger(PRIM n)
    : m_vec()
  {
    if (n < 0) {
      xmessage(stringb(
        "Attempted to create an APUInteger from negative value " <<
        n << "."));
    }

    if (sizeof(Word) >= sizeof(PRIM)) {
      setWord(0, (Word)n);
    }
    else {
      // `PRIM` is larger, so we need to divide it into word-sized
      // pieces.  Set words from least to most significant.
      for (Index i = 0; n != 0; ++i) {
        // Store the low bits of `n`.
        setWord(i, (Word)n);

        // Reduce its value correspondingly.
        n >>= bitsPerWord();
      }
    }
  }

  // ---------- Assignment ----------
  APUInteger &operator=(APUInteger const &obj)
  {
    if (this != &obj) {
      CMEMB(m_vec);
    }
    return *this;
  }

  APUInteger &operator=(APUInteger &&obj)
  {
    if (this != &obj) {
      MCMEMB(m_vec);
    }
    return *this;
  }

  // ---------- Zero ----------
  // True if this object represents zero.
  bool isZero() const
  {
    return this->maxWordIndex() == -1;
  }

  // Set the value of this object to zero.
  void setZero()
  {
    m_vec.clear();
  }

  // ---------- Convert to primitive ----------
  // Get as a primitive type, or `nullopt` if it will not fit.  That
  // includes the case where `PRIM` is a signed type and the value would
  // naively set its high bit; the result of this call is always
  // non-negative if it is not `nullopt`.
  template <typename PRIM>
  std::optional<PRIM> getAsOpt() const
  {
    static_assert(std::is_integral<PRIM>::value);

    Index maxWIndex = this->maxWordIndex();
    if (maxWIndex == -1) {
      return PRIM();         // Zero.
    }

    Index bitsPerPrim = sizeof(PRIM) * 8;
    if (std::is_signed<PRIM>::value) {
      bool highBit = this->getBit(bitsPerPrim - 1);
      if (highBit) {
        // The high bit is set, does not fit in a signed integer.
        return std::nullopt;
      }
    }

    // I assume that `PRIM` is smaller than `Word`, or its size is an
    // integer multiple of the word size.
    static_assert(sizeof(PRIM) < sizeof(Word) ||
                  sizeof(PRIM) % sizeof(Word) == 0);

    if (sizeof(Word) >= sizeof(PRIM)) {
      if (maxWIndex > 0) {
        // Too many words, does not fit.
        return std::nullopt;
      }

      Word w = this->getWord(0);

      if (sizeof(Word) > sizeof(PRIM)) {
        Word primMask = (1 << bitsPerPrim) - 1;
        Word masked = w & primMask;

        if (masked != w) {
          // There are bits in `w` that are beyond what `PRIM` can
          // store.
          return std::nullopt;
        }

        return PRIM(masked);
      }
      else {
        return PRIM(w);
      }
    }

    else /* sizeof(Word) < sizeof(PRIM) */ {
      // Note that the `static_assert` above ensures that this division
      // produces no remainder.
      Index wordsPerPrim = sizeof(PRIM) / sizeof(Word);
      if (maxWIndex >= wordsPerPrim) {
        // Too many words, does not fit.
        return std::nullopt;
      }

      PRIM ret = PRIM();

      // Populate `ret` from least to most significant word.
      for (Index i = 0; i <= maxWIndex; ++i) {
        PRIM v(this->getWord(i));
        v <<= i * bitsPerWord();
        ret |= v;
      }

      return ret;
    }
  }

  // Same as `getAsOpt()`, but throwing an exception if it does not fit.
  template <typename PRIM>
  PRIM getAs() const
  {
    std::optional<PRIM> res = getAsOpt<PRIM>();
    if (!res.has_value()) {
      throwDoesNotFitException<PRIM>("APUInteger", this->toString());
    }
    return res.value();
  }

  // Throw an exception complaining about the inability to convert a
  // value to `PRIM`.  The class name is a parameter so this can be used
  // by `APInteger` too.
  template <typename PRIM>
  static void throwDoesNotFitException(
    char const *className,
    std::string const &valueAsString)
  {
    xmessage(stringb(
      "Attempted to convert the " << className <<
      " value " << valueAsString << " to " <<
      (std::is_signed<PRIM>::value? "a signed " : "an unsigned ") <<
      (sizeof(PRIM)*8) << "-bit integer type, but it does not fit."));
  }

  // ---------- Treat as a sequence of Words ----------
  // Return the number of stored words.  Some of the high words may
  // be redundantly zero, but this method does not check for that.
  Index size() const
  {
    return static_cast<Index>(m_vec.size());
  }

  // Maximum index that contains a non-zero word.  If the value is zero
  // then this is -1.
  Index maxWordIndex() const
  {
    Index i = size() - 1;
    while (i >= 0 && m_vec[i] == 0) {
      --i;
    }
    return i;
  }

  // Get the `i`th word, where the 0th is the least significant.
  // Return 0 if that word is not currently stored.
  Word getWord(Index i) const
  {
    xassert(i >= 0);
    if (i < size()) {
      return m_vec[i];
    }
    else {
      return 0;
    }
  }

  // Set word `i` to `d`.  If `d` is zero and `i` is beyond the current
  // vector size, do nothing.
  void setWord(Index i, Word d)
  {
    xassert(i >= 0);
    if (i < size()) {
      m_vec[i] = d;
    }
    else if (d == 0) {
      // There is no need to explicitly set a zero word beyond the
      // current size.
    }
    else {
      // Add zeroes until we can place `d`.
      while (i >= size()) {
        m_vec.push_back(0);
      }

      m_vec[i] = d;
    }
  }

  // Multiply `*this` by `N ** amount`.
  void leftShiftByWords(Index amount)
  {
    xassert(amount >= 0);
    m_vec.insert(m_vec.begin(), amount, 0);
  }

  // ---------- Treat as a sequence of bits ----------
  // Index of the highest bit set to 1.  Returns -1 if the value of
  // `*this` is zero.
  Index maxBitIndex() const
  {
    Index i = (maxWordIndex()+1) * bitsPerWord() - 1;
    while (i >= 0) {
      if (this->getBit(i)) {
        return i;
      }
      --i;
    }
    return i;
  }

  // True if bit `i` is set, where bit 0 is the least significant.
  bool getBit(Index i) const
  {
    xassert(i >= 0);

    Index wordIndex = i / bitsPerWord();
    Index bitIndexWithinWord = i % bitsPerWord();

    Word w = getWord(wordIndex);
    return ((w >> bitIndexWithinWord) & 1) == 1;
  }

  // Set the bit at `i` to `b`.
  void setBit(Index i, bool b)
  {
    xassert(i >= 0);

    Index wordIndex = i / bitsPerWord();
    Index bitIndexWithinWord = i % bitsPerWord();

    Word w = getWord(wordIndex);
    Word bit = (Word)1 << bitIndexWithinWord;
    if (b) {
      w |= bit;
    }
    else {
      w &= ~bit;
    }
    setWord(wordIndex, w);
  }

  // Multiply `*this` by `2**amt`.
  void leftShiftByBits(Index amt)
  {
    xassert(amt >= 0);
    Index wordShiftAmt = amt / bitsPerWord();
    Index bitShiftAmt = amt % bitsPerWord();

    leftShiftByWords(wordShiftAmt);

    if (bitShiftAmt == 0) {
      return;
    }

    Index complementedBitShiftAmt = bitsPerWord() - bitShiftAmt;
    xassert(0 <= complementedBitShiftAmt &&
                 complementedBitShiftAmt < bitsPerWord());

    for (Index i = maxWordIndex()+1; i >= 0; --i) {
      // Compute the new word at `i` by shifting its current contents
      // left, then bringing in upper bits of the next word down.
      Word w = getWord(i);
      Word next = (i>0? getWord(i-1) : 0);

      Word wNew = (w << bitShiftAmt) | (next >> complementedBitShiftAmt);
      setWord(i, wNew);
    }
  }

  // Divide `*this` by two.
  void rightShiftOneBit()
  {
    Index complementedBitShiftAmt = bitsPerWord() - 1;

    for (Index i=0; i <= maxWordIndex(); ++i) {
      // Compute the new word at `i` by right shifting it one place,
      // then bringing in the low bit from the next word up.
      Word w = getWord(i);
      Word next = getWord(i+1);

      Word wNew = (w >> 1) | (next << complementedBitShiftAmt);
      setWord(i, wNew);
    }
  }

  // ---------- Relational comparison ----------
  // Return <0 if a<b, 0 if a==b, >0 if a>b.
  friend int compare(APUInteger const &a,
                     APUInteger const &b)
  {
    Index aMaxIndex = a.maxWordIndex();
    Index bMaxIndex = b.maxWordIndex();

    RET_IF_COMPARE(aMaxIndex, bMaxIndex);

    for (Index i = aMaxIndex; i >= 0; --i) {
      RET_IF_COMPARE(a.getWord(i), b.getWord(i));
    }

    return 0;
  }

  DEFINE_FRIEND_RELATIONAL_OPERATORS(APUInteger)

  // ---------- Convert to sequence of hexadecimal digits ----------
  // Write to `os` the hexadecimal digits of this number.  If
  // `withRadixMarker` is true then also print a leading "0x".
  void writeAsHex(std::ostream &os, bool withRadixMarker = true) const
  {
    if (withRadixMarker) {
      os << "0x";
    }

    Index maxIndex = maxWordIndex();
    if (maxIndex < 0) {
      os << '0';
      return;
    }

    for (Index i = maxIndex; i >= 0; --i) {
      // The first word does not get leading zeroes.
      bool leadingZeroes = i < maxIndex;

      writeWordAsHex(os, getWord(i), leadingZeroes);
    }
  }

  // The ordinary format for writing is hex because then we do not have
  // to do the complicated and expensive process of converting to
  // decimal.  It includes a leading "0x" radix marker.
  //
  // It does not currently respond to any formatting configuration of
  // the stream.  Maybe it should?  But then I would be writing decimal
  // by default, which I do not want.
  //
  friend std::ostream &operator<<(std::ostream &os, APUInteger const &n)
  {
    n.writeAsHex(os);
    return os;
  }

  // Return the same string that `operator<<` would produce.
  std::string toString() const
  {
    return stringb(*this);
  }

  // Return a string of hex digits without the "0x" prefix.
  std::string getAsHexDigits() const
  {
    std::ostringstream oss;
    writeAsHex(oss, false /*writeRadixMarker*/);
    return oss.str();
  }

  // ---------- Convert from sequence of hexadecimal digits ----------
  /* Interpret `digits` as a sequence of hexadecimal digits, *without*
     any radix marker, and return the value they denote.

     If `digits` is empty, return zero.

     Throw `XFormat` if there is a problem.
  */
  static APUInteger fromHexDigits(std::string_view digits)
  {
    APUInteger ret;

    // Running example (RE):
    //
    //   `Word` is `uint8_t`
    //   `digits` is "12345"

    Index numDigits = digits.size();                       // RE: 5
    Index digitsPerWord = sizeof(Word) * 2;                // RE: 2
    Index numWords = div_up(numDigits, digitsPerWord);     // RE: 3

    // Go from least to most significant word.
    for (Index i = 0; i < numWords; ++i) {                 // RE: i from 0 to 2 (inclusive)
      // Index one past the last digit in the block corresponding to
      // word `i`.  For the RE:
      //
      //   When i=0, this is 5.
      //   When i=1, this is 3.
      //   When i=2, this is 1.
      //
      Index digitBlockEnd = numDigits - i*digitsPerWord;

      // Index of the first digit in the block.  For the RE:
      //
      //   When i=0, this is 3.
      //   When i=1, this is 1.
      //   When i=2, this is 0.
      //
      Index digitBlockStart = std::max(digitBlockEnd - digitsPerWord,
                                       (Index)0);

      // For the RE, this is 2, 2, then 1.
      Index digitBlockSize = digitBlockEnd - digitBlockStart;
      xassert(0 < digitBlockSize &&
                  digitBlockSize <= digitsPerWord);

      // Decode the block of digits.  For the RE:
      //
      //   When i=0, this is 0x45.
      //   When i=1, this is 0x23.
      //   When i=2, this is 0x01.
      //
      Word w = wordFromHexDigit(
        digits.substr(digitBlockStart, digitBlockSize));

      // Store that.
      ret.setWord(i, w);
    }

    return ret;
  }

  // ---------- Convert to sequence of arbitrary-radix digits ----------
  /* Return a string containing the digits of `*this` using `radix`,
     which must be in [2,36].  No indicator of the radix is returned.

     This is a fairly slow procedure since it uses repeated division,
     although the case of `radix==16` is comparatively fast.
  */
  std::string getAsRadixDigits(int radix) const
  {
    if (radix == 16) {
      return getAsHexDigits();
    }
    else {
      return getAsRadixDigits_noFastPath(radix);
    }
  }

  // Slow case.  This is exposed just so I can compare it to
  // `getAsHexDigits()` in the unit tests.
  std::string getAsRadixDigits_noFastPath(int radix) const
  {
    xassert(2 <= radix && radix <= 36);

    if (isZero()) {
      return "0";
    }

    // Accumulate the digits, least significant first.
    std::vector<char> digits;

    APUInteger apRadix(radix);

    // Remaining value to print.
    APUInteger n(*this);
    while (!n.isZero()) {
      // Divide by the radix.
      APUInteger quotient, remainder;
      divide(quotient, remainder, n, apRadix);

      // The remainder is the digit to print this time.
      digits.push_back(
        getAsRadixDigit(remainder.template getAs<int>(), radix));

      // The quotient is what remains to be printed.
      n = std::move(quotient);
    }

    // Reverse the digits to get the most significant first.
    std::vector revDigits = vectorReverseOf(digits);

    return std::string(revDigits.begin(), revDigits.end());
  }

  // Return `*this` as a string of decimal rights.
  std::string getAsDecimalDigits() const
  {
    return getAsRadixDigits(10);
  }

  // Return a string of digits with the radix and its associated prefix.
  // `radix` must be 2, 8, 10, or 16.
  std::string getAsRadixPrefixedDigits(int radix) const
  {
    // Determine what prefix to use, if any.
    char letter = encodeRadixIndicatorLetter(radix);

    // Write the prefix.
    std::ostringstream oss;
    if (letter == 0) {
      xassert(radix == 10);
    }
    else {
      oss << '0' << letter;
    }

    // Write the rest.
    oss << getAsRadixDigits(radix);
    return oss.str();
  }

  // --------- Convert from sequence of arbitrary-radix digits ---------
  /* Treat `digits` as a sequence of digits in base `radix` and return
     the value they denote.  `radix` must be in [2,36].

     If `digits` is empty, return zero.

     If any digit is invalid, throw `XFormat`.
  */
  static APUInteger fromRadixDigits(std::string_view digits, int radix)
  {
    APUInteger ret;

    Word radixWord = (Word)radix;
    xassert((int)radixWord == radix);

    // Object into which I store successive digit values in order to add
    // them into `ret`.  This avoids creating and destroying an object
    // for each digit.
    APUInteger apDigit;

    // Work left to right.
    for (char digit : digits) {
      apDigit.setWord(0, wordFromRadixDigit(digit, radix));

      ret.multiplyWord(radixWord);
      ret.add(apDigit);
    }

    return ret;
  }

  static APUInteger fromDecimalDigits(std::string_view digits)
  {
    return fromRadixDigits(digits, 10);
  }

  /* Check for one of the recognized radix prefixes in `digits`.  If one
     is found, return its associated radix as one of {2, 8, 16}.
     Otherwise, return 0.

     This does not return 10 for the case of no prefix because the
     caller needs to handle an actual prefix differently by skipping it
     before interpreting the digits.
  */
  static int detectRadixPrefix(std::string_view digits)
  {
    if (digits.size() >= 3 && digits[0] == '0') {
      if (int radix = decodeRadixIndicatorLetter(digits[1])) {
        return radix;
      }
    }
    return 0;
  }

  /* Convert `digits` to an integer.  It is expected to be prefixed with
     a radix indicator, from among:

       0b   - binary
       0o   - octal
       0x   - hex
       else - decimal

     The 'b', 'o', and 'x' are case-insensitive.

     An empty string is treated as zero.

     If it does not have any of those forms, throw `XFormat`.  That
     includes the case where "0b", "Oo", or "0x" is not followed by
     anything.

     Note: The "Oo" syntax is not what C or C++ uses, although some
     other languages do.  Thus, the "radix prefix" used by this class is
     not compatible with C/C++ lexical convention.

     Why does this prefix interpretation stuff even belong in this
     class?  Well, I want sensible behavior from `operator<<`, hex is
     better for basic printing due to vastly simpler logic, I consider
     unprefixed hex too potentially confusing, and if I write a prefix
     then I should be able to read it too.  So here we are.
  */
  static APUInteger fromRadixPrefixedDigits(std::string_view digits)
  {
    if (int radix = detectRadixPrefix(digits)) {
      return fromRadixDigits(digits.substr(2), radix);
    }
    else {
      // No recognized radix indicator, use decimal.
      return fromDecimalDigits(digits);
    }
  }

  // There is no `operator>>` because I regard C++ `istream` formatted
  // input as completely inadequate as a parsing framework.  Something
  // else should parse, then hand this class a string (view).

  // ---------- Addition ----------
  // Add `other` to `*this`.
  APUInteger &operator+=(APUInteger const &other)
  {
    this->add(other);
    return *this;
  }

  APUInteger operator+(APUInteger const &other) const
  {
    APUInteger ret(*this);
    return ret += other;
  }

  // ---------- Subtraction ----------
  // Subtract `other` from `*this`.  If `other` is larger, then set
  // `*this` to zero.
  APUInteger &operator-=(APUInteger const &other)
  {
    if (*this >= other) {
      this->subtract(other);
    }
    else {
      this->setZero();
    }
    return *this;
  }

  APUInteger operator-(APUInteger const &other) const
  {
    APUInteger ret(*this);
    return ret -= other;
  }

  // ---------- Multiplication ----------
  // Set `*this` to the product of its original value and `w`.
  void multiplyWord(Word w)
  {
    // Amount to add from the previous iteration.
    Word carry = 0;

    for (Index i = 0; i < size() || carry; ++i) {
      Word d = this->getWord(i);

      Word lowProd, highProd;
      multiplyWords(lowProd, highProd, d, w);

      // The low word of the product goes into the `i`th slot.
      d = lowProd;

      // Plus whatever carries from the previous word.
      Word carry1 = addWithCarry(d, carry);
      this->setWord(i, d);

      // Then that carry combines with the high word.
      Word carry2 = addWithCarry(highProd, carry1);

      // It should not be possible for the second addition to overflow.
      xassert(carry2 == 0);

      // What is in `highProd` is what carries to the next word.
      carry = highProd;
    }
  }

  // Return the product of `*this` and `other`.
  APUInteger operator*(APUInteger const &other) const
  {
    APUInteger acc;

    for (Index i = 0; i < other.size(); ++i) {
      // Compute `*this * (N**i) * other[i]`.
      APUInteger partialSum(*this);
      partialSum.leftShiftByWords(i);
      partialSum.multiplyWord(other.getWord(i));

      // Add it to the running total.
      acc += partialSum;
    }

    return acc;
  }

  APUInteger &operator*=(APUInteger const &other)
  {
    APUInteger prod = *this * other;
    return *this = prod;
  }

  // ---------- Division ----------
  /* Compute `quotient`, the maximum number of times that `divisor`
     goes into `dividend`, and `remainder`, what is left over after
     taking that many divisors out.

     The operands must all be distinct objects, except that `dividend`
     and `divisor` could be the same.

     Precondition:

       divisor > 0
       distinct(&quotient, &remainder, {&dividend, &divisor})

     Postcondition:

       0 <= remainder < divisor
       divisor * quotient + remainder = dividend
  */
  static void divide(
    APUInteger &quotient,
    APUInteger &remainder,
    APUInteger const &dividend,        // aka numerator
    APUInteger const &divisor)         // aka denominator
  {
    // TODO: Throw a better exception.
    xassert(!divisor.isZero());

    // We will set bits in the quotient as we go.
    quotient.setZero();

    // Work with the dividend as it will exist after the divisor takes
    // chunks out of it.
    remainder = dividend;

    if (dividend.isZero()) {
      return;
    }

    // We will work one bit at a time.  This is slow but simple.
    Index s = dividend.maxBitIndex();

    // Work with a divisor shifted left by `s` bits.
    APUInteger shiftedDivisor(divisor);
    shiftedDivisor.leftShiftByBits(s);

    while (s >= 0) {
      // Can `shiftedDivisor` go into what remains?
      if (shiftedDivisor <= remainder) {
        // Yes, take another chunk out of it.
        remainder -= shiftedDivisor;
        quotient.setBit(s, true);
      }
      else {
        // No, remainder stays.
        //
        // We do not actually need to call `setBit(false)` since the
        // quotient started as all zeroes.
        //quotient.setBit(s, false);
      }

      // Move on to the next smaller divisor and a correspondingly less
      // significant quotient bit.
      shiftedDivisor.rightShiftOneBit();
      --s;
    }
  }

  APUInteger operator/(APUInteger const &divisor) const
  {
    APUInteger quotient, remainder;
    divide(quotient, remainder, *this, divisor);
    return quotient;
  }

  APUInteger operator%(APUInteger const &divisor) const
  {
    APUInteger quotient, remainder;
    divide(quotient, remainder, *this, divisor);
    return remainder;
  }

  APUInteger &operator/=(APUInteger const &divisor)
  {
    APUInteger q = *this / divisor;
    return *this = q;
  }

  APUInteger &operator%=(APUInteger const &divisor)
  {
    APUInteger r = *this % divisor;
    return *this = r;
  }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_AP_UINT_H
