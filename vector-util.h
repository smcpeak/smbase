// vector-util.h
// Utilities for `std::vector`.

// Convention: Names begin with "vec" and use camelCase.

#ifndef SMBASE_VECTOR_UTIL_H
#define SMBASE_VECTOR_UTIL_H

#include "container-util.h"            // CONTAINER_FOREACH
#include "overflow.h"                  // convertNumber
#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "xassert.h"                   // xfailure

#include <algorithm>                   // std::remove
#include <cstddef>                     // std::size_t
#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <set>                         // std::set
#include <sstream>                     // std::ostringstream
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
T vecAccumulateWith(std::vector<T> const &vec, T const &separator)
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


template <class T>
T accumulateWith(std::vector<T> const &vec, T const &separator)
  DEPRECATED("2024-06-28: Use `vecAccumulateWith` instead.");

template <class T>
T accumulateWith(std::vector<T> const &vec, T const &separator)
{
  return vecAccumulateWith(vec, separator);
}


// Like above, but apply 'op' to each element first.
template <class A, class B, class MapOperation>
B vecAccumulateWithMap(std::vector<A> const &vec, MapOperation op,
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


template <class A, class B, class MapOperation>
B accumulateWithMap(std::vector<A> const &vec, MapOperation op,
                    B const &separator)
  DEPRECATED("2024-06-28: Use `vecAccumulateWithMap` instead.");

template <class A, class B, class MapOperation>
B accumulateWithMap(std::vector<A> const &vec, MapOperation op,
                    B const &separator)
{
  return vecAccumulateWithMap(vec, op, separator);
}


// Apply 'op' to all elements.
template <class DEST, class SRC, class MapOperation>
std::vector<DEST> vecMapElements(std::vector<SRC> const &vec,
                                 MapOperation op)
{
  std::vector<DEST> ret;
  ret.reserve(vec.size());
  for (SRC const &s : vec) {
    ret.push_back(op(s));
  }
  return ret;
}


template <class DEST, class SRC, class MapOperation>
std::vector<DEST> mapElements(std::vector<SRC> const &vec,
                              MapOperation op)
  DEPRECATED("2024-06-28: Use `vecMapElements` instead.");

template <class DEST, class SRC, class MapOperation>
std::vector<DEST> mapElements(std::vector<SRC> const &vec,
                              MapOperation op)
{
  return vecMapElements<DEST>(vec, op);
}


// Convert elements from SRC to DEST.
template <class DEST, class SRC>
std::vector<DEST> vecConvertElements(std::vector<SRC> const &vec)
{
  return vecMapElements<DEST, SRC>(vec,
    [](SRC const &s) { return DEST(s); });
}


template <class DEST, class SRC>
std::vector<DEST> convertElements(std::vector<SRC> const &vec)
  DEPRECATED("2024-06-28: Use `vecConvertElements` instead.");

template <class DEST, class SRC>
std::vector<DEST> convertElements(std::vector<SRC> const &vec)
{
  return vecConvertElements<DEST>(vec);
}


// Print 'vec' to 'os' by using 'operator<<' on each element.  Returns
// 'os'.
//
// Note that there is an operator<< in string-util.h that operates on
// std::vector<std::string> and behaves differently (it quotes the
// strings).
template <class T>
std::ostream& operator<< (std::ostream &os, std::vector<T> const &vec)
{
  os << '[';
  for (std::size_t i=0; i < vec.size(); i++) {
    if (i > 0) {
      os << ' ';
    }
    os << vec[i];
  }
  os << ']';
  return os;
}


// Note there is an overload for std::string in string-util.h that
// behaves differently.
template <class T>
std::string toString(std::vector<T> const &vec)
{
  std::ostringstream oss;
  oss << vec;
  return oss.str();
}


// Return the back element of 'vec', or 'value' if it is empty.
template <class T>
T vecBackOr(std::vector<T> const &vec, T const &value)
{
  if (vec.empty()) {
    return value;
  }
  else {
    return vec.back();
  }
}



template <class T>
T back_or_value(std::vector<T> const &vec, T const &value)
  DEPRECATED("2024-06-28: Use `vecBackOr` instead.");

template <class T>
T back_or_value(std::vector<T> const &vec, T const &value)
{
  return vecBackOr(vec, value);
}


// Return the back element of 'vec', or `nullptr` if it is empty.
template <class T>
T *vecBackOrNull(std::vector<T*> const &vec)
{
  return vecBackOr(vec, (T*)nullptr);
}


template <class T>
T *back_or_null(std::vector<T*> const &vec)
  DEPRECATED("2024-06-28: Use `vecBackOrNull` instead.");

template <class T>
T *back_or_null(std::vector<T*> const &vec)
{
  return vecBackOrNull(vec);
}


// Pop the last element of 'vec', requiring it to equal 'value'.
template <class T>
void vecPopCheck(std::vector<T> &vec, T const &value)
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


template <class T>
void pop_check(std::vector<T> &vec, T const &value)
  DEPRECATED("2024-06-28: Use `vecPopCheck` instead.");

template <class T>
void pop_check(std::vector<T> &vec, T const &value)
{
  return vecPopCheck(vec, value);
}


// Return true if any element in 'vec' compares equal to 'value' using
// linear search.
template <class T>
bool vecContains(std::vector<T> const &vec, T const &value)
{
  CONTAINER_FOREACH(vec, it) {
    if (*it == value) {
      return true;
    }
  }
  return false;
}


template <class T>
bool vec_contains(std::vector<T> const &vec, T const &value)
  DEPRECATED("2024-06-28: Use `vecContains` instead.");

template <class T>
bool vec_contains(std::vector<T> const &vec, T const &value)
{
  return vecContains(vec, value);
}


// Remove all occurrences of 'value' from 'vec'.
template <class T>
void vecEraseAll(std::vector<T> &vec, T const &value)
{
  vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
}


template <class T>
void vec_erase(std::vector<T> &vec, T const &value)
  DEPRECATED("2024-06-28: Use `vecEraseAll` instead.");

template <class T>
void vec_erase(std::vector<T> &vec, T const &value)
{
  return vecEraseAll(vec, value);
}


// Return the set of elements in 'vec'.
template <class T>
std::set<T> vecToElementSet(std::vector<T> const &vec)
{
  std::set<T> ret;
  for (T const &t : vec) {
    ret.insert(t);
  }
  return ret;
}


template <class T>
std::set<T> vec_element_set(std::vector<T> const &vec)
  DEPRECATED("2024-06-28: Use `vecToElementSet` instead.");

template <class T>
std::set<T> vec_element_set(std::vector<T> const &vec)
{
  return vecToElementSet(vec);
}


// Return the first index of 'vec' where the element equals 't', or
// an empty value.
template <class T>
std::optional<std::size_t> vecFindIndex(std::vector<T> const &vec,
                                        T const &t)
{
  for (std::size_t i=0; i < vec.size(); ++i) {
    if (vec[i] == t) {
      return i;
    }
  }

  return std::nullopt;
}


template <class T>
long vec_find_index(std::vector<T> const &vec, T const &value)
  DEPRECATED("2024-06-28: Use `vecFindIndex` instead.");

template <class T>
long vec_find_index(std::vector<T> const &vec, T const &value)
{
  if (std::optional<std::size_t> indexOpt = vecFindIndex(vec, value)) {
    return convertNumber<long>(*indexOpt);
  }
  else {
    return -1;
  }
}


template <class T>
std::optional<std::size_t> vectorFirstIndexOf(std::vector<T> const &vec,
                                              T const &t)
  DEPRECATED("2024-06-28: Use `vecFindIndex` instead.");

template <class T>
std::optional<std::size_t> vectorFirstIndexOf(std::vector<T> const &vec,
                                              T const &t)
{
  return vecFindIndex(vec, t);
}


// Return a new vector with the same elements as `vec` but in reverse
// order.
template <class T>
std::vector<T> vecReverseOf(std::vector<T> const &vec)
{
  std::vector<T> ret;
  for (auto it = vec.crbegin(); it != vec.crend(); ++it) {
    ret.push_back(*it);
  }
  return ret;
}


template <class T>
std::vector<T> vectorReverseOf(std::vector<T> const &vec)
  DEPRECATED("2024-06-28: Use `vecReverseOf` instead.");

template <class T>
std::vector<T> vectorReverseOf(std::vector<T> const &vec)
{
  return vecReverseOf(vec);
}


// Number of elements in common at the start of 'a' and 'b'.
template <class T>
std::size_t vecCommonPrefixLength(std::vector<T> const &a,
                                  std::vector<T> const &b)
{
  std::size_t i = 0;
  for (; i < a.size() && i < b.size(); ++i) {
    if (!( a[i] == b[i] )) {
      break;
    }
  }
  return i;
}


template <class T>
std::size_t commonPrefixLength(std::vector<T> const &a,
                               std::vector<T> const &b)
  DEPRECATED("2024-06-28: Use `vecCommonPrefixLength` instead.");

template <class T>
std::size_t commonPrefixLength(std::vector<T> const &a,
                               std::vector<T> const &b)
{
  return vecCommonPrefixLength(a, b);
}


#endif // SMBASE_VECTOR_UTIL_H
