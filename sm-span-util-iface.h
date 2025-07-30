// sm-span-util-iface.h
// Interface for `sm-span-util` module.

// See license.txt for copyright and terms of use.

// Implementations are in `sm-span-util-ops.h`.

#ifndef SMBASE_SM_SPAN_UTIL_IFACE_H
#define SMBASE_SM_SPAN_UTIL_IFACE_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-span-fwd.h"        // smbase::Span


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


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_UTIL_IFACE_H
