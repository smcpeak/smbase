// type-name-and-size.h
// `TypeNameAndSize` class.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_TYPE_NAME_AND_SIZE_H
#define SMBASE_TYPE_NAME_AND_SIZE_H

#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <climits>                     // CHAR_BIT
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// Carry the name of a C++ type and its size in bits.
class TypeNameAndSize {
public:      // data
  // The name as it would appear in C++ source code, e.g.,
  // "unsigned long int".
  std::string m_name;

  // The number of bits it occupies, typically within the C++
  // implementation that has compiled this code, although this class
  // does not care.
  int m_bits;

public:      // methods
  // create-tuple-class: declarations for TypeNameAndSize +move
  /*AUTO_CTC*/ explicit TypeNameAndSize(std::string const &name, int bits);
  /*AUTO_CTC*/ explicit TypeNameAndSize(std::string &&name, int bits);
  /*AUTO_CTC*/ TypeNameAndSize(TypeNameAndSize const &obj) noexcept;
  /*AUTO_CTC*/ TypeNameAndSize(TypeNameAndSize &&obj) noexcept;
  /*AUTO_CTC*/ TypeNameAndSize &operator=(TypeNameAndSize const &obj) noexcept;
  /*AUTO_CTC*/ TypeNameAndSize &operator=(TypeNameAndSize &&obj) noexcept;

  // Return a string like:
  //
  //   "int" (32 bits)
  //
  std::string toString() const;
};


template <typename T>
TypeNameAndSize makeTypeNameAndSizeForType()
{
  return TypeNameAndSize(
    std::string(GetTypeName<T>::value),
    sizeof(T) * CHAR_BIT);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_TYPE_NAME_AND_SIZE_H
