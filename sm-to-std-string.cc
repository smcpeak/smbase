// sm-to-std-string.cc
// Code for sm-to-std-string.h.

#include "sm-to-std-string.h"          // this module


std::vector<OldSmbaseString> toSMStringVector(
  std::vector<std::string> const &vec)
{
  std::vector<OldSmbaseString> ret;
  for (std::string const &s : vec) {
    ret.push_back(OldSmbaseString(s.c_str()));
  }
  return ret;
}


// EOF
