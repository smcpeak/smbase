// detect-libcpp.h
// Detect the libc++ in use and set macros accordingly.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_DETECT_LIBCPP_H
#define SMBASE_DETECT_LIBCPP_H

// This header may set the symbol we use to detect the libc++.
// See https://stackoverflow.com/questions/31657499/how-to-detect-stdlib-libc-in-the-preprocessor.
#include <ciso646>


#if defined(_LIBCPP_VERSION)

  #define SMBASE_LIBCPP_IS_CLANG

#elif defined(__GLIBCXX__)

  #define SMBASE_LIBCPP_IS_GNU
  #define _GLIBCXX_BEGIN_NAMESPACE_CXX11_INLINE inline _GLIBCXX_BEGIN_NAMESPACE_CXX11

#elif defined(_MSC_VER)

  #define SMBASE_LIBCPP_IS_MSVC

#else

  // Warn, but in a way that can be suppressed for -Werror.
  #ifndef SMBASE_SUPPRESS_UNKNOWN_LIBCPP_WARNING
    #warning "Unknown libc++"
  #endif

  #define SMBASE_LIBCPP_IS_UNKNOWN

#endif


#endif // SMBASE_DETECT_LIBCPP_H
