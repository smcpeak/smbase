// sm-ap-int.h
// `APInteger`, an arbitrary-precision integer class.

// This file is in the public domain.

#ifndef SMBASE_SM_AP_INT_H
#define SMBASE_SM_AP_INT_H

#include "compare-util.h"              // RET_IF_COMPARE
#include "sm-ap-uint.h"                // APUInteger
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB

#include <limits>                      // std::numeric_limits
#include <optional>                    // std::{optional, nullopt}
#include <string_view>                 // std::string_view
#include <type_traits>                 // std::{is_unsigned, make_unsigned}


OPEN_NAMESPACE(smbase)


// Arbitrary-precision integer, positive or negative.
template <typename Word>
class APInteger {
public:      // types
  // The unsigned type upon which this is based.
  typedef APUInteger<Word> UInteger;

private:     // data
  // Magnitude of the value.
  UInteger m_magnitude;

  // Sign of the value.  If the value is negative, then the magnitude is
  // not zero.
  bool m_negative;

private:     // methods
  /* If `m_magnitude` is zero and `m_negative` is set, clear it.

     I do this in order to allow `APInteger`s to be constructed with a
     negative flag despite being zero.  This arises naturally in the
     arithmetic operations (such as (-1) - (-1), or (-1) * 0), and it
     would annoying to add extra logic to avoid a problem that is easily
     solved by normalizing at construction time.
  */
  void fixNegativeZero()
  {
    if (m_negative && m_magnitude.isZero()) {
      m_negative = false;
    }
  }

public:      // methods
  // ---------- Constructors ----------
  // Zero.
  APInteger()
    : m_magnitude(),
      m_negative(false)
  {}

  APInteger(APInteger const &obj)
    : DMEMB(m_magnitude),
      DMEMB(m_negative)
  {}

  APInteger(APInteger &&obj)
    : MDMEMB(m_magnitude),
      MDMEMB(m_negative)
  {}

  APInteger(UInteger const &magnitude, bool negative)
    : m_magnitude(magnitude),
      m_negative(negative)
  {
    fixNegativeZero();
  }

  APInteger(UInteger &&magnitude, bool negative)
    : m_magnitude(std::move(magnitude)),
      m_negative(negative)
  {
    fixNegativeZero();
  }

  // Construct from `PRIM`, presumed to be a primitive type.  As this
  // preserves information, it is allowed to be invoked implicitly.
  template <typename PRIM>
  APInteger(PRIM n)
    : m_magnitude(),
      m_negative(false)
  {
    // True if we need to add one more to the magnitude at the end.
    bool addOneAfterward = false;

    if (n < 0) {
      m_negative = true;

      if (n == std::numeric_limits<PRIM>::min()) {
        // The naive strategy of getting a positive value by applying
        // the unary `-` operator would fail since that value cannot be
        // represented as a `PRIM`.
        ++n;
        addOneAfterward = true;
      }

      // Flip the sign of `n`.
      n = -n;
      xassert(n >= 0);
    }

    m_magnitude = n;

    if (addOneAfterward) {
      m_magnitude += 1;
    }

    selfCheck();
  }

  // ---------- Assignment ----------
  APInteger &operator=(APInteger const &obj)
  {
    if (this != &obj) {
      CMEMB(m_magnitude);
      CMEMB(m_negative);
    }
    return *this;
  }

  APInteger &operator=(APInteger &&obj)
  {
    if (this != &obj) {
      MCMEMB(m_magnitude);
      MCMEMB(m_negative);
    }
    return *this;
  }

  // ---------- General ----------
  // Assert invariants.
  void selfCheck() const
  {
    xassert(!( m_negative && m_magnitude.isZero() ));
  }

  // ---------- Zero ----------
  // True if this object represents zero.
  bool isZero() const
  {
    return m_magnitude.isZero();
  }

  // Set the value of this object to zero.
  void setZero()
  {
    m_magnitude.clear();
    m_negative = false;
  }

  // ---------- Negative ----------
  bool isNegative() const
  {
    return m_negative;
  }

  // Flip the sign of `*this` unless the magnitude is zero.
  void flipSign()
  {
    if (isNegative()) {
      m_negative = false;
    }
    else {
      if (isZero()) {
        // Do not change the sign; -0 = 0.
      }
      else {
        m_negative = true;
      }
    }
  }

  // ---------- Convert to primitive ----------
  /* Get as a primitive type, or `nullopt` if it will not fit.  That
     includes the cases:

     * `PRIM` is unsigned and `isNegative()` is true.

     * `PRIM` is signed, the value is positive, it could fit into the
       unsigned counterpart of `PRIM`, but not `PRIM` itself since
       attempting to do so would make it negative.
  */
  template <typename PRIM>
  std::optional<PRIM> getAsOpt() const
  {
    static_assert(std::is_integral<PRIM>::value);

    if (isNegative()) {
      if (std::is_unsigned<PRIM>::value) {
        // Cannot store a negative value in an unsigned type.
        return std::nullopt;
      }

      // Extract the magnitude using the unsigned counterpart of `PRIM`.
      typedef typename std::make_unsigned<PRIM>::type UPRIM;
      std::optional<UPRIM> primMagOpt =
        m_magnitude.template getAs<UPRIM>();
      if (!primMagOpt.has_value()) {
        // Not representable even with the unsigned version.
        return std::nullopt;
      }
      UPRIM primMag = primMagOpt.value();

      // Can this fit into the signed `PRIM`?
      UPRIM highBit = ((UPRIM)1) << (sizeof(UPRIM)*8 - 1);
      if (primMag & highBit) {
        if (primMag == highBit) {
          // The value is just barely representable.
          return std::numeric_limits<PRIM>::min;
        }
        else {
          // No, it is too large for the signed version.
          return std::nullopt;
        }
      }

      // Yes, convert it and flip the sign.
      return -(PRIM)primMag;
    }

    else {
      // The value is non-negative, so the underlying unsigned AP
      // integer can handle this.
      return m_magnitude.template getAs<PRIM>();
    }
  }

  // Same as `getAsOpt()`, but throwing an exception if it does not fit.
  template <typename PRIM>
  PRIM getAs() const
  {
    std::optional<PRIM> res = getAsOpt<PRIM>();
    if (!res.has_value()) {
      UInteger::template throwDoesNotFitException<PRIM>(
        "APInteger", this->toString());
    }
    return res.value();
  }

  // ---------- Relational comparison ----------
  // Return <0 if a<b, 0 if a==b, >0 if a>b.
  friend int compare(APInteger const &a,
                     APInteger const &b)
  {
    // Negative comes before positive so flip the usual order here.
    RET_IF_COMPARE(b.m_negative, a.m_negative);

    if (a.m_negative) {
      // Larger negative comes before smaller negative, so again flip
      // the usual order.
      RET_IF_COMPARE(b.m_magnitude, a.m_magnitude);
    }
    else {
      RET_IF_COMPARE(a.m_magnitude, b.m_magnitude);
    }

    return 0;
  }

  DEFINE_FRIEND_RELATIONAL_OPERATORS(APInteger)

  // ---------- Convert to sequence of digits ----------
  /* Return a string of base-`radix` digits representing `*this`.  The
     radix must be in [2,36].  The output begins with '-' if the value
     is negative.

     If `radixIndicator`, then `radix` must be 2, 8, 10, or 16, and
     after the minus sign (if any), the output has "0b", "0o", "" (for
     decimal), or "0x" respectively, inserted before the digits.

     The output always includes at least one digit, but otherwise, does
     not have redundant leading zeroes.
  */
  std::string getAsRadixDigits(int radix, bool radixIndicator) const
  {
    std::string magString;
    if (!radixIndicator) {
      magString = m_magnitude.getAsRadixDigits(radix);
    }
    else {
      magString = m_magnitude.getAsRadixPrefixedDigits(radix);
    }

    if (isNegative()) {
      return std::string("-") + magString;
    }
    else {
      return magString;
    }
  }

  // Return the value as a decimal string.
  std::string toString() const
  {
    return getAsRadixDigits(10, false /*radixIndicator*/);
  }

  friend std::ostream &operator<<(std::ostream &os, APInteger const &n)
  {
    os << n.toString();
    return os;
  }

  // ---------- Convert from sequence of digits ----------
  /* Convert `digits` to an integer value.

     If it starts with '-', return a negative value.

     If `radix` is -1 and, after the optional minus sign, the digits
     begin with "0b", "0o", or "0x", then treat the digits that follow
     as being in base 2, 8, or 16 respectively.  Otherwise, treat them
     as decimal.

     If `radix` is not -1, then it must be in [2,36], and specifies the
     base in which to interpret the digits.

     If `digits` is empty, return zero.  If it consists only of "-",
     that is an error.  If there is a radix indicator but no following
     digits, that is also an error.

     Throw `XFormat` if there is an error.
  */
  static APInteger fromPossiblyRadixPrefixedDigits(
    std::string_view digits, int radix)
  {
    if (digits.empty()) {
      return APInteger();
    }

    bool negative = false;
    if (digits[0] == '-') {
      negative = true;
      digits.remove_prefix(1);

      if (digits.empty()) {
        xformat("Attempt to convert the string \"-\" to an integer.");
      }
    }

    if (radix < 0) {
      // Detect the radix.
      return APInteger(UInteger::fromRadixPrefixedDigits(digits),
                       negative);
    }
    else {
      // Radix is specified.
      return APInteger(UInteger::fromRadixDigits(digits, radix),
                       negative);
    }
  }

  // Calls `fromPossiblyRadixPrefixedDigits` with non-negative `radix`.
  static APInteger fromRadixDigits(std::string_view digits, int radix)
  {
    xassert(2 <= radix && radix <= 36);
    return fromPossiblyRadixPrefixedDigits(digits, radix);
  }

  // Calls `fromPossiblyRadixPrefixedDigits` with a negative `radix`.
  static APInteger fromRadixPrefixedDigits(std::string_view digits)
  {
    return fromPossiblyRadixPrefixedDigits(digits, -1 /*radix*/);
  }

  // There is no `operator>>`.  See explanation in `sm-ap-uint.h`.

  // ---------- Addition ----------
  APInteger &operator+=(APInteger const &other)
  {
    return *this = *this + other;
  }

  APInteger operator+(APInteger const &other) const
  {
    return sumOrDifference(other, true /*isSum*/);
  }

  // Return sum if `isSum`, difference otherwise.
  APInteger sumOrDifference(APInteger const &other, bool isSum) const
  {
    bool sameSign = this->isNegative() == other.isNegative();
    if (sameSign == isSum) {
      // Same effective signs, add magnitudes.
      return APInteger(this->m_magnitude + other.m_magnitude,
                       this->isNegative());
    }
    else if (this->m_magnitude >= other.m_magnitude) {
      // `*this` dominates.
      return APInteger(this->m_magnitude - other.m_magnitude,
                       this->isNegative());
    }
    else {
      // `other` dominates.  If `isSum`, we use its sign, otherwise we
      // flip it.
      return APInteger(other.m_magnitude - this->m_magnitude,
                       other.isNegative() == isSum);
    }
  }

  // ---------- Subtraction ----------
  APInteger &operator-=(APInteger const &other)
  {
    return *this = *this - other;
  }

  APInteger operator-(APInteger const &other) const
  {
    return sumOrDifference(other, false /*isSum*/);
  }

  // ---------- Multiplication ----------
  APInteger &operator*=(APInteger const &other)
  {
    return *this = *this * other;
  }

  APInteger operator*(APInteger const &other) const
  {
    return APInteger(this->m_magnitude * other.m_magnitude,
                     this->isNegative() != other.isNegative());
  }

  // ---------- Division ----------
  // TODO


};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_AP_INT_H
