// gdvtuple.h
// `GDVTuple`, a tuple class for use by `gdvalue.h`.

// This file is in the public domain.

#ifndef SMBASE_GDVTUPLE_H
#define SMBASE_GDVTUPLE_H

#include "gdvtuple-fwd.h"              // fwds for this module

// this dir
#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "gdvalue-fwd.h"               // GDValue
#include "sm-macros.h"                 // OPEN_NAMESPACE

// libc++
#include <cstddef>                     // std::{size_t, ptrdiff_t}
#include <initializer_list>            // std::initializer_list
#include <memory>                      // std::allocator
#include <vector>                      // std::vector


OPEN_NAMESPACE(gdv)


// A finite sequence of `GDValue` where it is presumed that the meaning
// of each element depends on its position (unlike with `GDVSequence`).
//
// This class is basically an exercise in trying to implement the
// `std::vector` API as described at
// https://en.cppreference.com/w/cpp/container/vector .
class GDVTuple {
public:      // types
  // The vector type I'm emulating.
  typedef std::vector<GDValue> Vector;

  // Member types that `std::vector` has.
  typedef GDValue                        value_type;

  typedef std::allocator<GDValue>        allocator;

  typedef std::size_t                    size_type;
  typedef std::ptrdiff_t                 difference_type;

  typedef GDValue &                      reference;
  typedef GDValue const &                const_reference;

  typedef GDValue *                      pointer;
  typedef GDValue const *                const_pointer;

  typedef Vector::iterator               iterator;
  typedef Vector::const_iterator         const_iterator;

  typedef Vector::reverse_iterator       reverse_iterator;
  typedef Vector::const_reverse_iterator const_reverse_iterator;

public:      // data
  // The actual storage.  This is public for now in case there is
  // something in the API I have trouble with, or just want to make it
  // easy to move data between `GDVTuple` and `GDVSequence`.
  Vector m_vector;

public:      // methods
  ~GDVTuple();

  // ---- Constructors ----
  // Empty tuple.
  GDVTuple() noexcept;

  // Tuple of `count` copies of `value`.
  GDVTuple(size_type count, GDValue const &value);

  // Tuple of `count` copies of a default-constructed GDValue.
  GDVTuple(size_type count);

  // The problem with the iterator constructor template is that would
  // require a definition in this file, which I do not want.

  GDVTuple(GDVTuple const &obj);
  GDVTuple(GDVTuple      &&obj);

  GDVTuple(std::initializer_list<GDValue> init);

  // ---- Assignment ----
  GDVTuple &operator=(GDVTuple const &obj);
  GDVTuple &operator=(GDVTuple      &&obj) noexcept;

  GDVTuple &operator=(std::initializer_list<GDValue> init);

  // TODO: assign, get_allocator

  // ---- Element access ----
  GDValue const &at(size_type pos) const;
  GDValue       &at(size_type pos)      ;

  GDValue const &operator[](size_type pos) const;
  GDValue       &operator[](size_type pos)      ;

  // TODO: front, back, data

  // ---- Iterators -----
  const_iterator cbegin() const noexcept;
  const_iterator  begin() const         ;
        iterator  begin()               ;

  const_iterator cend() const noexcept;
  const_iterator  end() const noexcept;
        iterator  end()       noexcept;

  // TODO: rbegin/rend

  // ---- Capacity ----
  bool empty() const noexcept;

  size_type size() const noexcept;

  // TODO: max_size, reserve, capacity, shrink_to_fit

  // ---- Modifiers ----
  void clear() noexcept;

  iterator insert(const_iterator pos, GDValue const &value);

  // TODO: Other `insert` overloads.

  // TODO: emplace

  iterator erase(const_iterator pos);

  // TODO: Other `erase` overload.

  void push_back(GDValue const &value);
  void push_back(GDValue      &&value);

  // TODO: emplace_back, append_range, pop_back

  void resize(size_type count);

  // TODO: The other `resize` overload.

  void swap(GDVTuple &obj) noexcept;

  // ---- Comparison ----
  friend int compare(GDVTuple const &a, GDVTuple const &b);
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDVTuple)
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVTUPLE_H
