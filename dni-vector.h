// dni-vector.h
// Like `vector` but with a specific `DistinctNumber` index type.

#ifndef SMBASE_DNI_VECTOR_H
#define SMBASE_DNI_VECTOR_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [f]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DMEMB, CMEMB, MDMEMB, MCMEMB

#include <initializer_list>            // std::initializer_list
#include <iosfwd>                      // std::ostream [f]
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Vector-like mapping from `INDEX`, which is expected to act like an
// unsigned integer type, to `VALUE`.
//
// The `INDEX` type is also returned by `size` because, the context of a
// vector, the size is often used to compute and bound indices.
//
// For the moment this is only a subset of the `std::vector` interface,
// but the plan is to add most of it.  The parts that are here are based
// on the description of `std::vector` at cppreference.com.
//
template <typename INDEX, typename VALUE>
class DNIVector {
private:     // types
  // Type of the underlying vector.
  using UnderVec = std::vector<VALUE>;

public:      // types
  using value_type = VALUE;
  using size_type = INDEX;

  // The difference type isn't closely tied to `INDEX`.
  using difference_type = typename UnderVec::difference_type;

  using reference = value_type &;
  using const_reference = value_type const &;

  using pointer = value_type *;
  using const_pointer = value_type const *;

  using iterator = typename UnderVec::iterator;
  using const_iterator = typename UnderVec::const_iterator;

  using reverse_iterator = typename UnderVec::reverse_iterator;
  using const_reverse_iterator = typename UnderVec::const_reverse_iterator;

private:     // data
  // Underlying storage.
  std::vector<VALUE> m_vec;

public:      // methods
  ~DNIVector() = default;

  // -------------------------- Constructors ---------------------------
  DNIVector()
    : m_vec()
  {}

  DNIVector(DNIVector const &obj)
    : DMEMB(m_vec)
  {}

  DNIVector(DNIVector &&obj);

  DNIVector(std::initializer_list<VALUE> init)
    : m_vec(init)
  {}

  // --------------------------- Assignment ----------------------------
  DNIVector &operator=(DNIVector const &obj)
  {
    CMEMB(m_vec);
    return *this;
  }

  DNIVector &operator=(DNIVector &&obj);

  // ------------------------- Element access --------------------------
  // The main point of this class is that `at` and `operator[]` require
  // an argument of type `INDEX`, not just any integral type.
  reference at(INDEX pos)
  {
    return m_vec.at(pos);
  }

  const_reference at(INDEX pos) const
  {
    return m_vec.at(pos);
  }

  reference operator[](INDEX pos)
  {
    return m_vec[pos];
  }

  const_reference operator[](INDEX pos) const
  {
    return m_vec[pos];
  }

  VALUE *data()
  {
    return m_vec.data();
  }

  VALUE const *data() const
  {
    return m_vec.data();
  }

  // ---------------------------- Iterators ----------------------------
  iterator begin()
  {
    return m_vec.begin();
  }

  const_iterator begin() const
  {
    return m_vec.begin();
  }

  const_iterator cbegin() const noexcept
  {
    return m_vec.cbegin();
  }

  iterator end() noexcept
  {
    return m_vec.end();
  }

  const_iterator end() const noexcept
  {
    return m_vec.end();
  }

  const_iterator cend() const noexcept
  {
    return m_vec.cend();
  }

  // ---------------------------- Capacity -----------------------------
  bool empty() const noexcept
  {
    return m_vec.empty();
  }

  // This is not marked `noexcept` because I allow for the possibility
  // that the `INDEX` ctor might throw.
  INDEX size() const
  {
    return INDEX(m_vec.size());
  }

  // ---------------------------- Modifiers ----------------------------
  void clear() noexcept
  {
    m_vec.clear();
  }

  void push_back(VALUE const &value)
  {
    m_vec.push_back(value);
  }

  void push_back(VALUE &&value);

  void swap(DNIVector &other)
  {
    m_vec.swap(other.m_vec);
  }

  // ---------------------- Relational operators -----------------------
  #define DEFINE_RELATIONAL_OPERATOR(op)                             \
    friend bool operator op (DNIVector const &a, DNIVector const &b) \
    {                                                                \
      return a.m_vec op b.m_vec;                                     \
    }

  DEFINE_RELATIONAL_OPERATOR(==)
  DEFINE_RELATIONAL_OPERATOR(!=)
  DEFINE_RELATIONAL_OPERATOR(<)
  DEFINE_RELATIONAL_OPERATOR(<=)
  DEFINE_RELATIONAL_OPERATOR(>)
  DEFINE_RELATIONAL_OPERATOR(>=)

  #undef DEFINE_RELATIONAL_OPERATOR

  // --------------------------- Extensions ----------------------------
  // Returns a sequence of `toGDValue` applied to the elements.
  operator gdv::GDValue() const;

  // Writes by serializing the GDValue.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, DNIVector const &obj)
  {
    obj.write(os);
    return os;
  }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_DNI_VECTOR_H
