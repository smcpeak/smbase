// optional-opll-iface.h
// Interface for `optional-opll` module.

// See license.txt for copyright and terms of use.

// This has been split off from `optional-util` because it has to be in
// the global namespace, so I think a client should have to request it
// specifically.
//
// I tried putting this into the `smbase` namespace, but then I have to
// add "using smbase::operator<<;" right after the #include (which has
// to be near the top of any .cc file) to find it.  And as if that was
// not bad enough, `clangd` complains, saying that declaration is
// unused, even though both GCC and Clang (as a compiler) require it.

// As mentioned, this file often has to go right at the start of a .cc
// file so the declaration is visible to templates that use
// `operator<<`.  Consequently, this file is very light on dependencies
// and should be kept that way.

#ifndef SMBASE_OPTIONAL_OPLL_IFACE_H
#define SMBASE_OPTIONAL_OPLL_IFACE_H

#include "smbase/std-optional-fwd.h"   // std::{optional,nullopt_t} [n]

#include <iosfwd>                      // std::ostream [n]


// Write `*opt`, or "null" if it is nullopt.
template <class T>
inline std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt);

inline std::ostream& operator<< (std::ostream &os, std::nullopt_t const &);


#endif // SMBASE_OPTIONAL_OPLL_IFACE_H
