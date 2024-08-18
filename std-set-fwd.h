// std-set-fwd.h
// Forward declarations for `<set>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_SET_FWD_H
#define SMBASE_STD_SET_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.
#include "std-functional-fwd.h"        // std::less [n]
#include "std-memory-fwd.h"            // std::allocator [n]


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_CONTAINER_NAMES

    template <typename _Key,
              typename _Compare,
              typename _Alloc>
    class set;

    template <typename _Key,
              typename _Compare,
              typename _Alloc>
    class multiset;

  SMBASE_LIBCPP_END_CONTAINER_NAMES

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <set>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _Key,
            typename _Compare = std::less<_Key>,
            typename _Alloc = std::allocator<_Key>>
  using set = std::set<_Key, _Compare, _Alloc>;

  template <typename _Key,
            typename _Compare = std::less<_Key>,
            typename _Alloc = std::allocator<_Key>>
  using multiset = std::multiset<_Key, _Compare, _Alloc>;
}


#endif // SMBASE_STD_SET_FWD_H
