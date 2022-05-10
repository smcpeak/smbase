// vector-utils.h
// Utilities for std::vector.

#ifndef SMBASE_VECTOR_UTILS_H
#define SMBASE_VECTOR_UTILS_H

#include "container-utils.h"           // CONTAINER_FOREACH
#include "dev-warning.h"               // DEV_WARNING
#include "xassert.h"                   // xfailure
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


// Like above, but apply 'op' to each element first.
template <class A, class B, class MapOperation>
B accumulateWithMap(std::vector<A> const &vec, MapOperation op,
                    B const &separator)
{
  if (vec.empty()) {
    return B();
  }

  auto it = vec.begin();
  B ret(op(*it));
  ++it;
  for (; it != vec.end(); ++it) {
    ret += separator;
    ret += op(*it);
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


// Return the back element of 'vec', or 'value' if it is empty.
template <class T>
T back_or_value(std::vector<T> const &vec, T const &value)
{
  if (vec.empty()) {
    return value;
  }
  else {
    return vec.back();
  }
}


// Return the back element of 'vec', or NULL if it is empty.
template <class T>
T *back_or_null(std::vector<T*> const &vec)
{
  return back_or_value(vec, (T*)NULL);
}


// Pop the last element of 'vec', requiring it to equal 'value'.
template <class T>
void pop_check(std::vector<T> &vec, T const &value)
{
  if (vec.empty()) {
    xfailure("Cannot pop empty vector.");
  }
  else if (!( vec.back() == value )) {
    xfailure("Value does not equal vector back.");
  }
  else {
    vec.pop_back();
  }
}


// Return true if any element in 'vec' compares equal to 'value' using
// linear search.
template <class T>
bool vec_contains(std::vector<T> const &vec, T const &value)
{
  CONTAINER_FOREACH(vec, it) {
    if (*it == value) {
      return true;
    }
  }
  return false;
}


#endif // SMBASE_VECTOR_UTILS_H
