// std-list-fwd.h
// Forward declarations for `<list>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_LIST_FWD_H
#define SMBASE_STD_LIST_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.
#include "std-memory-fwd.h"            // std::allocator [n]


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_DECLARE_STRING_NAMESPACE

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    SMBASE_LIBCPP_BEGIN_CONTAINER_NAMES

      // Obviously, `list` is not a "string" name.  But according to
      // `cpp-std-fwd`, it lives in the same "inline c++11" namespace
      // that the string classes do, so I'm sort of abusing the existing
      // macro rather than make another.
      SMBASE_LIBCPP_BEGIN_STRING_NAMES

        template <typename _Tp, typename _Alloc>
        class list;

      SMBASE_LIBCPP_END_STRING_NAMES

    SMBASE_LIBCPP_END_CONTAINER_NAMES

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <list>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
  using list = std::list<_Tp, _Alloc>;
}


#endif // SMBASE_STD_LIST_FWD_H
