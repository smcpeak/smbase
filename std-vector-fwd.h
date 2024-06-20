// std-vector-fwd.h
// Forward declarations for `<vector>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_VECTOR_FWD_H
#define SMBASE_STD_VECTOR_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_XXX
#include "std-memory-fwd.h"            // std::allocator [n]


// -------------------------------- GNU --------------------------------
#if defined(SMBASE_LIBCPP_IS_GNU)

namespace std _GLIBCXX_VISIBILITY(default)
{
  inline namespace __cxx11 __attribute__((__abi_tag__("cxx11"))) {}

  _GLIBCXX_BEGIN_NAMESPACE_VERSION

    _GLIBCXX_BEGIN_NAMESPACE_CONTAINER

      template <typename _Tp, typename _Alloc>
      class vector;

    _GLIBCXX_END_NAMESPACE_CONTAINER

  _GLIBCXX_END_NAMESPACE_VERSION
}


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
