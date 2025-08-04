// optional-util-iface.h
// Interface for `optional-util.h`.

#ifndef SMBASE_OPTIONAL_UTIL_IFACE_H
#define SMBASE_OPTIONAL_UTIL_IFACE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-optional-fwd.h"   // std::{optional,nullopt_t} [n]
#include "smbase/std-string-fwd.h"     // std::string [n]

#include <iosfwd>                      // std::ostream [n]


OPEN_NAMESPACE(smbase)


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


// Write `*opt`, or "null" if it is nullopt.
//
// I tried putting this into the `smbase` namespace, but then I have to
// add "using smbase::operator<<;" right after the #include (which has
// to be near the top of any .cc file) to find it.  And as if that was
// not bad enough, `clangd` complains, saying that declaration is
// unused, even though both GCC and Clang (as a compiler) require it.
template <class T>
inline std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt);

inline std::ostream& operator<< (std::ostream &os, std::nullopt_t const &);


#endif // SMBASE_OPTIONAL_UTIL_IFACE_H
