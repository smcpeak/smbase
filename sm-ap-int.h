// sm-ap-int.h
// `APInteger`, an arbitrary-precision integer class.

// This file is in the public domain.

#ifndef SMBASE_SM_AP_INT_H
#define SMBASE_SM_AP_INT_H

#include "compare-util.h"              // RET_IF_COMPARE
#include "overflow.h"                  // convertNumberOpt
#include "sm-ap-uint.h"                // APUInteger
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB
#include "xarithmetic.h"               // XDivideByZero

#include <cstdint>                     // std::int32_t
#include <limits>                      // std::numeric_limits
#include <optional>                    // std::{optional, nullopt}
#include <string_view>                 // std::string_view
#include <type_traits>                 // std::{is_unsigned, make_unsigned}


OPEN_NAMESPACE(smbase)


/* Arbitrary-precision integer, positive or negative.

   When the value is large, it is represented as a sequence of `Word`
   elements.  `Word` must be unsigned.

   When it is small, it is represented as a single `EmbeddedInt`, which
   must be signed.

   Both of these are template parameters primarily to facilitiate
   testing.  The `Integer` class (declared in `sm-integer.h`) wraps this
   with a more convenient, non-templated interface.

   My nominal assumption is that, when used in production, `Word` will
   be `uint32_t` and `EmbeddedInt` will be `int32_t`.
*/
template <typename Word, typename EmbeddedInt>
class APInteger {
private:     // types
  // Indicator of signedness or of an embedded value.
  enum SignOrEmbed : std::uint8_t {
    // The value is positive `m_magnitude` (which includes the case
    // where it is zero).  `m_eValue==0`.
    SOE_POSITIVE,

    // The value is negative `m_magnitude`.  In this case, `m_magnitude`
    // is not zero and `m_eValue==0`.
    SOE_NEGATIVE,

    // The value is `m_eValue`.  `m_magnitude` is zero.
    SOE_EMBEDDED
  };

  // The unsigned type upon which this is based.
  typedef APUInteger<Word> UInteger;

private:     // data
  // Magnitude of the value if it is larger than what can be represented
  // in `m_eValue`.
  UInteger m_magnitude;

  // Indicator of where the value is, and its signedness if it is in
  // `m_magnitude`.
  SignOrEmbed m_soe;

  /* If `m_soe==SOE_EMBEDDED`, this contains the value.  Storing small
     values in the object saves a dynamic memory allocation in the
     fairly common case that it fits.

     I choose to use a separate integer rather than, say, using
     `uintptr_t` and combining it with `m_soe`, for simplicity.  I
     generally regard 64-bit as the primary target, and by letting
     `EmbeddedInt` be `int32_t`, I can represent a reasonably large
     range of integers using the embedded value while still not using
     another word of memory.

     I also choose to *not* use the most-negative value that is
     representable, for example -128 if `EmbeddedInt` is `int8_t`.  The
     reason is if I avoid that value, then the range is symmetric (-127
     to 127 in that case), and I can use `UInteger::getAsOpt` to check
     representability.  It also means I can always flip the sign without
     changing how the value is represented.
  */
  EmbeddedInt m_eValue;

private:     // methods
  // Are we using the value embedded as `m_eValue`?
  bool isEmbedded() const
  {
    return m_soe == SOE_EMBEDDED;
  }

  // Normalize the representation.
  void normalize()
  {
    if (isEmbedded()) {
      m_magnitude.setZero();
    }
    else {
      // Check if the value can be represented in `m_eValue`.
      std::optional<EmbeddedInt> valOpt =
        m_magnitude.template getAsOpt<EmbeddedInt>();
      if (valOpt.has_value()) {
        // Use the embedded representation.
        m_eValue = valOpt.value() * (m_soe==SOE_POSITIVE? +1 : -1);
        m_soe = SOE_EMBEDDED;
        m_magnitude.setZero();
      }
      else {
        // Ensure this is zero when not in use.
        m_eValue = 0;
      }
    }
  }

  // Get the magnitude as an AP integer.
  UInteger getAPMagnitude() const
  {
    if (isEmbedded()) {
      // Note that this is safe, in part, because we do not use the
      // most-negative value representable by `EmbeddedInt`.
      return UInteger(std::abs(m_eValue));
    }
    else {
      return m_magnitude;
    }
  }

  // Return sum if `isSum`, difference otherwise.
  APInteger sumOrDifference(APInteger const &other, bool isSum) const
  {
    if (this->isEmbedded() || other.isEmbedded()) {
      // Convert both to AP integers (even if one already is).
      return innerSumOrDifference(
        this->getAPMagnitude(), this->isNegative(),
        other.getAPMagnitude(), other.isNegative(),
        isSum);
    }
    else {
      return innerSumOrDifference(
        this->m_magnitude, this->isNegative(),
        other.m_magnitude, other.isNegative(),
        isSum);
    }
  }

  // Return the sum or difference of two numbers represented as AP
  // magnitudes and signs.
  static APInteger innerSumOrDifference(
    UInteger const &thisMagnitude,
    bool thisIsNegative,
    UInteger const &otherMagnitude,
    bool otherIsNegative,
    bool isSum)
  {
    bool sameSign = thisIsNegative == otherIsNegative;
    if (sameSign == isSum) {
      // Same effective signs, add magnitudes.
      return APInteger(thisMagnitude + otherMagnitude,
                       thisIsNegative);
    }
    else if (thisMagnitude >= otherMagnitude) {
      // `this` dominates.
      return APInteger(thisMagnitude - otherMagnitude,
                       thisIsNegative);
    }
    else {
      // `other` dominates.  If `isSum`, we use its sign, otherwise we
      // flip it.
      return APInteger(otherMagnitude - thisMagnitude,
                       otherIsNegative == isSum);
    }
  }

  // Return the most-negative value that `EmbeddedInt` can represent.
  // We do *not* let `m_eValue` use this value.
  static constexpr EmbeddedInt mostNegativeEmbeddedInt()
  {
    return std::numeric_limits<EmbeddedInt>::min();
  }

public:      // methods
  ~APInteger()
  {
    static_assert(std::is_unsigned<Word>::value);
    static_assert(std::is_signed<EmbeddedInt>::value);
  }

  // ---------- Constructors ----------
  // Zero.
  APInteger()
    : m_magnitude(),
      m_soe(SOE_EMBEDDED),
      m_eValue(0)
  {}

  APInteger(APInteger const &obj)
    : DMEMB(m_magnitude),
      DMEMB(m_soe),
      DMEMB(m_eValue)
  {}

  APInteger(APInteger &&obj)
    : MDMEMB(m_magnitude),
      MDMEMB(m_soe),
      MDMEMB(m_eValue)
  {}

  APInteger(UInteger const &magnitude, bool negative)
    : m_magnitude(magnitude),
      m_soe(negative? SOE_NEGATIVE : SOE_POSITIVE),
      m_eValue(0)
  {
    normalize();
  }

  APInteger(UInteger &&magnitude, bool negative)
    : m_magnitude(std::move(magnitude)),
      m_soe(negative? SOE_NEGATIVE : SOE_POSITIVE),
      m_eValue(0)
  {
    normalize();
  }

  // Construct from `PRIM`, presumed to be a primitive type.  As this
  // preserves information, it is allowed to be invoked implicitly.
  template <typename PRIM>
  APInteger(PRIM n)
    : m_magnitude(),
      m_soe(SOE_EMBEDDED),
      m_eValue(static_cast<EmbeddedInt>(n))
  {
    // Check if we are able represent it with the embedded value.
    if (m_eValue < 0 && std::is_unsigned<PRIM>::value) {
      // No, the conversion flipped the sign.
    }
    else if (static_cast<PRIM>(m_eValue) != n) {
      // No, the conversion lost range.
    }
    else if (m_eValue == mostNegativeEmbeddedInt()) {
      // The value fits just barely as the most-negative representable
      // value.  I choose to not use this value.
    }
    else {
      // Yes, it fits.
      selfCheck();
      return;
    }

    // Since it does not fit in `m_eValue`, we will store it in
    // `m_magnitude`.
    m_eValue = 0;

    // True if we need to add one more to the magnitude at the end.
    bool addOneAfterward = false;

    if (n < 0) {
      m_soe = SOE_NEGATIVE;

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
    else {
      m_soe = SOE_POSITIVE;
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
      CMEMB(m_soe);
      CMEMB(m_eValue);
    }
    return *this;
  }

  APInteger &operator=(APInteger &&obj)
  {
    if (this != &obj) {
      MCMEMB(m_magnitude);
      MCMEMB(m_soe);
      MCMEMB(m_eValue);
    }
    return *this;
  }

  // TODO: Add `swap`.

  // ---------- General ----------
  // Assert invariants.
  void selfCheck() const
  {
    if (isEmbedded()) {
      xassertInvariant(m_magnitude.isZero());
      xassertInvariant(m_eValue != mostNegativeEmbeddedInt());
    }
    else {
      // The magnitude must not fit into `m_eValue`.
      //
      // Note that this check rules out the combination of
      // `m_soe==SOE_NEGATIVE` and `m_magnitude.isZero()`.
      xassert(!m_magnitude.template getAsOpt<EmbeddedInt>().has_value());
    }

    m_magnitude.selfCheck();
  }

  // ---------- Zero ----------
  // True if this object represents zero.
  bool isZero() const
  {
    if (isEmbedded()) {
      return m_eValue == 0;
    }
    else {
      return false;
    }
  }

  // Set the value of this object to zero.
  void setZero()
  {
    m_magnitude.setZero();
    m_soe = SOE_EMBEDDED;
    m_eValue = 0;
  }

  // ---------- Negative ----------
  bool isNegative() const
  {
    if (isEmbedded()) {
      return m_eValue < 0;
    }
    else {
      return m_soe == SOE_NEGATIVE;
    }
  }

  // Flip the sign of `*this` unless the magnitude is zero.
  void flipSign()
  {
    if (isEmbedded()) {
      m_eValue = -m_eValue;
    }
    else if (m_soe == SOE_NEGATIVE) {
      m_soe = SOE_POSITIVE;
    }
    else {
      m_soe = SOE_NEGATIVE;
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

    if (isEmbedded()) {
      // Try to convert `m_eValue` to `PRIM`.
      return convertNumberOpt<PRIM>(m_eValue);
    }

    if (isNegative()) {
      if (std::is_unsigned<PRIM>::value) {
        // Cannot store a negative value in an unsigned type.
        return std::nullopt;
      }

      // Extract the magnitude using the unsigned counterpart of `PRIM`.
      typedef typename std::make_unsigned<PRIM>::type UPRIM;
      std::optional<UPRIM> primMagOpt =
        m_magnitude.template getAsOpt<UPRIM>();
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
          return std::numeric_limits<PRIM>::min();
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
      return m_magnitude.template getAsOpt<PRIM>();
    }
  }

  // Same as `getAsOpt()`, but throwing an exception if it does not fit.
  template <typename PRIM>
  PRIM getAs() const
  {
    std::optional<PRIM> res = getAsOpt<PRIM>();
    if (!res.has_value()) {
      throwDoesNotFitException<PRIM>("APInteger", this->toString());
    }
    return res.value();
  }

  // Throw an exception complaining about the inability to convert a
  // value to `PRIM`.
  template <typename PRIM>
  static void throwDoesNotFitException(
    char const *className,
    std::string const &valueAsString)
  {
    UInteger::template throwDoesNotFitException<PRIM>(
      className, valueAsString);
  }

  // ---------- Relational comparison ----------
  // Compare the numbers as if they were positive, and ignoring the
  // specific value in `m_eValue` because we know they are not *both*
  // using the embedded value.
  //
  // This function should be regarded as private to `APInteger`
  // even though it is not a methed.
  friend int compareAsIfPositive(APInteger const &a,
                                 APInteger const &b)
  {
    // For positive numbers, embedded==true are smaller than, so sort as
    // less than, embedded==false.  But false<true, so flip the order.
    RET_IF_COMPARE(b.isEmbedded(), a.isEmbedded());

    // Both numbers are non-embedded.  Compare magnitudes in the usual
    // way.
    RET_IF_COMPARE(a.m_magnitude, b.m_magnitude);

    return 0;
  }

  // Return <0 if a<b, 0 if a==b, >0 if a>b.
  friend int compare(APInteger const &a,
                     APInteger const &b)
  {
    if (a.isEmbedded() && b.isEmbedded()) {
      return compare(a.m_eValue, b.m_eValue);
    }

    // Negative comes before positive so flip the usual order here.
    RET_IF_COMPARE(b.isNegative(), a.isNegative());

    if (a.isNegative()) {
      // Flip the comparison order to account for negativity.
      return compareAsIfPositive(b, a);
    }

    else {
      return compareAsIfPositive(a, b);
    }
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
    if (isEmbedded()) {
      static_assert(sizeof(EmbeddedInt) <= sizeof(int64_t));
      return int64ToRadixDigits(m_eValue, radix, radixIndicator);
    }

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

  // Return the value as a hex string with radix indicator.
  std::string toHexString() const
  {
    return getAsRadixDigits(16, true /*radixIndicator*/);
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

    // In both of the following cases, if the value ends up being
    // representable in `m_eValue`, that will be handled by the call to
    // `normalize` in the constructor.

    if (radix < 0) {
      // Detect the radix.
      return APInteger(UInteger::fromDigits(digits),
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
    xassertPrecondition(2 <= radix && radix <= 36);
    return fromPossiblyRadixPrefixedDigits(digits, radix);
  }

  // Calls `fromPossiblyRadixPrefixedDigits` with a negative `radix`.
  static APInteger fromDigits(std::string_view digits)
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
    if (this->isEmbedded() && other.isEmbedded()) {
      std::optional<EmbeddedInt> res =
        addWithOverflowCheckOpt(this->m_eValue, other.m_eValue);
      if (res.has_value()) {
        // Note that this is not guaranteed to result in an embedded
        // value because it could be `mostNegativeEmbeddedInt()`.
        return APInteger(res.value());
      }
    }

    return sumOrDifference(other, true /*isSum*/);
  }

  APInteger const &operator+() const
  {
    return *this;
  }

  // ---------- Subtraction ----------
  APInteger &operator-=(APInteger const &other)
  {
    return *this = *this - other;
  }

  APInteger operator-(APInteger const &other) const
  {
    if (this->isEmbedded() && other.isEmbedded()) {
      std::optional<EmbeddedInt> res =
        subtractWithOverflowCheckOpt(this->m_eValue, other.m_eValue);
      if (res.has_value()) {
        return APInteger(res.value());
      }
    }

    return sumOrDifference(other, false /*isSum*/);
  }

  APInteger operator-() const
  {
    APInteger ret(*this);
    ret.flipSign();
    return ret;
  }

  // ---------- Multiplication ----------
  APInteger &operator*=(APInteger const &other)
  {
    return *this = *this * other;
  }

  APInteger operator*(APInteger const &other) const
  {
    if (this->isEmbedded() && other.isEmbedded()) {
      std::optional<EmbeddedInt> res =
        multiplyWithOverflowCheckOpt(this->m_eValue, other.m_eValue);
      if (res.has_value()) {
        return APInteger(res.value());
      }
    }

    return APInteger(this->getAPMagnitude() * other.getAPMagnitude(),
                     this->isNegative() != other.isNegative());
  }

  // ---------- Division ----------
  /* Compute `quotient`, the maximum number of times that `divisor`
     goes into `dividend`, and `remainder`, what is left over after
     taking that many divisors out.

     The operands must all be distinct objects, except that `dividend`
     and `divisor` could be the same.

     Following the C++ rules in [expr.mul], the quotient is "the
     algebraic quotient with any fractional part discarded", i.e. it is
     rounded toward *zero*, not negative infinity.  This in turn has the
     consequence that if the remainder if not zero, its sign is the same
     as that of the dividend (numerator).

     Examples:

       dividend      divisor     quotient    remainder
       --------      -------     --------    ---------
              5            3            1            2
             -5            3           -1           -2
              5           -3           -1            2
             -5           -3            1           -2

     Precondition:

       divisor != 0
       distinct(&quotient, &remainder, {&dividend, &divisor})

     Postcondition:

       (dividend < 0) ==> (remainder <= 0)
       0 <= abs(remainder) < abs(divisor)
       divisor * quotient + remainder = dividend
  */
  static void divide(
    APInteger &quotient,
    APInteger &remainder,
    APInteger const &dividend,         // aka numerator
    APInteger const &divisor)          // aka denominator
  {
    if (divisor.isZero()) {
      // Use the hex form in order to avoid the expensive and
      // complicated process of decimalization.
      THROW(XDivideByZero(dividend.toHexString()));
    }

    if (dividend.isEmbedded() && divisor.isEmbedded()) {
      EmbeddedInt q, r;
      if (divideWithOverflowCheckOpt(
            q,
            r,
            dividend.m_eValue,
            divisor.m_eValue)) {
        quotient = q;
        remainder = r;
        return;
      }
      else {
        // Since we know the divisor is not zero, and we do not use the
        // most-negative value for `EmbeddedInt`, it should not be
        // possible for division to overflow.
        xfailure("not possible");
      }
    }

    // Set up to do the division on the magnitudes.
    UInteger magQuotient;
    UInteger magRemainder;

    // Compute result magnitudes without regard to sign.
    UInteger::divide(
      magQuotient,
      magRemainder,
      dividend.getAPMagnitude(),
      divisor.getAPMagnitude());

    // Compute the signs of the results.
    bool negQuotient =
      dividend.isNegative() != divisor.isNegative();
    bool negRemainder =
      dividend.isNegative();

    // Package the results as APIntegers.
    quotient = APInteger(std::move(magQuotient), negQuotient);
    remainder = APInteger(std::move(magRemainder), negRemainder);
  }

  APInteger operator/(APInteger const &divisor) const
  {
    APInteger quotient, remainder;
    divide(quotient, remainder, *this, divisor);
    return quotient;
  }

  APInteger operator%(APInteger const &divisor) const
  {
    APInteger quotient, remainder;
    divide(quotient, remainder, *this, divisor);
    return remainder;
  }

  APInteger &operator/=(APInteger const &divisor)
  {
    APInteger q = *this / divisor;
    return *this = std::move(q);
  }

  APInteger &operator%=(APInteger const &divisor)
  {
    APInteger r = *this % divisor;
    return *this = std::move(r);
  }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_AP_INT_H
