// distinct-number.h
// Wrapper template for numbers that do not implicitly convert.

// Operations with extra dependencies are defined in
// `distinct-number-ops.h`.

#ifndef DISTINCT_NUMBER_H
#define DISTINCT_NUMBER_H

#include "gdvalue-fwd.h"               // gdv::GDValue [n]
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB

#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(smbase)


// Variation of `NUM` that does not allow implicit conversion *into*
// this type, but does allow implicit conversion out.
//
// `TAG` is expected to be a class type defined for the purpose of
// making the resulting specialization distinct from any other
// DistinctNumber.  It is never actually used within the template.
//
// `NUM` is expected to be something that acts like a built-in numeric
// type.
//
template <typename TAG, typename NUM>
class DistinctNumber {
private:     // data
  // Underlying number.
  NUM m_num;

public:
  ~DistinctNumber() = default;

  DistinctNumber()
    : m_num()
  {}

  DistinctNumber(DistinctNumber const &obj)
    : DMEMB(m_num)
  {}

  DistinctNumber &operator=(DistinctNumber const &obj)
  {
    CMEMB(m_num);
    return *this;
  }

  // The whole point of this class is that this is `explicit`.
  explicit DistinctNumber(NUM num)
    : m_num(num)
  {}

  NUM get() const
  {
    return m_num;
  }

  void set(NUM const &num)
  {
    m_num = num;
  }

  bool isZero() const
  {
    return m_num == NUM();
  }

  bool isNotZero() const
  {
    return m_num != NUM();
  }

  // Allow implicit conversion to `NUM` because, otherwise, I think this
  // might be too much hassle to use.
  operator NUM() const
  {
    return m_num;
  }

  #define DEFINE_ARITHMETIC_OPERATOR(op)                        \
    friend DistinctNumber operator op (DistinctNumber const &a, \
                                       DistinctNumber const &b) \
    {                                                           \
      return DistinctNumber(a.get() op b.get());                \
    }                                                           \
                                                                \
    DistinctNumber &operator op##= (DistinctNumber const &obj)  \
    {                                                           \
      m_num op##= obj.get();                                    \
      return *this;                                             \
    }

  DEFINE_ARITHMETIC_OPERATOR(+)
  DEFINE_ARITHMETIC_OPERATOR(-)
  DEFINE_ARITHMETIC_OPERATOR(*)
  DEFINE_ARITHMETIC_OPERATOR(/)
  DEFINE_ARITHMETIC_OPERATOR(%)

  #undef DEFINE_ARITHMETIC_OPERATOR

  #define DEFINE_COMPARISON_OPERATOR(op)              \
    friend bool operator op (DistinctNumber const &a, \
                             DistinctNumber const &b) \
    {                                                 \
      return a.get() op b.get();                      \
    }

  DEFINE_COMPARISON_OPERATOR(==)
  DEFINE_COMPARISON_OPERATOR(!=)
  DEFINE_COMPARISON_OPERATOR(<)
  DEFINE_COMPARISON_OPERATOR(<=)
  DEFINE_COMPARISON_OPERATOR(>)
  DEFINE_COMPARISON_OPERATOR(>=)

  #undef DEFINE_COMPARISON_OPERATOR

  DistinctNumber &operator++()
  {
    ++m_num;
    return *this;
  }

  DistinctNumber &operator--()
  {
    --m_num;
    return *this;
  }

  DistinctNumber operator++(int)
  {
    DistinctNumber ret(*this);
    ++m_num;
    return ret;
  }

  DistinctNumber operator--(int)
  {
    DistinctNumber ret(*this);
    --m_num;
    return ret;
  }

  void write(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os,
                                  DistinctNumber const &obj)
  {
    obj.write(os);
    return os;
  }

  operator gdv::GDValue() const;
};


CLOSE_NAMESPACE(smbase)


#endif // DISTINCT_NUMBER_H
