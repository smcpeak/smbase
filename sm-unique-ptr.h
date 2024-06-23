// sm-unique-ptr.h
// `UniquePtr`, like `std::unique_ptr` but with lighter compile-time
// dependencies.

// This file is in the public domain.

/* `std::unique_ptr` is nice, but it is in the header `<memory>`, which
   has a ton of other stuff.  In the libc++ that comes with GCC-9.3,
   preprocessing a file with just "#include <memory>" produces over
   32k lines of code!

   This class is meant as an interface-compatible workalike for
   `std::unique_ptr` with much lighter compile-time dependencies.

   It is similar to `Owner` in `owner.h`, but that class was designed
   totally independenly of `std::unique_ptr` and hence is not close to
   being interface-compatible.

   There are some differences from `std::unique_ptr`:

   * It omits the `Deleter` template argument.  (This class exclusively
     uses `delete` to deallocate.)

   * It omits support for managing points to arrays.

   * The dereference operators are not marked `noexcept`, and in fact
     will throw `XAssert` if a precondition is violated.

   Otherwise, it is my intention that a given use of `UniquePtr` could
   be changed to `std::unique_ptr` without breaking anything.  That is
   why I have not, for example, added any extra methods, even when doing
   so might improve clarity (e.g., I would like `has_value()` rather
   than just `operator bool`).
*/

#ifndef SMBASE_SM_UNIQUE_PTR_H
#define SMBASE_SM_UNIQUE_PTR_H

#include "sm-unique-ptr-iface.h"       // interface for this module

#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "xassert.h"                   // xassertPrecondition

#include <utility>                     // std::swap


OPEN_NAMESPACE(smbase)


template <typename T>
inline constexpr UniquePtr<T>::UniquePtr() noexcept
  : m_ptr(nullptr)
{}


template <typename T>
inline constexpr UniquePtr<T>::UniquePtr(std::nullptr_t) noexcept
  : m_ptr(nullptr)
{}


template <typename T>
inline constexpr UniquePtr<T>::UniquePtr(T * NULLABLE p) noexcept
  : m_ptr(p)
{}


template <typename T>
inline constexpr UniquePtr<T>::UniquePtr(UniquePtr &&obj) noexcept
  : m_ptr(obj.release())
{}


template <typename T>
template <typename U>
inline constexpr UniquePtr<T>::UniquePtr(UniquePtr<U> &&obj) noexcept
  : m_ptr(obj.release())
{}


template <typename T>
inline UniquePtr<T>::~UniquePtr() noexcept
{
  reset(nullptr);
}


template <typename T>
inline constexpr UniquePtr<T> &UniquePtr<T>::operator=(UniquePtr &&obj) noexcept
{
  reset(obj.release());
  return *this;
}


template <typename T>
template <typename U>
inline constexpr UniquePtr<T> &UniquePtr<T>::operator=(UniquePtr<U> &&obj) noexcept
{
  reset(obj.release());
  return *this;
}


template <typename T>
inline constexpr UniquePtr<T> &UniquePtr<T>::operator=(std::nullptr_t) noexcept
{
  reset();
  return *this;
}


template <typename T>
inline constexpr T &UniquePtr<T>::operator*() const
{
  xassertPrecondition(get() != nullptr);
  return *(get());
}


template <typename T>
inline constexpr T *UniquePtr<T>::operator->() const
{
  xassertPrecondition(get() != nullptr);
  return get();
}


template <typename T>
inline constexpr T *UniquePtr<T>::get() const noexcept
{
  return m_ptr;
}


template <typename T>
inline constexpr UniquePtr<T>::operator bool() const noexcept
{
  return get() != nullptr;
}


template <typename T>
inline constexpr T *UniquePtr<T>::release() noexcept
{
  T *ret = get();
  m_ptr = nullptr;
  return ret;
}


template <typename T>
inline constexpr void UniquePtr<T>::reset(T *p) noexcept
{
  T *old_p = get();
  m_ptr = p;
  if (old_p) {
    delete old_p;
  }
}


template <typename T>
inline constexpr void UniquePtr<T>::swap(UniquePtr &obj) noexcept
{
  using std::swap;
  swap(m_ptr, obj.m_ptr);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_UNIQUE_PTR_H
