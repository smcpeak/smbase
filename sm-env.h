// sm-env.h
// Utilities related to querying environment variables.

#ifndef SMBASE_SM_ENV_H
#define SMBASE_SM_ENV_H

#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "std-string-fwd.h"            // std::string


OPEN_NAMESPACE(smbase)


// The type of a function that acts like `std::getenv`.
typedef char const *GetenvFunc(char const *var);

// Optional override for `sm_getenv`.
extern GetenvFunc *sm_getenv_func;

// Call `std::getenv`, but if `sm_getenv_func` is set, call that
// instead.  This is meant to allow hooking getenv for testing; outside
// of code that may want to be hookable for testing, there is no
// particular need to use this instead of directly calling
// `std::getenv`.
char const *sm_getenv(char const *var);


// True if `envvar` is set to a value that `atoi` regards as non-zero.
bool envAsBool(char const *envvar);

// Return the value of `envvar`, or an empty string if it is not set.
char const *envOrEmpty(char const *envvar);

// Get the value of $XDG_CONFIG_HOME or its default if unset.  The
// default is "$HOME/.config".  If $HOME is unset, returns ".config".
//
// This is the directory under which user configuration files should go.
std::string getXDGConfigHome();

// Similarly, get $XDG_STATE_HOME.
std::string getXDGStateHome();


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_ENV_H

