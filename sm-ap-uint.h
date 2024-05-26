// sm-ap-uint.h
// Arbitrary-precision unsigned integer arithmetic.

// This file is in the public domain.

#ifndef SMBASE_SM_AP_UINT_H
#define SMBASE_SM_AP_UINT_H

#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "double-width-type.h"         // smbase::DoubleWidthType
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB
#include "xassert.h"                   // xassert

#include <cstddef>                     // std::ptrdiff_t
#include <iostream>                    // std::ostream
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Arbitrary-precision unsigned integer represented as a sequence of
// `Word`, which must be an unsigned type.
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

     This can be empty for the case of zero.
  */
  std::vector<Word> m_vec;

private:     // methods
  // ---------- Operations on Words ----------
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

public:      // methods
  // ---------- Constructors ----------
  // Zero.
  APUInteger()
    : m_vec()
  {}

  APUInteger(APUInteger const &obj)
    : DMEMB(m_vec)
  {}

  APUInteger(APUInteger &&obj)
    : MDMEMB(m_vec)
  {}

  // Represent `smallMagnitude`.
  //
  // This allows implicit conversion because it is safe (no external
  // side effects, etc.) and preserves information.
  /*implicit*/ APUInteger(Word smallMagnitude)
    : m_vec(1, smallMagnitude)
  {}

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

  // ---------- General ----------
  // Return the number of bits in each word.
  static constexpr Index bitsPerWord()
  {
    return sizeof(Word) * 8;
  }

  // True if this object represents zero.
  bool isZero() const
  {
    return maxWordIndex() == -1;
  }

  // Set the value of this object to zero.
  void setZero()
  {
    m_vec.clear();
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
      if (getBit(i)) {
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
  // decimal.
  friend std::ostream &operator<<(std::ostream &os, APUInteger const &n)
  {
    n.writeAsHex(os);
    return os;
  }

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

     Precondition:

       divisor > 0

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
