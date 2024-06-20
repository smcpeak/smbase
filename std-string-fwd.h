// std-string-fwd.h
// Forward declarations for `<string>`.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_STD_STRING_FWD_H
#define SMBASE_STD_STRING_FWD_H

#include "detect-libcpp.h"             // SMBASE_LIBCPP_IS_XXX
#include "std-memory-fwd.h"            // std::allocator [n]


// -------------------------------- GNU --------------------------------
#if defined(SMBASE_LIBCPP_IS_GNU)

namespace std _GLIBCXX_VISIBILITY(default)
{
  inline namespace __cxx11 __attribute__((__abi_tag__("cxx11"))) {}

  _GLIBCXX_BEGIN_NAMESPACE_VERSION

    template <class CharT>
    struct char_traits;

    _GLIBCXX_BEGIN_NAMESPACE_CXX11_INLINE

      template <class _Elem, class _Traits, class _Alloc>
      class basic_string;

      using string = basic_string<char, char_traits<char>, allocator<char>>;
      using wstring = basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>>;
      using u16string = basic_string<char16_t, char_traits<char16_t>, allocator<char16_t>>;
      using u32string = basic_string<char32_t, char_traits<char32_t>, allocator<char32_t>>;

    _GLIBCXX_END_NAMESPACE_CXX11

  _GLIBCXX_END_NAMESPACE_VERSION
}


// ----------------------------- Fallback ------------------------------
#else

#include <string>

#endif


// ------------------------------ Generic ------------------------------
namespace stdfwd {
  template <typename Char, typename Traits = std::char_traits<Char>, typename Allocator = std::allocator<Char>>
  using basic_string = std::basic_string<Char, Traits, Allocator>;

  using std::string;
  using std::u16string;
  using std::u32string;
  using std::wstring;
}


#endif // SMBASE_STD_STRING_FWD_H
