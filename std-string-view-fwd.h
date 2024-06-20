// std-string-view-fwd.h
// Forward declarations for `<string_view>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_STRING_VIEW_FWD_H
#define SMBASE_STD_STRING_VIEW_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_KNOWN, etc.
#include "std-string-fwd.h"            // std::char_traits [n]


// ------------------------------- Known -------------------------------
#if defined(SMBASE_LIBCPP_IS_KNOWN)

SMBASE_LIBCPP_BEGIN_NAMESPACE_STD

  SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION

    template <typename _CharT, typename _Traits>
    class basic_string_view;

    using string_view = basic_string_view<char, std::char_traits<char>>;
    using wstring_view = basic_string_view<wchar_t, std::char_traits<wchar_t>>;
    using u16string_view = basic_string_view<char16_t, std::char_traits<char16_t>>;
    using u32string_view = basic_string_view<char32_t, std::char_traits<char32_t>>;

  SMBASE_LIBCPP_END_NAMESPACE_VERSION

SMBASE_LIBCPP_END_NAMESPACE_STD


// ----------------------------- Fallback ------------------------------
#else

#include <string_view>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename _CharT, typename _Traits = std::char_traits<_CharT>>
  using basic_string_view = std::basic_string_view<_CharT, _Traits>;

  using std::string_view;
  using std::u16string_view;
  using std::u32string_view;
  using std::wstring_view;
}


#endif // SMBASE_STD_STRING_VIEW_FWD_H
