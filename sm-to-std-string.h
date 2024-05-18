// sm-to-std-string.h
// Utilities to convert between my 'OldSmbaseString' and 'std::string'.

// This module is intended to be temporary.  It will go away once I
// fully transition smbase to std::string.

#ifndef SM_TO_STD_STRING_H
#define SM_TO_STD_STRING_H

#include "str.h"                       // smbase OldSmbaseString

#include <string>                      // std::string
#include <vector>                      // std::vector


// Convert a vector of std::string to a vector of OldSmbaseString.
std::vector<OldSmbaseString> toSMStringVector(
  std::vector<std::string> const &vec);


#endif // SM_TO_STD_STRING_H
