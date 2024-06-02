// string-hash.h
// Compute the hash of a string.

// This file is in the public domain.

#ifndef SMBASE_STRING_HASH_H
#define SMBASE_STRING_HASH_H

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t


OPEN_NAMESPACE(smbase)


// Compute the hash of the `size` bytes at `data`.
unsigned stringHash(char const *data, std::size_t size);


// NUL-terminated variant.
unsigned stringHashNulTerm(char const *cstr);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_STRING_HASH_H
