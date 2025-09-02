// std-tuple-fwd.h
// Forward declarations for `<tuple>`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_STD_TUPLE_FWD_H
#define SMBASE_STD_TUPLE_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    template <typename... _Elements>
    class tuple;

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <tuple>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  using std::tuple;
}


#endif // SMBASE_STD_TUPLE_FWD_H
