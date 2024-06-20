// detect-libcpp.h
// Detect the libc++ in use and set macros accordingly.

// See std-fwds.txt for overview and license.

#ifndef SMBASE_DETECT_LIBCPP_H
#define SMBASE_DETECT_LIBCPP_H

// This header may set the symbol we use to detect the libc++.
// See https://stackoverflow.com/questions/31657499/how-to-detect-stdlib-libc-in-the-preprocessor.
#include <ciso646>


// ------------------------------ Generic ------------------------------
// The generic section provides definitions that may be overridden in
// the libc++-specific sections.

// Surround names that go into namespace `std`.
#define SMBASE_LIBCPP_BEGIN_NAMESPACE_STD namespace std {
#define SMBASE_LIBCPP_END_NAMESPACE_STD }

// Surround names from most of libc++, excluding the containers.  I'm
// not sure what the pattern is; this is driven by what GNU libc++ does
// at the moment.  Also, Philip's stdfwd.hh seems to use this
// inconsistently so I might be misunderstanding something.
#define SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION /*nothing*/
#define SMBASE_LIBCPP_END_NAMESPACE_VERSION /*nothing*/

// Surround the names that come from <string>.
#define SMBASE_LIBCPP_BEGIN_STRING_NAMES /*nothing*/
#define SMBASE_LIBCPP_END_STRING_NAMES /*nothing*/

// Declare the thing that the `STRING_NAMES` refers to.
#define SMBASE_LIBCPP_DECLARE_STRING_NAMESPACE /*nothing*/

// Surround names from container headers like `<vector>`.
#define SMBASE_LIBCPP_BEGIN_CONTAINER_NAMES /*nothing*/
#define SMBASE_LIBCPP_END_CONTAINER_NAMES /*nothing*/



// ------------------------------- Clang -------------------------------
#if defined(_LIBCPP_VERSION)

  #define SMBASE_LIBCPP_IS_KNOWN
  #define SMBASE_LIBCPP_IS_CLANG

  // __config: #define _LIBCPP_BEGIN_NAMESPACE_STD namespace std { inline namespace _LIBCPP_ABI_NAMESPACE {
  #undef SMBASE_LIBCPP_BEGIN_NAMESPACE_STD
  #define SMBASE_LIBCPP_BEGIN_NAMESPACE_STD \
    _LIBCPP_BEGIN_NAMESPACE_STD

  // __config: #define _LIBCPP_END_NAMESPACE_STD }}
  #undef SMBASE_LIBCPP_END_NAMESPACE_STD
  #define SMBASE_LIBCPP_END_NAMESPACE_STD \
    _LIBCPP_END_NAMESPACE_STD


// -------------------------------- GNU --------------------------------
#elif defined(__GLIBCXX__)

  #define SMBASE_LIBCPP_IS_KNOWN
  #define SMBASE_LIBCPP_IS_GNU


  #undef SMBASE_LIBCPP_BEGIN_NAMESPACE_STD
  #define SMBASE_LIBCPP_BEGIN_NAMESPACE_STD \
    namespace std _GLIBCXX_VISIBILITY(default) {

  #undef SMBASE_LIBCPP_END_NAMESPACE_STD
  #define SMBASE_LIBCPP_END_NAMESPACE_STD }


  #undef SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION
  #define SMBASE_LIBCPP_BEGIN_NAMESPACE_VERSION \
    _GLIBCXX_BEGIN_NAMESPACE_VERSION

  #undef SMBASE_LIBCPP_END_NAMESPACE_VERSION
  #define SMBASE_LIBCPP_END_NAMESPACE_VERSION \
    _GLIBCXX_END_NAMESPACE_VERSION


  #undef SMBASE_LIBCPP_DECLARE_STRING_NAMESPACE
  #define SMBASE_LIBCPP_DECLARE_STRING_NAMESPACE \
    inline namespace __cxx11 __attribute__((__abi_tag__("cxx11"))) {}

  #undef SMBASE_LIBCPP_BEGIN_STRING_NAMES
  #define SMBASE_LIBCPP_BEGIN_STRING_NAMES \
    inline _GLIBCXX_BEGIN_NAMESPACE_CXX11

  #undef SMBASE_LIBCPP_END_STRING_NAMES
  #define SMBASE_LIBCPP_END_STRING_NAMES \
    _GLIBCXX_END_NAMESPACE_CXX11


  #undef SMBASE_LIBCPP_BEGIN_NAMESPACE_CONTAINER
  #define SMBASE_LIBCPP_BEGIN_NAMESPACE_CONTAINER \
    _GLIBCXX_BEGIN_NAMESPACE_CONTAINER

  #undef SMBASE_LIBCPP_END_NAMESPACE_CONTAINER
  #define SMBASE_LIBCPP_END_NAMESPACE_CONTAINER \
    _GLIBCXX_END_NAMESPACE_CONTAINER


// ------------------------------- MSVC --------------------------------
// This is not implemented.
#elif 0 && defined(_MSC_VER)

  #define SMBASE_LIBCPP_IS_KNOWN
  #define SMBASE_LIBCPP_IS_MSVC


// ----------------------------- Fallback ------------------------------
#else

  // Warn, but in a way that can be suppressed for -Werror.
  #ifndef SMBASE_SUPPRESS_UNKNOWN_LIBCPP_WARNING
    #warning "Unknown libc++"
  #endif

  #define SMBASE_LIBCPP_IS_UNKNOWN

#endif


#endif // SMBASE_DETECT_LIBCPP_H
