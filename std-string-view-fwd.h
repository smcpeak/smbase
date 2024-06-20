// std-string-view-fwd.h
// Forward declarations for `<string_view>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_STRING_VIEW_FWD_H
#define SMBASE_STD_STRING_VIEW_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_XXX
#include "std-string-fwd.h"            // std::char_traits [n]


// -------------------------------- GNU --------------------------------
#if defined(SMBASE_LIBCPP_IS_GNU)

namespace std _GLIBCXX_VISIBILITY(default)
{
  inline namespace __cxx11 __attribute__((__abi_tag__("cxx11"))) {}

  _GLIBCXX_BEGIN_NAMESPACE_VERSION

    template <typename _CharT, typename _Traits>
    class basic_string_view;

    using string_view = basic_string_view<char, std::char_traits<char>>;
    using wstring_view = basic_string_view<wchar_t, std::char_traits<wchar_t>>;
    using u16string_view = basic_string_view<char16_t, std::char_traits<char16_t>>;
    using u32string_view = basic_string_view<char32_t, std::char_traits<char32_t>>;

  _GLIBCXX_END_NAMESPACE_VERSION
}


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
