// std-memory-fwd.h
// Forward declarations for `<memory>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_MEMORY_FWD_H
#define SMBASE_STD_MEMORY_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  template <class _Tp>
  class allocator;

  template <typename _Tp>
  class shared_ptr;

  template <typename _Tp>
  struct default_delete;

  template <typename _Tp, typename _Dp>
  class unique_ptr;

  template <typename _Tp>
  class weak_ptr;

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <memory>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  using std::allocator;

  using std::shared_ptr;

  using std::weak_ptr;

  template <typename _Tp, typename _Dp = std::default_delete<_Tp>>
  using unique_ptr = std::unique_ptr<_Tp, _Dp>;
}


#endif // SMBASE_STD_MEMORY_FWD_H
