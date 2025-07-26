// sm-span-util.h
// Utilities for `smbase::Span`.

// See license.txt for copyright and terms of use.

// Implementations are in `sm-span-util-ops.h`.

#ifndef SMBASE_SM_SPAN_UTIL_H
#define SMBASE_SM_SPAN_UTIL_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-span-fwd.h"        // smbase::Span


OPEN_NAMESPACE(smbase)


// Return the sum of all elements in `span`.
template <typename T>
T spanSum(Span<T> span);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_UTIL_H
