// std-vector-fwd.h
// Forward declarations for `<vector>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_VECTOR_FWD_H
#define SMBASE_STD_VECTOR_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.
#include "std-memory-fwd.h"            // std::allocator [n]


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_CONTAINER_NAMES

    template <typename _Tp, typename _Alloc>
    class vector;

  SMBASE_LIBCPP_END_CONTAINER_NAMES

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <vector>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
  using vector = std::vector<_Tp, _Alloc>;
}


#endif // SMBASE_STD_VECTOR_FWD_H
