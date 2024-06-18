// sm-env.cc
// Code for `sm-env.h`.

#include "sm-env.h"                    // this module

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstdlib>                     // std::{atoi, getenv}


OPEN_NAMESPACE(smbase)


bool envAsBool(char const *envvar)
{
  if (char const *value = std::getenv(envvar)) {
    return std::atoi(value) != 0;
  }
  else {
    return false;
  }
}


char const *envOrEmpty(char const *envvar)
{
  if (char const *value = std::getenv(envvar)) {
    return value;
  }
  else {
    return "";
  }
}


CLOSE_NAMESPACE(smbase)


// EOF
