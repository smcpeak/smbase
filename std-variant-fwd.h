// std-variant-fwd.h
// Forward declarations for `<variant>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_VARIANT_FWD_H
#define SMBASE_STD_VARIANT_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  template <typename... _Types>
  class variant;

  struct monostate;

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <variant>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  using std::monostate;
  using std::variant;
}


#endif // SMBASE_STD_VARIANT_FWD_H
