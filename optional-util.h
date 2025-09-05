// optional-util.h
// Utilities related to `std::optional`.

#ifndef SMBASE_OPTIONAL_UTIL_H
#define SMBASE_OPTIONAL_UTIL_H

#include "optional-util-iface.h"       // interface for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <algorithm>                   // std::max
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <sstream>                     // std::ostringstream


OPEN_NAMESPACE(smbase)


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


template <typename T>
void optAccumulateMax(std::optional<T> &opt, T const &t)
{
  if (opt.has_value()) {
    opt = std::max(*opt, t);
  }
  else {
    opt = t;
  }
}


// This definition (and in fact signature) is not as general as it could
// be, but I choose simplicity and lighter dependencies over full
// generality.  See `optInvokeAlt` and its comments in
// `optional-util-test.cc` for more.
template <typename FUNC, typename T>
auto optInvoke(FUNC &&f, std::optional<T> const &opt)
  -> std::optional<decltype(f(*opt))>
{
  if (opt) {
    return std::make_optional(f(*opt));
  }
  else {
    return std::nullopt;
  }
}


// If `s` has a value, construct `DEST(*s)` and wrap that in an
// `optional`.  Otherwise return `nullopt`.
template <typename DEST, typename SRC>
std::optional<DEST> optFromOpt(std::optional<SRC> const &s)
{
  if (s.has_value()) {
    return std::optional<DEST>(*s);
  }
  else {
    return std::nullopt;
  }
}


// If `opt` has a value, invoke `method(args)` on it, then wrap the
// result in an `optional`.  Otherwise return `nullopt`.
#define OPT_INVOKE_METHOD(opt, method, ...) \
   (opt? std::make_optional(opt->method(__VA_ARGS__)) : std::nullopt)


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_OPTIONAL_UTIL_H
