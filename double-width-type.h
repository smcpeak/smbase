// double-width-type.h
// `DoubleWidthType` template meta-lookup.

// This file is in the public domain.

#ifndef SMBASE_DOUBLE_WIDTH_TYPE_H
#define SMBASE_DOUBLE_WIDTH_TYPE_H

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstdint>                     // std::uint{8,16,32,64}_t


OPEN_NAMESPACE(smbase)


// Template primary is not defined.
template <typename T>
struct DoubleWidthType;

// Then we have certain specializations, one for each type we know how
// to double.

template <>
struct DoubleWidthType<uint8_t> {
  typedef uint16_t DWT;
};

template <>
struct DoubleWidthType<uint16_t> {
  typedef uint32_t DWT;
};

template <>
struct DoubleWidthType<uint32_t> {
  typedef uint64_t DWT;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_DOUBLE_WIDTH_TYPE_H
