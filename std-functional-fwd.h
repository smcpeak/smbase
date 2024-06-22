// std-functional-fwd.h
// Forward declarations for `<functional>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_FUNCTIONAL_FWD_H
#define SMBASE_STD_FUNCTIONAL_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    template <typename _Signature>
    class function;

    template <typename _Tp>
    struct hash;

    template <typename _Tp>
    struct equal_to;

    template <typename _Tp>
    struct not_equal_to;

    template <typename _Tp>
    struct greater;

    template <typename _Tp>
    struct less;

    template <typename _Tp>
    struct greater_equal;

    template <typename _Tp>
    struct less_equal;

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <functional>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  using std::function;

  using std::hash;

  template <typename _Tp = void>
  using equal_to = std::equal_to<_Tp>;

  template <typename _Tp = void>
  using not_equal_to = std::not_equal_to<_Tp>;

  template <typename _Tp = void>
  using greater = std::greater<_Tp>;

  template <typename _Tp = void>
  using less = std::less<_Tp>;

  template <typename _Tp = void>
  using greater_equal = std::greater_equal<_Tp>;

  template <typename _Tp = void>
  using less_equal = std::less_equal<_Tp>;
}


#endif // SMBASE_STD_FUNCTIONAL_FWD_H
