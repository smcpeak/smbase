// sm-env.h
// Utilities related to querying environment variables.

#ifndef SMBASE_SM_ENV_H
#define SMBASE_SM_ENV_H

#include "sm-macros.h"                 // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)


// True if `envvar` is set to a value that `atoi` regards as non-zero.
bool envAsBool(char const *envvar);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_ENV_H

