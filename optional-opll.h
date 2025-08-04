// optional-opll.h
// `operator<<` for `std::optional`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_OPTIONAL_OPLL_H
#define SMBASE_OPTIONAL_OPLL_H

// Interface for this module, which also has the documentation.
#include "optional-opll-iface.h"

#include <iostream>                    // std::ostream
#include <optional>                    // std::optional


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


#endif // SMBASE_OPTIONAL_OPLL_H
