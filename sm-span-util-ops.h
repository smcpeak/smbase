// sm-span-util-ops.h
// Operations for `sm-span-util` module.

// See license.txt for copyright and terms of use.

// See comments in `sm-span-util-iface.h` for function specifications.

#ifndef SMBASE_SM_SPAN_UTIL_OPS_H
#define SMBASE_SM_SPAN_UTIL_OPS_H

#include "sm-span-util-iface.h"        // interface for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <numeric>                     // std::accumulate


OPEN_NAMESPACE(smbase)


template <typename T>
T spanSum(Span<T> span)
{
  return std::accumulate(span.begin(), span.end(), T());
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_SPAN_UTIL_OPS_H
