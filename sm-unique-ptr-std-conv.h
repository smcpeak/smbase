// sm-unique-ptr-std-conv.h
// Conversions between `smbase::UniquePtr` and `std::unique_ptr`.

// See license.txt for copyright and terms of use.

/*
  This file is separate from `sm-unique-ptr.h` because it incurs the
  relatively heavy <memory> dependency.
*/

#ifndef SMBASE_SM_UNIQUE_PTR_STD_CONV_H
#define SMBASE_SM_UNIQUE_PTR_STD_CONV_H

#include "sm-unique-ptr-iface.h"       // interface for this module

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(smbase)


template <typename T>
inline constexpr UniquePtr<T>::UniquePtr(std::unique_ptr<T> &&obj) noexcept
  : m_ptr(obj.release())
{}


template <typename T>
template <typename U>
inline constexpr UniquePtr<T>::UniquePtr(std::unique_ptr<U> &&obj) noexcept
  : m_ptr(obj.release())
{}


template <typename T>
inline constexpr UniquePtr<T>::operator std::unique_ptr<T>() && noexcept
{
  return std::unique_ptr<T>(this->release());
}


template <typename T>
template <typename U>
inline constexpr UniquePtr<T>::operator std::unique_ptr<U>() && noexcept
{
  return std::unique_ptr<T>(this->release());
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_UNIQUE_PTR_STD_CONV_H
