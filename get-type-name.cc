// get-type-name.cc
// Code for `get-type-name` module.

#include "get-type-name.h"             // this module

#include "string-util.h"               // replaceAll_sv

#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


std::string normalizeTypeName(std::string_view tname)
{
  // Currently, the only normalization is for anonymous namespaces.
  // This replaces the Clang notation with that used by GCC.  The latter
  // is slightly preferable due to being shorter.
  return replaceAll_sv(tname, "(anonymous namespace)", "{anonymous}");
}


CLOSE_NAMESPACE(smbase)


// EOF
