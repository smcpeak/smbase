// std-optional-fwd.h
// Forward declarations for `<optional>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_OPTIONAL_FWD_H
#define SMBASE_STD_OPTIONAL_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    template <typename _Tp>
    class optional;

    struct nullopt_t;

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <optional>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _Tp>
  using optional = std::optional<_Tp>;

  using std::nullopt_t;
}


#endif // SMBASE_STD_OPTIONAL_FWD_H
