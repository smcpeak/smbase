// sm-compare.cc
// Code for sm-compare.h.

#include "sm-compare.h"                // this module

#include <cstring>                     // std::strcmp


StrongOrdering strongOrder(char const *a, char const *b)
{
  int c = std::strcmp(a, b);
  return strongOrder(c, 0);
}


// EOF
