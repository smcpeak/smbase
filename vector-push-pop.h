// vector-push-pop.h
// `VECTOR_PUSH_POP` macro to push a value and pop it on scope exit.

// This file is in the public domain.

#ifndef SMBASE_VECTOR_PUSH_POP_H
#define SMBASE_VECTOR_PUSH_POP_H

#include "vector-push-pop-iface.h"     // interface for this modue

#include "dev-warning.h"               // DEV_WARNING
#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <optional>                    // std::optional
#include <vector>                      // std::vector
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// --------------------------- VectorPushPop ---------------------------
template <typename T>
VectorPushPop<T>::VectorPushPop(std::vector<T> &vec, T const &value)
  : m_vec(vec)
{
  m_vec.push_back(value);
}


template <typename T>
VectorPushPop<T>::VectorPushPop(std::vector<T> &vec, T &&value)
  : m_vec(vec)
{
  m_vec.push_back(std::move(value));
}


template <typename T>
VectorPushPop<T>::~VectorPushPop()
{
  if (m_vec.empty()) {
    DEV_WARNING("vector to pop is empty");
  }
  else {
    m_vec.pop_back();
  }
}


// ------------------------- VectorPushPopOpt --------------------------
template <typename T>
VectorPushPopOpt<T>::VectorPushPopOpt(
  std::vector<T> &vec, std::optional<T> const &value)
  : m_vec(vec),
    m_doPushPop(value.has_value())
{
  if (m_doPushPop) {
    m_vec.push_back(*value);
  }
}


template <typename T>
VectorPushPopOpt<T>::VectorPushPopOpt(
  std::vector<T> &vec, std::optional<T> &&value)
  : m_vec(vec),
    m_doPushPop(value.has_value())
{
  if (m_doPushPop) {
    m_vec.push_back(std::move(*value));
  }
}


template <typename T>
VectorPushPopOpt<T>::~VectorPushPopOpt()
{
  if (m_doPushPop) {
    if (m_vec.empty()) {
      DEV_WARNING("vector to pop is empty");
    }
    else {
      m_vec.pop_back();
    }
  }
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_VECTOR_PUSH_POP_H
