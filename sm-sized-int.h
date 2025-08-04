// sm-sized-int.h
// Wrapper around `<cstdint>` with some additional convenience macros.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SM_SIZED_INT_H
#define SMBASE_SM_SIZED_INT_H

#include <cstdint>                     // std::{int32_t, etc.}


// Do `macro` for each of the sized integer types.
#define SM_FOREACH_SIZED_INT(macro) \
  macro(std::int8_t)                \
  macro(std::uint8_t)               \
  macro(std::int16_t)               \
  macro(std::uint16_t)              \
  macro(std::int32_t)               \
  macro(std::uint32_t)              \
  macro(std::int64_t)               \
  macro(std::uint64_t)


#endif // SMBASE_SM_SIZED_INT_H
