// get-type-name.h
// `GetTypeName` class template to get the name of its type argument.

// As it is derived from an SO post,
// this file is licensed under CC BY-SA 4.0:
// https://creativecommons.org/licenses/by-sa/4.0/

#ifndef SMBASE_GET_TYPE_NAME_H
#define SMBASE_GET_TYPE_NAME_H

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
#include <string_view>                 // std::string_view


OPEN_NAMESPACE(smbase)


// Based on https://stackoverflow.com/a/68139582/2659307 by "vrqq".  It
// has been modified, mostly by adding comments, but also making it more
// tolerant of an unsupported compiler.
template <typename T>
struct GetTypeName {
  /* Get a string view of the full name of this method.  It will contain
     the name of `T` buried somewhere inside it.

     According to the orignal SO post, the strings produced by the
     various supported compilers are:

       Clang:
         static std::string_view GetTypeName<void>::fullname_intern() [T = void]

       GCC:
         static constexpr std::string_view GetTypeName<T>::fullname_intern() [with T = void; std::string_view = std::basic_string_view<char>]

       MSVC:
         class std::basic_string_view<char,struct std::char_traits<char> > __cdecl GetTypeName<void>::fullname_intern(void)
  */
  constexpr static std::string_view fullname_intern()
  {
    #if defined(__clang__) || defined(__GNUC__)
      return __PRETTY_FUNCTION__;
    #elif defined(_MSC_VER)
      return __FUNCSIG__;
    #else
      // Unsupported compiler.
      return "unknown";
    #endif
  }

  // Get a string view of the name of `T`.
  constexpr static std::string_view name()
  {
    // Find `void` in the full name for the `void`-typed method.
    std::string_view voidName(GetTypeName<void>::fullname_intern());
    std::size_t prefix_len = voidName.find("void");
    if (prefix_len == voidName.npos) {
      return "unknown";
    }

    // By how many characters does the size vary when we use a type that
    // is one smaller than "void"?  We infer that this is the number of
    // times the type name appears in the string.
    std::size_t multiple = voidName.size() -
                           GetTypeName<int>::fullname_intern().size();

    // How long would the name be if the type name were removed?  The
    // "4" is the length of the word "void".
    std::size_t dummy_len = voidName.size() - 4*multiple;

    // Get the name that contains `T`.
    std::string_view tName(fullname_intern());

    // How long is the name of `T`?
    std::size_t target_len = (tName.size() - dummy_len)/multiple;

    // Pull that name out.
    std::string_view rv = tName.substr(prefix_len, target_len);

    // This code causes "unsigned int" to be turned into "int", which is
    // really bad for my primary usage, which is to get type names in
    // the `overflow` module.  So I'll just remove it and wait to see if
    // anything pops up requiring more adjustment.
    #if 0
    // If it has a space in it, assume that is separating an elaborated
    // type keyword (e.g., "class") from the actual name.  Evidently,
    // MSVC inserts such keywords.
    //
    // I'm not sure about using `rfind` rather than `find` here.  That
    // is what the code I'm basing this on did, but if the type is a
    // template specialization and some of the arguments also have
    // elaborated keywords, we could be throwing away most of the type
    // name.
    auto offset = rv.rfind(' ');
    if (offset == rv.npos) {
      // No keyword.
      return rv;
    }
    else {
      // Skip keyword.
      return rv.substr(offset+1);
    }
    #endif

    return rv;
  }

  // Make the type argument available to clients if needed.
  using type = T;

  // Expose the name as a variable.  Use `inline` so this declaration is
  // also a definition.
  constexpr inline static std::string_view value = name();
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_GET_TYPE_NAME_H
