// sm-span-iface.h
// `Span` class, a view onto a contiguous in memory sequence of
// elements, similar to C++20 `std::span`.

// See license.txt for copyright and terms of use.

// This is the interface definition.  The implementation is in
// `sm-span-ops.h`.

#ifndef SMBASE_SM_SPAN_IFACE_H
#define SMBASE_SM_SPAN_IFACE_H

#include "sm-span-fwd.h"               // fwds for this module

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-type-traits.h"     // smbase::IsConstAndNonConst_v
#include "smbase/std-vector-fwd.h"     // std::vector

#include <cstddef>                     // std::size_t


OPEN_NAMESPACE(smbase)


// View onto a contiguous array of elements.
//
// This is implemented as a pointer and bounds.  The underlying data is
// used in-place, and its storage must outlive the view.
//
// Constness of the view itself only restrains modifying the view
// endpoints, not the data elements it points at.
//
// Note that `T` could be a const-qualified type to achieve a read-only
// view.
//
// This is basicaly C++20 `std::span` with dynamic extent, but I'm not
// using C++20 yet, and implementing it is an interesting exercise.  The
// interface is meant to be similar.
//
template <typename T>
class Span {
public:      // types
  typedef std::size_t size_type;

  class iterator {
    friend class Span<T>;

  private:     // data
    // Pointer to current view element.
    //
    // Invariant: With respect to the originating view `v`:
    //
    //   v.m_data <= m_elementPointer <= v.m_data+m_size
    //
    // However this invariant is not enforced.
    T *m_elementPointer;

  private:     // methods
    explicit inline iterator(T *elementPointer);

  public:      // methods
    T &operator*() const { return *m_elementPointer; }

    // No `operator->` because applying `->` to iterators is generally
    // troublesome.

    inline iterator &operator++();
    inline iterator operator++(int);

    DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(iterator)
  };

  // There is no `const_iterator` because constness of the view is not
  // transitive to the elements.

private:     // data
  // First element in the view.
  T *m_data;

  // Number of elements in the view.
  size_type m_size;

public:      // methods
  // Pointer and size.
  inline Span(T *data, size_type size);

  // Array with known size.
  template <std::size_t size>
  inline Span(T (&arr)[size])
    : m_data(arr), m_size(size) {}

  // Whole vector, read-only.  `T` must be const-qualified, and upon
  // removing that qualifier, be the same as `T2`.  (Mere convertibility
  // is insufficient since that would allow `T2` to be a subclass of
  // `T`, which would ruin the pointer arithmetic.)
  template <typename T2, typename A,
            std::enable_if_t<IsConstAndNonConst_v<T, T2>, int> = 0>
  inline Span(std::vector<T2,A> const &vec);

  // Whole vector, with read/write access to elements.
  template <typename A>
  inline Span(std::vector<T,A> &vec);

  // Convert from non-const `T2` to const `T`.
  template <typename T2,
            std::enable_if_t<IsConstAndNonConst_v<T, T2>, int> = 0>
  inline Span(Span<T2> const &obj);


  inline bool empty() const { return m_size == 0; }

  inline size_type size() const { return m_size; }
  inline T *data() const { return m_data; }

  inline T &at(size_type index) const;
  inline T &operator[](size_type index) const { return at(index); }

  inline iterator begin() const;
  inline iterator end() const;

  // Return the subsequence starting at `start` and containing `size`
  // elements.
  inline Span subspan(size_type start, size_type size) const;

  // Return the subsequence starting at `start` and containing all
  // elements from there to the end.
  inline Span subspan(size_type start) const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_IFACE_H
