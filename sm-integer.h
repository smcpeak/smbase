// sm-integer.h
// `Integer`, an arbitrary-precision integer.
//
// This acts as a wrapper for the underlying `APInteger`, isolating
// clients from its implementation details and dependencies.

// This file is in the public domain.

#ifndef SMBASE_SM_INTEGER_H
#define SMBASE_SM_INTEGER_H

#include "smbase/sm-integer-fwd.h"     // fwds for this module

#include "compare-util-iface.h"        // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "std-optional-fwd.h"          // std::optional [n]
#include "std-string-fwd.h"            // std::string [n]
#include "std-string-view-fwd.h"       // std::string_view [n]

#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(smbase)


// Helper class defined in the implementation file.
struct IntegerHelper;


// Arbitrary-precision integer, positive or negative.
class Integer {
private:     // data
  // An APInteger has:
  //   - APUInteger, which has:
  //     - std::vector, which has:
  //       - start pointer
  //       - end pointer
  //       - end-of-storage pointer
  //   - SignOrEmbed: uint8_t
  //   - EmbeddedInt: int32_t
  //
  // With padding, that is four or five `void*` words.  I declare this
  // as an array of `void*` rather than `unsigned char` to (hopefully?)
  // ensure proper alignment.  The implementation has a `static_assert`
  // that checks that this is enough space.
  void *m_storage[sizeof(void*) >= 8? 4 : 5];

private:     // methods
  // Special constructor that accepts a pointer to an underlying integer
  // that can be moved from.
  enum UnderIntegerMoveCtorTag { UnderIntegerMoveCtor };
  Integer(void *underInteger, UnderIntegerMoveCtorTag);
  friend struct IntegerHelper;

public:      // methods
  ~Integer();

  // ---------- Constructors ----------
  // Zero.
  Integer();

  Integer(Integer const &obj);
  Integer(Integer      &&obj);

  // Convert from a primitive type.  The possible types are listed at
  // the end of this file.
  template <typename PRIM>
  Integer(PRIM n);

  // ---------- Assignment ----------
  Integer &operator=(Integer const &obj);
  Integer &operator=(Integer      &&obj);

  // ---------- General ----------
  // Assert invariants.
  void selfCheck() const;

  // ---------- Zero ----------
  // True if this object represents zero.
  bool isZero() const;

  // Set the value of this object to zero.
  void setZero();

  // ---------- Negative ----------
  // True if `*this* is less than zero.
  bool isNegative() const;

  // Flip the sign of `*this` unless the magnitude is zero.
  void flipSign();

  // ---------- Convert to primitive ----------
  /* Get as a primitive type, or `nullopt` if it will not fit.  That
     includes the cases:

     * `PRIM` is unsigned and `isNegative()` is true.

     * `PRIM` is signed, the value is positive, it could fit into the
       unsigned counterpart of `PRIM`, but not `PRIM` itself since
       attempting to do so would make it negative.
  */
  template <typename PRIM>
  std::optional<PRIM> getAsOpt() const;

  // Same as `getAsOpt()`, but throwing an exception if it does not fit.
  template <typename PRIM>
  PRIM getAs() const;

  // ---------- Relational comparison ----------
  // Return <0 if a<b, 0 if a==b, >0 if a>b.
  friend int compare(Integer const &a,
                     Integer const &b);

  DEFINE_FRIEND_RELATIONAL_OPERATORS(Integer)

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
  std::string getAsRadixDigits(int radix, bool radixIndicator) const;

  // Return the value as a decimal string.
  std::string toString() const;

  // Write to `os` in the same form as `toString()`.
  friend std::ostream &operator<<(std::ostream &os, Integer const &n);

  // Return the value as a hex string with radix indicator.
  std::string toHexString() const;

  // ---------- Convert from sequence of digits ----------
  /* Convert `digits` to an integer value.

     If it starts with '-', return a negative value.

     If `radix` is -1 and, after the optional minus sign, the digits
     begin with "0b", "0o", or "0x" (case-insensitive), then treat the
     digits that follow as being in base 2, 8, or 16 respectively.
     Otherwise, treat them as decimal.

     If `radix` is not -1, then it must be in [2,36], and specifies the
     base in which to interpret the digits.

     If `digits` is empty, return zero.  If it consists only of "-",
     that is an error.  If there is a radix indicator but no following
     digits, that is also an error.

     Throw `XFormat` if there is an error.
  */
  static Integer fromPossiblyRadixPrefixedDigits(
    std::string_view digits, int radix);

  // Calls `fromPossiblyRadixPrefixedDigits` with non-negative `radix`.
  static Integer fromRadixDigits(std::string_view digits, int radix);

  // Calls `fromPossiblyRadixPrefixedDigits` with a negative `radix`.
  // That is, read the digits, which might have a radix prefix.  This is
  // the most user-friendly interface.
  static Integer fromDigits(std::string_view digits);

  // ---------- Addition ----------
  Integer &operator+=(Integer const &other);

  Integer operator+(Integer const &other) const;

  Integer const &operator+() const;

  // ---------- Subtraction ----------
  Integer &operator-=(Integer const &other);

  Integer operator-(Integer const &other) const;

  Integer operator-() const;

  // ---------- Multiplication ----------
  Integer &operator*=(Integer const &other);

  Integer operator*(Integer const &other) const;

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
    Integer &quotient,
    Integer &remainder,
    Integer const &dividend,           // aka numerator
    Integer const &divisor);           // aka denominator

  Integer operator/(Integer const &divisor) const;
  Integer operator%(Integer const &divisor) const;

  Integer &operator/=(Integer const &divisor);
  Integer &operator%=(Integer const &divisor);
};


// Explicit instantiation declaration for the Integer method templates.
// This tells the compiler *not* to instantiate them implicitly.  There
// are explicit instantiations in the implementation file.
#define DECLARE_INTEGER_METHOD_SPECIALIZATIONS(PRIM) \
  extern template                                    \
  Integer::Integer(PRIM n);                          \
                                                     \
  extern template                                    \
  std::optional<PRIM> Integer::getAsOpt() const;     \
                                                     \
  extern template                                    \
  PRIM Integer::getAs() const;


// My intent is to list all of the types that are used to make the
// various types like `int64_t`.  Perhaps I should list those directly?
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(char)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(signed char)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(unsigned char)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(short)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(unsigned short)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(int)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(unsigned)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(long)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(unsigned long)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(long long)
DECLARE_INTEGER_METHOD_SPECIALIZATIONS(unsigned long long)


#undef DECLARE_INTEGER_METHOD_SPECIALIZATIONS


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_INTEGER_H
