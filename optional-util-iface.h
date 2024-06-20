// optional-util-iface.h
// Interface for `optional-util.h`.

#ifndef SMBASE_OPTIONAL_UTIL_IFACE_H
#define SMBASE_OPTIONAL_UTIL_IFACE_H

#include "std-string-fwd.h"            // std::string

#include <optional>                    // std::{optional,nullopt_t} [n]
#include <iosfwd>                      // std::ostream [n]


// Convert 'o' to a string using its insert operator, or 'ifNone' if 'o'
// does not contain a value.
template <class T>
std::string optionalToString(std::optional<T> const &o, char const *ifNone);


template <class T>
std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt);


inline std::ostream& operator<< (std::ostream &os, std::nullopt_t const &);


#endif // SMBASE_OPTIONAL_UTIL_IFACE_H
