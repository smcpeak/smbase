// optional-util-iface.h
// Interface for `optional-util.h`.

#ifndef SMBASE_OPTIONAL_UTIL_IFACE_H
#define SMBASE_OPTIONAL_UTIL_IFACE_H

#include "std-string-fwd.h"            // std::string [n]

#include <optional>                    // std::{optional,nullopt_t} [n]
#include <iosfwd>                      // std::ostream [n]


// Convert 'o' to a string using its insert operator, or 'ifNone' if 'o'
// does not contain a value.
template <class T>
std::string optionalToString(std::optional<T> const &o, char const *ifNone);


template <class T>
std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt);


inline std::ostream& operator<< (std::ostream &os, std::nullopt_t const &);


// If either `a` or `b` is absent, then return the other one.  (Thus, if
// both are absent, the result is absent.)  Otherwise, return
// `std::optional<T>(func(*a, *b))`.
template <typename T, typename FUNC>
std::optional<T> liftToOptional(
  std::optional<T> const &a,
  std::optional<T> const &b,
  FUNC func);


#endif // SMBASE_OPTIONAL_UTIL_IFACE_H
