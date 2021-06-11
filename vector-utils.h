// vector-utils.h
// Utilities for std::vector.

#ifndef INTERP_VECTOR_UTILS_H
#define INTERP_VECTOR_UTILS_H

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


#endif // INTERP_VECTOR_UTILS_H
