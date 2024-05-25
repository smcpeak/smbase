// sm-ap-uint.h
// Arbitrary-precision unsigned integer arithmetic.

// This file is in the public domain.

#ifndef SMBASE_SM_AP_UINT_H
#define SMBASE_SM_AP_UINT_H

#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB
#include "xassert.h"                   // xassert

#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Arbitrary-precision unsigned integer represented as a sequence of
// `Word`, which must be an unsigned type.
template <typename Word>
class APUInteger {
public:      // types
  // An index used to access elements of `m_vec`.
  typedef std::size_t Index;

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
  // Add `other` into `w`, returning the carry bit.
  static Word addWithCarry(Word &w, Word other)
  {
    w += other;
    return w < other;
  }

  // Add `other` into `this`.
  void add(APUInteger const &other)
  {
    // Carry value from the previous digit.
    Word carry = 0;

    // Process all of the digits in `other`, then keep going as long as
    // we have a carry to propagate.
    Index i = 0;
    for (; i < other.size() || carry != 0; ++i) {
      // Get current digit.
      Word d = this->getDigit(i);

      // Add the carry and the other number's digit.
      Word carry1 = addWithCarry(d, carry);
      Word carry2 = addWithCarry(d, other.getDigit(i));

      // Update the digit.
      this->setDigit(i, d);

      // Compute the carry to use in the next iteration.
      carry = carry1 + carry2;

      // It is not possible for both additions to yield a carry because
      // if the first does, then the resulting `d` is zero, so the
      // second addition yields `other.digit(i)` with no carry.
      xassert(carry <= 1);
    }
  }

public:      // methods
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

  // Represent `smallMagnitude`.
  //
  // This allows implicit conversion because it is safe (no external
  // side effects, etc.) and preserves information.
  /*implicit*/ APUInteger(Word smallMagnitude)
    : m_vec(1, smallMagnitude)
  {}

  // Return the number of stored digits.  Some of the high digits may
  // be redundantly zero, but this method does not check for that.
  Index size() const
  {
    return m_vec.size();
  }

  // Get the underlying vector.  This can be useful for debug printing.
  std::vector<Word> const &getVector() const
  {
    return m_vec;
  }

  // Get the `i`th digit, where the 0th is the least significant.
  // Return 0 if that digit is not currently stored.
  Word getDigit(Index i) const
  {
    if (i < m_vec.size()) {
      return m_vec[i];
    }
    else {
      return 0;
    }
  }

  // Set digit `i` to `d`.  If `d` is zero and `i` is beyond the current
  // vector size, do nothing.
  void setDigit(Index i, Word d)
  {
    if (i < m_vec.size()) {
      m_vec[i] = d;
    }
    else if (d == 0) {
      // There is no need to explicitly set a zero digit beyond the
      // current size.
    }
    else {
      // Add zeroes until we can place `d`.
      while (i >= m_vec.size()) {
        m_vec.push_back(0);
      }

      m_vec[i] = d;
    }
  }

  APUInteger<Word> &operator+=(APUInteger<Word> const &other)
  {
    this->add(other);
    return *this;
  }

  APUInteger<Word> operator+(APUInteger<Word> const &other) const
  {
    APUInteger<Word> ret(*this);
    return ret += other;
  }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_AP_UINT_H
