// optional-util-iface.h
// Interface for `optional-util.h`.

#ifndef SMBASE_OPTIONAL_UTIL_IFACE_H
#define SMBASE_OPTIONAL_UTIL_IFACE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::{optional,nullopt_t} [n]
#include "smbase/std-string-fwd.h"     // std::string [n]

#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(smbase)


// 2025-08-03: `operator<<(std::ostream, std::optional)` has been moved
// to the `optional-opll` module.


// Convert 'o' to a string using its insert operator, or 'ifNone' if 'o'
// does not contain a value.
template <class T>
std::string optionalToString(std::optional<T> const &o, char const *ifNone);


// If either `a` or `b` is absent, then return the other one.  (Thus, if
// both are absent, the result is absent.)  Otherwise, return
// `std::optional<T>(func(*a, *b))`.
template <typename T, typename FUNC>
std::optional<T> liftToOptional(
  std::optional<T> const &a,
  std::optional<T> const &b,
  FUNC func);


// If `opt` has no value, set it to `t`.  Otherwise, set it to the
// larger of `t` and the value it contains.
template <typename T>
void optAccumulateMax(std::optional<T> &opt, T const &t);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_OPTIONAL_UTIL_IFACE_H
