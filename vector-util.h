// vector-util.h
// Utilities for std::vector.

#ifndef SMBASE_VECTOR_UTILS_H
#define SMBASE_VECTOR_UTILS_H

#include "container-util.h"            // CONTAINER_FOREACH
#include "overflow.h"                  // convertNumber
#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "xassert.h"                   // xfailure

#include <algorithm>                   // std::remove
#include <iostream>                    // std::ostream
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


// Apply 'op' to all elements.
template <class DEST, class SRC, class MapOperation>
std::vector<DEST> mapElements(std::vector<SRC> const &vec,
                              MapOperation op)
{
  std::vector<DEST> ret;
  ret.reserve(vec.size());
  for (SRC const &s : vec) {
    ret.push_back(op(s));
  }
  return ret;
}


// Convert elements from SRC to DEST.
template <class DEST, class SRC>
std::vector<DEST> convertElements(std::vector<SRC> const &vec)
{
  return mapElements<DEST, SRC>(vec,
    [](SRC const &s) { return DEST(s); });
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
  for (size_t i=0; i < vec.size(); i++) {
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


// Remove all occurrences of 'value' from 'vec'.
template <class T>
void vec_erase(std::vector<T> &vec, T const &value)
{
  vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
}


// Return the set of elements in 'vec'.
template <class T>
std::set<T> vec_element_set(std::vector<T> const &vec)
{
  std::set<T> ret;
  for (T const &t : vec) {
    ret.insert(t);
  }
  return ret;
}


// Report the first index of 'value' in 'vec', or -1 if it is not
// present.
template <class T>
long vec_find_index(std::vector<T> const &vec, T const &value)
{
  auto it = std::find(vec.begin(), vec.end(), value);
  if (it != vec.end()) {
    return convertNumber<long>(it - vec.begin());
  }
  else {
    return -1;
  }
}


// Return a new vector with the same elements as `vec` but in reverse
// order.
template <class T>
std::vector<T> vectorReverseOf(std::vector<T> const &vec)
{
  std::vector<T> ret;
  for (auto it = vec.crbegin(); it != vec.crend(); ++it) {
    ret.push_back(*it);
  }
  return ret;
}


#endif // SMBASE_VECTOR_UTILS_H
