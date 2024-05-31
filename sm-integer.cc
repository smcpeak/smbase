// sm-integer.cc
// Code for `sm-integer.h`.

// This file is in the public domain.

#include "sm-integer.h"                // this module

#include "sm-ap-int.h"                 // APInteger
#include "sm-macros.h"                 // OPEN_NAMESPACE, STATICDEF

#include <cstdint>                     // std::uint32_t
#include <new>                         // placement `new`
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// The underlying implementation type.
typedef APInteger<std::uint32_t, std::int32_t> UnderInteger;

// The underlying integer as a `const` pseudo-member of `Integer` `obj`.
#define M_UNDER_OF_CONST(obj) \
  (*reinterpret_cast<UnderInteger const *>(&((obj).m_storage)))

// The underlying integer as a non-`const` pseudo-member of `Integer`
// `obj`.
#define M_UNDER_OF(obj) \
  (*reinterpret_cast<UnderInteger       *>(&((obj).m_storage)))

// The underlying integer of `*this`.
#define M_UNDER_CONST M_UNDER_OF_CONST(*this)
#define M_UNDER       M_UNDER_OF      (*this)


Integer::Integer(void *underInteger, UnderIntegerMoveCtorTag)
  : m_storage()
{
  new (&M_UNDER) UnderInteger(
    std::move(*reinterpret_cast<UnderInteger*>(underInteger))
  );
}


// This class is needed to get around the access control applied to the
// constructor and its enumeration tag.
struct IntegerHelper {
  static Integer underToInteger(UnderInteger &&under)
  {
    return Integer(&under, Integer::UnderIntegerMoveCtor);
  }
};

// Given a moveable reference to an `UnderInteger`, move it into a new
// `Integer` object.
static Integer underToInteger(UnderInteger &&under)
{
  return IntegerHelper::underToInteger(std::move(under));
}


Integer::~Integer()
{
  M_UNDER.~UnderInteger();
}


Integer::Integer()
  : m_storage()
{
  // Make sure an `UnderInteger` object fits into `m_storage`.
  static_assert(sizeof(UnderInteger) <= sizeof(m_storage));

  new (&M_UNDER) UnderInteger();
}


Integer::Integer(Integer const &obj)
  : m_storage()
{
  new (&M_UNDER) UnderInteger(M_UNDER_OF_CONST(obj));
}


Integer::Integer(Integer &&obj)
  : m_storage()
{
  new (&M_UNDER) UnderInteger(std::move(M_UNDER_OF(obj)));
}


template <typename PRIM>
Integer::Integer(PRIM n)
  : m_storage()
{
  new (&M_UNDER) UnderInteger(n);
}


Integer &Integer::operator=(Integer const &obj)
{
  M_UNDER = M_UNDER_OF_CONST(obj);
  return *this;
}


Integer &Integer::operator=(Integer &&obj)
{
  M_UNDER = std::move(M_UNDER_OF(obj));
  return *this;
}


void Integer::selfCheck() const
{
  M_UNDER_CONST.selfCheck();
}


bool Integer::isZero() const
{
  return M_UNDER_CONST.isZero();
}


void Integer::setZero()
{
  M_UNDER.setZero();
}


bool Integer::isNegative() const
{
  return M_UNDER_CONST.isNegative();
}


void Integer::flipSign()
{
  M_UNDER.flipSign();
}


template <typename PRIM>
std::optional<PRIM> Integer::getAsOpt() const
{
  return M_UNDER_CONST.template getAsOpt<PRIM>();
}


template <typename PRIM>
PRIM Integer::getAs() const
{
  // I choose to re-implement this rather than calling the method of
  // `M_UNDER` because I want to use the correct name for *this* class
  // rather than letting the "APInteger" name leak through.

  std::optional<PRIM> res = getAsOpt<PRIM>();
  if (!res.has_value()) {
    UnderInteger::template throwDoesNotFitException<PRIM>(
      "Integer", this->toString());
  }
  return res.value();
}


int compare(Integer const &a,
            Integer const &b)
{
  return compare(M_UNDER_OF_CONST(a), M_UNDER_OF_CONST(b));
}


std::string Integer::getAsRadixDigits(int radix, bool radixIndicator) const
{
  return M_UNDER_CONST.getAsRadixDigits(radix, radixIndicator);
}


std::string Integer::toString() const
{
  return M_UNDER_CONST.toString();
}


std::ostream &operator<<(std::ostream &os, Integer const &n)
{
  return os << M_UNDER_OF_CONST(n);
}


std::string Integer::toHexString() const
{
  return M_UNDER_CONST.toHexString();
}


STATICDEF Integer Integer::fromPossiblyRadixPrefixedDigits(
  std::string_view digits, int radix)
{
  return underToInteger(
    UnderInteger::fromPossiblyRadixPrefixedDigits(digits, radix)
  );
}


STATICDEF Integer Integer::fromRadixDigits(
  std::string_view digits, int radix)
{
  return underToInteger(
    UnderInteger::fromRadixDigits(digits, radix)
  );
}


STATICDEF Integer Integer::fromDigits(std::string_view digits)
{
  return underToInteger(
    UnderInteger::fromDigits(digits)
  );
}


Integer &Integer::operator+=(Integer const &other)
{
  M_UNDER += M_UNDER_OF_CONST(other);
  return *this;
}


Integer Integer::operator+(Integer const &other) const
{
  return underToInteger(M_UNDER_CONST + M_UNDER_OF_CONST(other));
}


Integer const &Integer::operator+() const
{
  // I simply assume that `operator+` does not change anything.
  return *this;
}


Integer &Integer::operator-=(Integer const &other)
{
  M_UNDER -= M_UNDER_OF_CONST(other);
  return *this;
}


Integer Integer::operator-(Integer const &other) const
{
  return underToInteger(M_UNDER_CONST - M_UNDER_OF_CONST(other));
}


Integer Integer::operator-() const
{
  return underToInteger(-M_UNDER_CONST);
}

Integer &Integer::operator*=(Integer const &other)
{
  M_UNDER *= M_UNDER_OF_CONST(other);
  return *this;
}


Integer Integer::operator*(Integer const &other) const
{
  return underToInteger(M_UNDER_CONST * M_UNDER_OF_CONST(other));
}


STATICDEF void Integer::divide(
  Integer &quotient,
  Integer &remainder,
  Integer const &dividend,
  Integer const &divisor)
{
  UnderInteger uq, ur;
  UnderInteger::divide(
    uq,
    ur,
    M_UNDER_OF_CONST(dividend),
    M_UNDER_OF_CONST(divisor));
  quotient = underToInteger(std::move(uq));
  remainder = underToInteger(std::move(ur));
}


Integer Integer::operator/(Integer const &divisor) const
{
  return underToInteger(M_UNDER_CONST / M_UNDER_OF_CONST(divisor));
}


Integer Integer::operator%(Integer const &divisor) const
{
  return underToInteger(M_UNDER_CONST % M_UNDER_OF_CONST(divisor));
}


Integer &Integer::operator/=(Integer const &divisor)
{
  M_UNDER /= M_UNDER_OF_CONST(divisor);
  return *this;
}


Integer &Integer::operator%=(Integer const &divisor)
{
  M_UNDER %= M_UNDER_OF_CONST(divisor);
  return *this;
}


// Explicit instantiation definition for the Integer method templates.
#define DEFINE_INTEGER_METHOD_SPECIALIZATIONS(PRIM) \
  template                                          \
  Integer::Integer(PRIM n);                         \
                                                    \
  template                                          \
  std::optional<PRIM> Integer::getAsOpt() const;    \
                                                    \
  template                                          \
  PRIM Integer::getAs() const;


// My intent is to list all of the types that are used to make the
// various types like `int64_t`.  Perhaps I should list those directly?
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(char)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(signed char)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(unsigned char)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(short)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(unsigned short)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(int)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(unsigned)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(long)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(unsigned long)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(long long)
DEFINE_INTEGER_METHOD_SPECIALIZATIONS(unsigned long long)


#undef DEFINE_INTEGER_METHOD_SPECIALIZATIONS


CLOSE_NAMESPACE(smbase)


// EOF
