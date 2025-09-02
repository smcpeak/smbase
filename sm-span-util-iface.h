// sm-span-util-iface.h
// Interface for `sm-span-util` module.

// See license.txt for copyright and terms of use.

// Implementations are in `sm-span-util-ops.h` and `sm-span-util.cc`.

#ifndef SMBASE_SM_SPAN_UTIL_IFACE_H
#define SMBASE_SM_SPAN_UTIL_IFACE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-span-fwd.h"        // smbase::Span [n]
#include "smbase/std-string-fwd.h"     // std::string [n]


OPEN_NAMESPACE(smbase)


// Return the sum of all elements in `span`.
//
// That is, return:
//
//   (((T() + span[0]) + span[1]) + ...) + span[span.size()-1]
//
// using whatever the infix `+` operator resolves to in that context.
// (So, for example, with `std::string`, this will concatenate all of
// the strings.)
template <typename T>
T spanSum(Span<T> span);


// Return elements of 'span' separated and terminated by 'sep'.  For
// example, `joinTerminate({"a", "b", "c"}, "_")` = "a_b_c_".
std::string joinTerminate(Span<std::string const> span,
                          std::string const &sep);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_UTIL_IFACE_H
