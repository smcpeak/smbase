// optional-util.h
// Utilities related to `std::optional`.

#ifndef SMBASE_OPTIONAL_UTIL_H
#define SMBASE_OPTIONAL_UTIL_H

#include "optional-util-iface.h"       // interface for this module

#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <sstream>                     // std::ostringstream


template <class T>
std::string optionalToString(std::optional<T> const &o, char const *ifNone)
{
  if (o.has_value()) {
    std::ostringstream oss;
    oss << o.value();
    return oss.str();
  }
  else {
    return ifNone;
  }
}


template <class T>
std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt)
{
  if (opt.has_value()) {
    os << opt.value();
  }
  else {
    // This assumes 'null' will not be confused with whatever 'T' is.
    // That's not true in every possible case, but in practice it almost
    // always is, and I can handle exceptions separately.
    os << "null";
  }
  return os;
}


inline std::ostream& operator<< (std::ostream &os, std::nullopt_t const &)
{
  return os << "null";
}


template <typename T, typename FUNC>
std::optional<T> liftToOptional(
  std::optional<T> const &a,
  std::optional<T> const &b,
  FUNC func)
{
  if (!a.has_value()) {
    return b;
  }
  if (!b.has_value()) {
    return a;
  }

  return std::optional<T>(func(*a, *b));
}


#endif // SMBASE_OPTIONAL_UTIL_H
