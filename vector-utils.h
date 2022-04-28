// vector-utils.h
// Utilities for std::vector.

#ifndef SMBASE_VECTOR_UTILS_H
#define SMBASE_VECTOR_UTILS_H

#include "dev-warning.h"               // DEV_WARNING
#include "sm-macros.h"                 // NO_OBJECT_COPIES

#include <vector>                      // std::vector


// If 'vec' is empty, return T().  Otherwise, return:
//
//   vec[0] + separator + vec[1] + separator + ... + vec[size()-1]
//
// One application is when T is 'string'; then 'accumulateWith' is like
// 'join' from other languages.
//
// This requires that T have a default copy constructor and 'operator+='.
template <class T>
T accumulateWith(std::vector<T> const &vec, T const &separator)
{
  if (vec.empty()) {
    return T();
  }

  auto it = vec.begin();
  T ret(*it);
  ++it;
  for (; it != vec.end(); ++it) {
    ret += separator;
    ret += *it;
  }

  return ret;
}


// Push an element onto a vector in the constructor, then pop an element
// (prsumably the same one, but that is not checked) in the destructor.
template <class T>
class VectorPushPop {
  NO_OBJECT_COPIES(VectorPushPop);

public:      // data
  // The vector to push onto and pop off of.
  std::vector<T> &m_vector;

public:      // methods
  VectorPushPop(std::vector<T> &vector, T const &element)
    : m_vector(vector)
  {
    m_vector.push_back(element);
  }

  ~VectorPushPop()
  {
    if (m_vector.empty()) {
      DEV_WARNING("vector to pop is empty");
    }
    else {
      m_vector.pop_back();
    }
  }
};


// Return the back element of 'vec', or NULL if it is empty.
template <class T>
T *back_or_null(std::vector<T*> const &vec)
{
  if (vec.empty()) {
    return NULL;
  }
  else {
    return vec.back();
  }
}


#endif // SMBASE_VECTOR_UTILS_H
