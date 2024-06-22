// std-map-fwd.h
// Forward declarations for `<map>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_MAP_FWD_H
#define SMBASE_STD_MAP_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.
#include "std-functional-fwd.h"        // std::less [n]
#include "std-memory-fwd.h"            // std::allocator [n]
#include "std-utility-fwd.h"           // std::utility [n]


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_CONTAINER_NAMES

    template <typename _Key,
              typename _Value,
              typename _Compare,
              typename _Alloc>
    class map;

    template <typename _Key,
              typename _Value,
              typename _Compare,
              typename _Alloc>
    class multimap;

  SMBASE_LIBCPP_END_CONTAINER_NAMES

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <map>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _Key,
            typename _Value,
            typename _Compare = std::less<_Key>,
            typename _Alloc = std::allocator<std::pair<const _Key, _Value>>>
  using map = std::map<_Key, _Value, _Compare, _Alloc>;

  template <typename _Key,
            typename _Value,
            typename _Compare = std::less<_Key>,
            typename _Alloc = std::allocator<std::pair<const _Key, _Value>>>
  using multimap = std::multimap<_Key, _Value, _Compare, _Alloc>;
}


#endif // SMBASE_STD_MAP_FWD_H
