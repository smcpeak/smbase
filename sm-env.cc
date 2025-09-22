// sm-env.cc
// Code for `sm-env.h`.

#include "sm-env.h"                    // this module

#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "stringb.h"                   // stringb

#include <cstdlib>                     // std::{atoi, getenv}
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


GetenvFunc *sm_getenv_func = nullptr;


char const *sm_getenv(char const *var)
{
  if (sm_getenv_func) {
    return sm_getenv_func(var);
  }
  else {
    return std::getenv(var);
  }
}



int envAsIntOr(int defaultValue, char const *envvar)
{
  if (char const *value = sm_getenv(envvar)) {
    return std::atoi(value);
  }
  else {
    return defaultValue;
  }
}


bool envAsBool(char const *envvar)
{
  return envAsIntOr(0, envvar) != 0;
}


char const *envOrEmpty(char const *envvar)
{
  return envAsStringOr("", envvar);
}


char const *envAsStringOr(char const *defaultValue, char const *envvar)
{
  if (char const *value = sm_getenv(envvar)) {
    return value;
  }
  else {
    return defaultValue;
  }
}


std::string getXDGConfigHome()
{
  // The specification at:
  //
  //   https://specifications.freedesktop.org/basedir-spec/latest/
  //
  // says that a non-absolute path should be ignored.  But checking for
  // that is non-portable due to Windows drive letters and UNC paths,
  // and does not seem very important, so I don't.

  if (char const *value = sm_getenv("XDG_CONFIG_HOME")) {
    return value;
  }

  if (char const *home = sm_getenv("HOME")) {
    return stringb(home << "/.config");
  }

  return ".config";
}


std::string getXDGStateHome()
{
  if (char const *value = sm_getenv("XDG_STATE_HOME")) {
    return value;
  }

  if (char const *home = sm_getenv("HOME")) {
    return stringb(home << "/.local/state");
  }

  return ".local/state";
}


CLOSE_NAMESPACE(smbase)


// EOF
