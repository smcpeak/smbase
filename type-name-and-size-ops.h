// type-name-and-size-ops.h
// Operations for `type-name-and-size.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_TYPE_NAME_AND_SIZE_OPS_H
#define SMBASE_TYPE_NAME_AND_SIZE_OPS_H

#include "smbase/type-name-and-size.h" // this module

#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <climits>                     // CHAR_BIT


OPEN_NAMESPACE(smbase)


template <typename T>
TypeNameAndSize makeTypeNameAndSizeForType()
{
  return TypeNameAndSize(
    std::string(GetTypeName<T>::value),
    sizeof(T) * CHAR_BIT);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_TYPE_NAME_AND_SIZE_OPS_H
