// std-utility-fwd.h
// Forward declarations for `<utility>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_UTILITY_FWD_H
#define SMBASE_STD_UTILITY_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    template <typename _T1, typename _T2>
    struct pair;

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <utility>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  using std::pair;
}


#endif // SMBASE_STD_UTILITY_FWD_H
