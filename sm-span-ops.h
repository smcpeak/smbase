// sm-span-ops.h
// Operations for `sm-span` module.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SM_SPAN_OPS_H
#define SMBASE_SM_SPAN_OPS_H

#include "sm-span-iface.h"             // decls for this module

#include "smbase/compare-util.h"       // RET_IF_COMPARE_MEMBERS
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP
#include "smbase/xassert.h"            // xassertPrecondition


OPEN_NAMESPACE(smbase)


// ----------------------------- iterator ------------------------------
template <typename T>
Span<T>::iterator::iterator(T *elementPointer)
  : IMEMBFP(elementPointer)
{}


template <typename T>
auto Span<T>::iterator::operator++() -> iterator &
{
  ++m_elementPointer;
  return *this;
}


template <typename T>
auto Span<T>::iterator::operator++(int) -> iterator
{
  iterator ret(*this);
  ++m_elementPointer;
  return ret;
}


template <typename T>
int Span<T>::iterator::compareTo(iterator const &b) const
{
  auto const &a = *this;
  RET_IF_COMPARE_MEMBERS(m_elementPointer);
  return 0;
}


// ----------------------------- Span -----------------------------
template <typename T>
Span<T>::Span()
  : m_data(nullptr),
    m_size(0)
{}


template <typename T>
Span<T>::Span(T *data, size_type size)
  : IMEMBFP(data),
    IMEMBFP(size)
{}


// GCC does not accept this, even though Clang does, so the function is
// defined in the class body.
#if 0
template <typename T>
template <std::size_t size>
Span<T>::Span(T (&arr)[size])
  : m_data(arr),
    m_size(size)
{}
#endif // 0


template <typename T>
template <typename A>
Span<T>::Span(std::vector<T,A> &vec)
  : m_data(vec.data()),
    m_size(vec.size())
{}


template <typename T>
template <typename T2, typename A,
          std::enable_if_t<IsConstAndNonConst_v<T, T2>, int>>
Span<T>::Span(std::vector<T2,A> const &vec)
  : m_data(vec.data()),
    m_size(vec.size())
{}


template <typename T>
template <typename T2,
          std::enable_if_t<IsConstAndNonConst_v<T, T2>, int>>
Span<T>::Span(Span<T2> const &obj)
  : m_data(obj.data()),
    m_size(obj.size())
{}


template <typename T>
T &Span<T>::at(size_type index) const
{
  xassertPrecondition(index < m_size);
  return m_data[index];
}


template <typename T>
auto Span<T>::begin() const -> iterator
{
  return iterator(m_data);
}


template <typename T>
auto Span<T>::end() const -> iterator
{
  return iterator(m_data + m_size);
}


template <typename T>
Span<T> Span<T>::subspan(size_type start, size_type size) const
{
  xassertPrecondition(start <= m_size);
  xassertPrecondition(start+size <= m_size);
  return Span(m_data + start, size);
}


template <typename T>
Span<T> Span<T>::subspan(size_type start) const
{
  xassertPrecondition(start <= m_size);
  return subspan(start, m_size - start);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_OPS_H
